#pragma once

#include "IAssetManager.h"


class AssetManagerLocator
{
private:

	std::vector<IAssetManager*> _managers;

	template <typename ManagerType, typename... Specifier>
	inline static uint32_t manager_index{ ~0u };

	inline static uint32_t num_managers{0u};

	template <typename ManagerType>
	void registerManager(ManagerType manager)
	{
		_managers.push_back(manager);
		manager_index<ManagerType> = num_managers++;
	}

public:

	// Intended to be called once. Can be called multiple times, however all managers have to be re-registered every time.
	template <typename... ManagerTypes>
	void registerManagers(ManagerTypes*... managers)
	{
		num_managers = 0u;
		_managers.clear();

		_managers.reserve(sizeof...(ManagerTypes));
		(registerManager(managers), ...);
	}

	template <typename... ManagerTypes>
	void appendManagers(ManagerTypes*... managers)
	{
		_managers.reserve(_managers.size() + sizeof...(ManagerTypes));
		(registerManager(managers), ...);
	}

	template <typename ManagerType>
	ManagerType* get()
	{
		const auto& index = manager_index<ManagerType>;
		assert(index < num_managers, "Attempted to access undeclared asset manager through the locator.");
		return reinterpret_cast<ManagerType*>(_managers[index]);
	}
};