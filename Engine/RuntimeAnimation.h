#pragma once

class Skeleton;
class Animation;

struct RuntimeAnimation
{

};


RuntimeAnimation BuildRuntimeAnimation(const Skeleton& skeleton, const Animation& animation);