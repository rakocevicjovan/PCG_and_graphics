#pragma once
#include "pch.h"
#include "gtest/gtest.h"
#include "../Engine/PooledSceneGraph.h"
#include "../Engine/Math.h"


TEST(PooledSceneGraph, MaxDepth)
{
	int a = 0;
	
	PooledSceneGraph<4> psg(16);
	auto n1 = psg.createNode(SMatrix{}, nullptr);
	auto n2 = psg.createNode(SMatrix::CreateRotationX(PI), nullptr);
	auto n3 = psg.createNode(SMatrix::CreateRotationY(PI), n1);
	auto n4 = psg.createNode(SMatrix{}, n3);
	auto n5 = psg.createNode(SMatrix{}, n4);
	auto n6 = psg.createNode(SMatrix{}, n5);
	auto n7 = psg.createNode(SMatrix{}, n6);

	psg.update(n6);

	EXPECT_DEATH(psg.update(n7), "");	// This only works in debug - no assert on std::array oob indexing in release
}