#pragma once
#include "pch.h"
#include "gtest/gtest.h"
#include "../Engine/PooledSceneGraph.h"
#include "../Engine/Math.h"


// This should very likely be in another project...
TEST(PooledSceneGraph, MaxDepth)
{
	int a = 0;
	
	PooledSceneGraph<4> lsg(16);
	auto n1 = lsg.createNode(SMatrix{}, nullptr);
	auto n2 = lsg.createNode(SMatrix::CreateRotationX(PI), nullptr);
	auto n3 = lsg.createNode(SMatrix::CreateRotationY(PI), n1);
	auto n4 = lsg.createNode(SMatrix{}, n3);
	auto n5 = lsg.createNode(SMatrix{}, n4);
	auto n6 = lsg.createNode(SMatrix{}, n5);
	auto n7 = lsg.createNode(SMatrix{}, n6);

	lsg.update(n6);

	EXPECT_DEATH(lsg.update(n7), "");	// This only works in debug - no assert on std::array oob indexing in release
}