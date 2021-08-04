#pragma once


namespace ShGen
{

struct ShaderOption
{
	const char* name;
	uint16_t _offset;
	uint16_t _numBits{ 1 };
	uint16_t _maxVal{ 1 };
	uint64_t depMask{ (~0ul) };
};


// Hardcoded, but rarely changes and it's the easiest way to expose it

// VS options
constexpr ShaderOption OPT_TEX{ "TEX", 0, 3, 4 };
constexpr ShaderOption OPT_NRM{ "NRM", 3 };
constexpr ShaderOption OPT_COL{ "COL", 4 };
constexpr ShaderOption OPT_TAN{ "TAN", 5, 1, 1, (1ul << 3) };
constexpr ShaderOption OPT_BTN{ "BTN", 6, 1, 1, (1ul << 5 | 1ul << 3) };
constexpr ShaderOption OPT_SIW{ "SIW", 7 };
constexpr ShaderOption OPT_INS{ "INS", 8 };

// PS options
constexpr UINT PS_O = 9u; // Horrible but aight for now;
constexpr ShaderOption OPT_LMOD{ "LMOD",	PS_O + 0, 2 };
constexpr ShaderOption OPT_ALPHA{ "ALPHA",	PS_O + 2 };
constexpr ShaderOption OPT_FOG{ "FOG",	PS_O + 3 };
constexpr ShaderOption OPT_SHD{ "SHADOW",	PS_O + 4 };
constexpr ShaderOption OPT_GAMMA{ "GAMMA",	PS_O + 5 };

// 10 texture options, each 1 bit (crazy number of permutations already)
constexpr UINT PS_T_O = PS_O + 6u;
constexpr ShaderOption TX_DIF{ "TEX_DIF", PS_T_O + 0 };
constexpr ShaderOption TX_NRM{ "TEX_NRM", PS_T_O + 1 };
constexpr ShaderOption TX_SPC{ "TEX_SPC", PS_T_O + 2 };
constexpr ShaderOption TX_SHN{ "TEX_SHN", PS_T_O + 3 };
constexpr ShaderOption TX_OPC{ "TEX_OPC", PS_T_O + 4 };
constexpr ShaderOption TX_DPM{ "TEX_DPM", PS_T_O + 5 };
constexpr ShaderOption TX_AMB{ "TEX_AMB", PS_T_O + 6 };
constexpr ShaderOption TX_MTL{ "TEX_MTL", PS_T_O + 7 };
constexpr ShaderOption TX_RGH{ "TEX_RGH", PS_T_O + 8 };
constexpr ShaderOption TX_RFL{ "TEX_RFL", PS_T_O + 9 };
constexpr ShaderOption TX_RFR{ "TEX_RFR", PS_T_O + 10 };

}