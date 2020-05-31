#pragma once
#include <string>

class Resource
{
protected:
	unsigned int _handle = 0u;
	unsigned int _refCount = 0u;
	//std::string _path;
	//std::string _name;

public:

	Resource() : _handle(0u), _refCount(0u) { }

	//Resource(std::string path, std::string name) : _path(path), _name(name), _handle(0u), _refCount(0u) {}

	~Resource() {}

	inline void incRef() { ++_refCount; }
	
	inline void decRef() { --_refCount; }

	//inline const std::string& getPath() { return _path; }

	//inline const std::string& getName() { return _name; }

	//inline void setPathName(const std::string& path, const std::string& name) { _path = path; _name = name; }

	inline bool isInUse() { return _refCount > 0; }
};