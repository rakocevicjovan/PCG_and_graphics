#pragma once

#pragma warning( disable : 4267 )

// C++ stl includes
#include <algorithm>
#include <array>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <memory>
#include <execution>
#include <future>
#include <mutex>
#include <numeric>
#include <random>
#include <unordered_map>
#include <string>
#include <map>
#include <type_traits>
#include <variant>
#include <optional>

// For dealing with wstrings
#include <locale>
#include <codecvt>

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


// Graphics includes
#include <d3d11_4.h>
#include <dxgiformat.h>

// Windows includes
#include <wrl/client.h>

// First party code coming from aeolian that's commonly used elsewhere
#include "Math.h"
#include "Deserialize.h"
