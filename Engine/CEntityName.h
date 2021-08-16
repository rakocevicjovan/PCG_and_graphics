#pragma once

struct CEntityName
{
	char _name[32]{};


	CEntityName() = default;


	CEntityName(const char* src)
	{
		set(src);
	}


	inline void set(const char* name)
	{
		//assert(strlen(name) < 32); No need, ImGui input will not allow overflow.
		std::strcpy(_name, name);
	}


	inline char* get()
	{
		return _name;
	}
};