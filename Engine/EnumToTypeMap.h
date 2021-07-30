#pragma once

template <auto Val>
struct MappedTypeOf
{
	using type = decltype(Val);
	static_assert(std::is_enum_v<type> && "MappedTypeOf requires an enum.");
};