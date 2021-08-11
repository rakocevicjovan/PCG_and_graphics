#pragma once

class Proj
{
private:

	std::string _name;

	std::string _path;
	std::string _ledgerPath;

	// Add project defaults etc. later when the need arises

public:

	inline const char* name() { return _name.c_str(); }
	inline const char* path() { return _path.c_str(); }
	inline const char* ledgerPath() { return _ledgerPath.c_str(); }

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_name, _path, _ledgerPath);
	}
};