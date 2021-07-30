#pragma once

// type id counter can be templated to allow multiple groups with individual counters
template <typename Group>
static uint32_t type_id_counter{ 0u };

template <typename T, typename Group = void>
struct seq_type_id final
{
	static uint32_t value() noexcept
	{
		static const uint32_t id{ type_id_counter<Group>++ };
		return id;
	}
};