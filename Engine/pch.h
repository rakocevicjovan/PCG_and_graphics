#pragma once

// C++ stl includes
#include <algorithm>
#include <array>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <random>
#include <unordered_map>
#include <string>
#include <map>

// Cereal includes - this is abundant throughout the codebase
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

// Misc
#include <entt/entt.hpp>

// Graphics includes
#include <d3d11.h>

// Windows includes
#include <wrl/client.h>

// First party code coming from aeolian that's commonly used elsewhere
#include "Deserialize.h"