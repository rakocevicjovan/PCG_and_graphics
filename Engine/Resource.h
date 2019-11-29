#pragma once
#include <string>

class Resource
{
protected:
	unsigned int handle;
	unsigned int _refCount;
	std::string _path;
	std::string _name;

public:

	Resource() {}

	Resource(std::string path, std::string name) : _path(path), _name(name) {}

	~Resource() {}

	inline void incRef() { ++_refCount; }
	
	inline void decRef() { --_refCount; }

	inline const std::string& getPath() { return _path; }

	inline const std::string& getName() { return _name; }

	inline void setPathName(const std::string& path, const std::string& name) { _path = path; _name = name; }

	inline bool isInUse() { return _refCount > 0; }
};