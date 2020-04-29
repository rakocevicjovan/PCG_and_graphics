#pragma once
#include <memory>


/*
template<typename ReturnType, typename ... Arguments>
class Callable
{
public:

	template<typename Callable>
	universal_call(Callable&& callable) 
		: _impl { std::make_unique<Model<Callable>>(std::forward<Callable>(callable)) } 
	{}



	ReturnType operator()(Arguments&&... args) const
	{
		return _impl->call(std::forward<Arguments>(args)...);
	}


private:

	struct Concept
	{
		virtual R call(Arguments&&... args) = 0;
		virtual ~concept() = default;
	};


	template<class Callable>
	struct Model : Concept
	{
		Model(Callable&& callable) : _callable(std::move(callable)) {}


		R call(Arguments&&... args) override
		{
			return _callable(std::forward<Arguments>(args)...);
		}


		Callable _callable;
	};

	std::unique_ptr<Concept> _impl;
};
*/