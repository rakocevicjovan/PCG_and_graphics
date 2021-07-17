#pragma once

struct Blob
{
	std::unique_ptr<char[]> _data;
	size_t _size = 0u;

	const void* data() const
	{
		return static_cast<void*>(_data.get());
	}

	void* mutableData()
	{
		return static_cast<void*>(_data.get());
	}

	template <typename DesiredType>
	DesiredType* dataAsType()
	{
		return reinterpret_cast<DesiredType*>(_data.get());
	}

	size_t size() const { return _size; }

	bool empty() const { return !_data; }

	void reset()
	{
		_data.reset();
		_size = 0u;
	}
};