#pragma once

// C++ includes
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


// Graphics includes
#include <d3d11.h>


// Cereal includes - this is abundant
#include <cereal/cereal.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

// Windows includes
#include <wrl/client.h>