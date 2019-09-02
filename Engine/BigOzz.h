/*
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/animation/offline/raw_skeleton.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/offline/skeleton_builder.h"
#include "ozz/animation/offline/animation_builder.h"

#include "ozz/base/io/stream.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/log.h"

class BigOzz
{
	ozz::animation::Skeleton*	skeleton;
	ozz::animation::Animation*	animation;

public:
	BigOzz() {};
	~BigOzz() {};

	int init(const std::string& filename)
	{
		ozz::io::File file(filename.c_str(), "rb");

		if (!file.opened())
		{
			ozz::log::Err() << "Cannot open file " << filename.c_str() << "." << std::endl;
			return EXIT_FAILURE;
		}

		ozz::io::IArchive archive(&file);

		if (!archive.TestTag<ozz::animation::Skeleton>()) {
			ozz::log::Err() << "Archive doesn't contain the expected object type." <<
				std::endl;
			return EXIT_FAILURE;
		}

		ozz::animation::Skeleton skeleton;
		archive >> skeleton;
	}



	int itIsAMeeeSkeletor()
	{
		ozz::animation::offline::RawSkeleton raw_skeleton;

		raw_skeleton.roots.resize(1);

		ozz::animation::offline::RawSkeleton::Joint& root = raw_skeleton.roots[0];
		root.name = "root";
		root.transform.translation = ozz::math::Float3(0.f, 1.f, 0.f);
		root.transform.rotation = ozz::math::Quaternion(0.f, 0.f, 0.f, 1.f);
		root.transform.scale = ozz::math::Float3(1.f, 1.f, 1.f);

		root.children.resize(2);

		ozz::animation::offline::RawSkeleton::Joint& left = root.children[0];
		left.name = "left";
		left.transform.translation = ozz::math::Float3(1.f, 0.f, 0.f);
		//left.transform.rotation

		if (!raw_skeleton.Validate())
			return EXIT_FAILURE;

		ozz::animation::offline::SkeletonBuilder builder;

		skeleton = builder(raw_skeleton);
	}



	int buildAnimation()
	{
		ozz::animation::offline::RawAnimation raw_animation;
		raw_animation.duration = 1.4f;
		raw_animation.tracks.resize(3);

		const ozz::animation::offline::RawAnimation::TranslationKey key0 = {0.f, ozz::math::Float3(0.f, 4.6f, 0.f) };
		raw_animation.tracks[0].translations.push_back(key0);

		// Create a new keyframe, at t=0.93 (must be less than duration), with a translation value.
		const ozz::animation::offline::RawAnimation::TranslationKey key1 = {.93f, ozz::math::Float3(0.f, 9.9f, 0.f) };
		raw_animation.tracks[0].translations.push_back(key1);

		const ozz::animation::offline::RawAnimation::RotationKey rotKey1 = {.46f, ozz::math::Quaternion(0.f, 1.f, 0.f, 0.f) };
		raw_animation.tracks[0].rotations.push_back(rotKey1);
		
		if (!raw_animation.Validate())
			return EXIT_FAILURE;
	
		ozz::animation::offline::AnimationBuilder builder;
		animation = builder(raw_animation);
	}



	void deletSkeletor()
	{
		ozz::memory::default_allocator()->Delete(skeleton);
		ozz::memory::default_allocator()->Delete(animation);
	}

};
*/