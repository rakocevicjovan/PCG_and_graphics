#pragma once

#include "pch.h"

// The following won't build, but it works in the engine code. It's here as an example.
// It could build if all necessary includes were resolved.
// This won't be an issue when I clean up includes in asset code eventually.

//#include "../Engine/ServiceLocator.h"
//#include "../Engine/EnumToTypeMap.h"
//#include "../Engine/AssetType.h"
//#include "../Engine/IAssetManager.h"
//#include "../Engine/ModelManager.h"
//#include "../Engine/MaterialManager.h"

//template<>
//struct MappedTypeOf<AssetType::SK_MODEL>
//{
//	using type = ModelManager;
//};
//
//
//TEST(ServiceLocatorTest, GetServiceOfType)
//{
//	ServiceLocator<IAssetManager>::registerServices(
//		new ModelManager(),
//		new MaterialManager());
//
//	using bonk = MappedTypeOf<AssetType::SK_MODEL>::type;
//	auto* manager = ServiceLocator<IAssetManager>::get<bonk>();
//}