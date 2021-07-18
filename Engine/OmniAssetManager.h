#pragma once

#include <tuple>

// One to rule them all and in darkness bind them. Connect all loaders and managers here, with the ledger and possibly using the locator.

template <typename... Types>
class OmniAssetManager
{
private:

	std::tuple<Types...> _managers;

public:

	OmniAssetManager(Types... managers)
	{
		_managers = std::make_tuple(managers);
	}

};