#pragma once

#pragma warning( disable : 4267 )

// C++ stl includes
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <fstream>
#include <future>
#include <immintrin.h>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <streambuf>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

// For dealing with wstrings
#include <codecvt>
#include <locale>

#if __cplusplus > 201703L
#include <span>
#endif


// Third party - disable warnings for these 

// cereal 
#pragma warning(push)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#include <cereal/cereal.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/memory.hpp>
#pragma warning(pop) 

// entt
#include <entt/entt.hpp>

// rapidjson
#pragma warning(push)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 4996)
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#pragma warning(pop) 


// Graphics DX11 includes
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11_4.h>
#include <dxgiformat.h>
#include <d3dcompiler.h>

// Windows includes
#include <wrl/client.h>

// First party code coming from aeolian that's commonly used elsewhere
#include "Math.h"
#include "Deserialize.h"
