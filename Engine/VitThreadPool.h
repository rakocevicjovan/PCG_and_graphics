/*********************************************************
*
*  Copyright (C) 2014 by Vitaliy Vitsentiy
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*********************************************************/


#ifndef __ctpl_stl_thread_pool_H__
#define __ctpl_stl_thread_pool_H__

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>
#include <mutex>
#include <queue>



// thread pool to run user's functors with signature
//      ret func(int id, other_params)
// where id is the index of the thread that runs the functor
// ret is some return type


namespace ctpl {

	namespace detail {
		template <typename T>
		class Queue {
		public:
			bool push(T const & value) {
				std::unique_lock<std::mutex> lock(this->mutex);
				this->q.push(value);
				return true;
			}
			// deletes the retrieved element, do not use for non integral types
			bool pop(T & v) {
				std::unique_lock<std::mutex> lock(this->mutex);
				if (this->q.empty())
					return false;
				v = this->q.front();
				this->q.pop();
				return true;
			}
			bool empty() {
				std::unique_lock<std::mutex> lock(this->mutex);
				return this->q.empty();
			}
		private:
			std::queue<T> q;
			std::mutex mutex;
		};
	}

	class thread_pool {

	public:

		thread_pool() { this->init(); }
		thread_pool(int nThreads) { this->init(); this->resize(nThreads); }

		// the destructor waits for all the functions in the queue to be finished
		~thread_pool() {
			this->stop(true);
		}

		// get the number of running threads in the pool
		int size() { return static_cast<int>(this->threads.size()); }

		// number of idle threads
		int n_idle() { return this->nWaiting; }
		std::thread & get_thread(int i) { return *this->threads[i]; }

		// change the number of threads in the pool
		// should be called from one thread, otherwise be careful to not interleave, also with this->stop()
		// nThreads must be >= 0
		void resize(int nThreads) {
			if (!this->isStop && !this->isDone) {
				int oldNThreads = static_cast<int>(this->threads.size());
				if (oldNThreads <= nThreads) {  // if the number of threads is increased
					this->threads.resize(nThreads);
					this->flags.resize(nThreads);

					for (int i = oldNThreads; i < nThreads; ++i) {
						this->flags[i] = std::make_shared<std::atomic<bool>>(false);
						this->set_thread(i);
					}
				}
				else {  // the number of threads is decreased
					for (int i = oldNThreads - 1; i >= nThreads; --i) {
						*this->flags[i] = true;  // this thread will finish
						this->threads[i]->detach();
					}
					{
						// stop the detached threads that were waiting
						std::unique_lock<std::mutex> lock(this->mutex);
						this->cv.notify_all();
					}
					this->threads.resize(nThreads);  // safe to delete because the threads are detached
					this->flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
				}
			}
		}

		// empty the queue
		void clear_queue() {
			std::function<void(int id)> * _f;
			while (this->q.pop(_f))
				delete _f; // empty the queue
		}

		// pops a functional wrapper to the original function
		std::function<void(int)> pop() {
			std::function<void(int id)> * _f = nullptr;
			this->q.pop(_f);
			std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
			std::function<void(int)> f;
			if (_f)
				f = *_f;
			return f;
		}

		// wait for all computing threads to finish and stop all threads
		// may be called asynchronously to not pause the calling thread while waiting
		// if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
		void stop(bool isWait = false) {
			if (!isWait) {
				if (this->isStop)
					return;
				this->isStop = true;
				for (int i = 0, n = this->size(); i < n; ++i) {
					*this->flags[i] = true;  // command the threads to stop
				}
				this->clear_queue();  // empty the queue
			}
			else {
				if (this->isDone || this->isStop)
					return;
				this->isDone = true;  // give the waiting threads a command to finish
			}
			{
				std::unique_lock<std::mutex> lock(this->mutex);
				this->cv.notify_all();  // stop all waiting threads
			}
			for (int i = 0; i < static_cast<int>(this->threads.size()); ++i) {  // wait for the computing threads to finish
				if (this->threads[i]->joinable())
					this->threads[i]->join();
			}
			// if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
			// therefore delete them here
			this->clear_queue();
			this->threads.clear();
			this->flags.clear();
		}

		template<typename F, typename... Rest>
		auto push(F && f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
			auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
				std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
				);
			auto _f = new std::function<void(int id)>([pck](int id) {
				(*pck)(id);
			});
			this->q.push(_f);
			std::unique_lock<std::mutex> lock(this->mutex);
			this->cv.notify_one();
			return pck->get_future();
		}

		// run the user's function that excepts argument int - id of the running thread. returned value is templatized
		// operator returns std::future, where the user can get the result and rethrow the catched exceptins
		template<typename F>
		auto push(F && f) ->std::future<decltype(f(0))> {
			auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
			auto _f = new std::function<void(int id)>([pck](int id) {
				(*pck)(id);
			});
			this->q.push(_f);
			std::unique_lock<std::mutex> lock(this->mutex);
			this->cv.notify_one();
			return pck->get_future();
		}


	private:

		// deleted
		thread_pool(const thread_pool &);// = delete;
		thread_pool(thread_pool &&);// = delete;
		thread_pool & operator=(const thread_pool &);// = delete;
		thread_pool & operator=(thread_pool &&);// = delete;

		void set_thread(int i) {
			std::shared_ptr<std::atomic<bool>> flag(this->flags[i]); // a copy of the shared ptr to the flag
			auto f = [this, i, flag/* a copy of the shared ptr to the flag */]() {
				std::atomic<bool> & _flag = *flag;
				std::function<void(int id)> * _f;
				bool isPop = this->q.pop(_f);
				while (true) {
					while (isPop) {  // if there is anything in the queue
						std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
						(*_f)(i);
						if (_flag)
							return;  // the thread is wanted to stop, return even if the queue is not empty yet
						else
							isPop = this->q.pop(_f);
					}
					// the queue is empty here, wait for the next command
					std::unique_lock<std::mutex> lock(this->mutex);
					++this->nWaiting;
					this->cv.wait(lock, [this, &_f, &isPop, &_flag]() { isPop = this->q.pop(_f); return isPop || this->isDone || _flag; });
					--this->nWaiting;
					if (!isPop)
						return;  // if the queue is empty and this->isDone == true or *flag then return
				}
			};
			this->threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
		}

		void init() { this->nWaiting = 0; this->isStop = false; this->isDone = false; }

		std::vector<std::unique_ptr<std::thread>> threads;
		std::vector<std::shared_ptr<std::atomic<bool>>> flags;
		detail::Queue<std::function<void(int id)> *> q;
		std::atomic<bool> isDone;
		std::atomic<bool> isStop;
		std::atomic<int> nWaiting;  // how many threads are waiting

		std::mutex mutex;
		std::condition_variable cv;
	};

}

#endif // __ctpl_stl_thread_pool_H__


/* Licensing

Apache License
						   Version 2.0, January 2004
						http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

	  "License" shall mean the terms and conditions for use, reproduction,
	  and distribution as defined by Sections 1 through 9 of this document.

	  "Licensor" shall mean the copyright owner or entity authorized by
	  the copyright owner that is granting the License.

	  "Legal Entity" shall mean the union of the acting entity and all
	  other entities that control, are controlled by, or are under common
	  control with that entity. For the purposes of this definition,
	  "control" means (i) the power, direct or indirect, to cause the
	  direction or management of such entity, whether by contract or
	  otherwise, or (ii) ownership of fifty percent (50%) or more of the
	  outstanding shares, or (iii) beneficial ownership of such entity.

	  "You" (or "Your") shall mean an individual or Legal Entity
	  exercising permissions granted by this License.

	  "Source" form shall mean the preferred form for making modifications,
	  including but not limited to software source code, documentation
	  source, and configuration files.

	  "Object" form shall mean any form resulting from mechanical
	  transformation or translation of a Source form, including but
	  not limited to compiled object code, generated documentation,
	  and conversions to other media types.

	  "Work" shall mean the work of authorship, whether in Source or
	  Object form, made available under the License, as indicated by a
	  copyright notice that is included in or attached to the work
	  (an example is provided in the Appendix below).

	  "Derivative Works" shall mean any work, whether in Source or Object
	  form, that is based on (or derived from) the Work and for which the
	  editorial revisions, annotations, elaborations, or other modifications
	  represent, as a whole, an original work of authorship. For the purposes
	  of this License, Derivative Works shall not include works that remain
	  separable from, or merely link (or bind by name) to the interfaces of,
	  the Work and Derivative Works thereof.

	  "Contribution" shall mean any work of authorship, including
	  the original version of the Work and any modifications or additions
	  to that Work or Derivative Works thereof, that is intentionally
	  submitted to Licensor for inclusion in the Work by the copyright owner
	  or by an individual or Legal Entity authorized to submit on behalf of
	  the copyright owner. For the purposes of this definition, "submitted"
	  means any form of electronic, verbal, or written communication sent
	  to the Licensor or its representatives, including but not limited to
	  communication on electronic mailing lists, source code control systems,
	  and issue tracking systems that are managed by, or on behalf of, the
	  Licensor for the purpose of discussing and improving the Work, but
	  excluding communication that is conspicuously marked or otherwise
	  designated in writing by the copyright owner as "Not a Contribution."

	  "Contributor" shall mean Licensor and any individual or Legal Entity
	  on behalf of whom a Contribution has been received by Licensor and
	  subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
	  this License, each Contributor hereby grants to You a perpetual,
	  worldwide, non-exclusive, no-charge, royalty-free, irrevocable
	  copyright license to reproduce, prepare Derivative Works of,
	  publicly display, publicly perform, sublicense, and distribute the
	  Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
	  this License, each Contributor hereby grants to You a perpetual,
	  worldwide, non-exclusive, no-charge, royalty-free, irrevocable
	  (except as stated in this section) patent license to make, have made,
	  use, offer to sell, sell, import, and otherwise transfer the Work,
	  where such license applies only to those patent claims licensable
	  by such Contributor that are necessarily infringed by their
	  Contribution(s) alone or by combination of their Contribution(s)
	  with the Work to which such Contribution(s) was submitted. If You
	  institute patent litigation against any entity (including a
	  cross-claim or counterclaim in a lawsuit) alleging that the Work
	  or a Contribution incorporated within the Work constitutes direct
	  or contributory patent infringement, then any patent licenses
	  granted to You under this License for that Work shall terminate
	  as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
	  Work or Derivative Works thereof in any medium, with or without
	  modifications, and in Source or Object form, provided that You
	  meet the following conditions:

	  (a) You must give any other recipients of the Work or
		  Derivative Works a copy of this License; and

	  (b) You must cause any modified files to carry prominent notices
		  stating that You changed the files; and

	  (c) You must retain, in the Source form of any Derivative Works
		  that You distribute, all copyright, patent, trademark, and
		  attribution notices from the Source form of the Work,
		  excluding those notices that do not pertain to any part of
		  the Derivative Works; and

	  (d) If the Work includes a "NOTICE" text file as part of its
		  distribution, then any Derivative Works that You distribute must
		  include a readable copy of the attribution notices contained
		  within such NOTICE file, excluding those notices that do not
		  pertain to any part of the Derivative Works, in at least one
		  of the following places: within a NOTICE text file distributed
		  as part of the Derivative Works; within the Source form or
		  documentation, if provided along with the Derivative Works; or,
		  within a display generated by the Derivative Works, if and
		  wherever such third-party notices normally appear. The contents
		  of the NOTICE file are for informational purposes only and
		  do not modify the License. You may add Your own attribution
		  notices within Derivative Works that You distribute, alongside
		  or as an addendum to the NOTICE text from the Work, provided
		  that such additional attribution notices cannot be construed
		  as modifying the License.

	  You may add Your own copyright statement to Your modifications and
	  may provide additional or different license terms and conditions
	  for use, reproduction, or distribution of Your modifications, or
	  for any such Derivative Works as a whole, provided Your use,
	  reproduction, and distribution of the Work otherwise complies with
	  the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
	  any Contribution intentionally submitted for inclusion in the Work
	  by You to the Licensor shall be under the terms and conditions of
	  this License, without any additional terms or conditions.
	  Notwithstanding the above, nothing herein shall supersede or modify
	  the terms of any separate license agreement you may have executed
	  with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
	  names, trademarks, service marks, or product names of the Licensor,
	  except as required for reasonable and customary use in describing the
	  origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
	  agreed to in writing, Licensor provides the Work (and each
	  Contributor provides its Contributions) on an "AS IS" BASIS,
	  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
	  implied, including, without limitation, any warranties or conditions
	  of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
	  PARTICULAR PURPOSE. You are solely responsible for determining the
	  appropriateness of using or redistributing the Work and assume any
	  risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
	  whether in tort (including negligence), contract, or otherwise,
	  unless required by applicable law (such as deliberate and grossly
	  negligent acts) or agreed to in writing, shall any Contributor be
	  liable to You for damages, including any direct, indirect, special,
	  incidental, or consequential damages of any character arising as a
	  result of this License or out of the use or inability to use the
	  Work (including but not limited to damages for loss of goodwill,
	  work stoppage, computer failure or malfunction, or any and all
	  other commercial damages or losses), even if such Contributor
	  has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
	  the Work or Derivative Works thereof, You may choose to offer,
	  and charge a fee for, acceptance of support, warranty, indemnity,
	  or other liability obligations and/or rights consistent with this
	  License. However, in accepting such obligations, You may act only
	  on Your own behalf and on Your sole responsibility, not on behalf
	  of any other Contributor, and only if You agree to indemnify,
	  defend, and hold each Contributor harmless for any liability
	  incurred by, or claims asserted against, such Contributor by reason
	  of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS


   Copyright (C) 2014 by Vitaliy Vitsentiy

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/