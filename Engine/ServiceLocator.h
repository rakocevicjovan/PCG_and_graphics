#pragma once

#include "TypeID.h" 
#include <assert.h>

// All instances instantiated with the same BaseType will SHARE values. For all intents and purposes, every instantiation of this template is a "static" class.
template <typename BaseType>
class ServiceLocator
{
private:
	
	using My_type = ServiceLocator<BaseType>;

	static_assert(!std::is_pointer_v<BaseType> && "ServiceLocator BaseType must not be a pointer.");

	inline static std::vector<BaseType*> _services{};

	template <typename ServiceType>
	static void registerService(ServiceType* manager)
	{
		static_assert(std::is_base_of_v<BaseType, std::remove_pointer<ServiceType>::type>);
		auto id = seq_type_id<ServiceType, My_type>::value();
		assert(_services[id] == nullptr && "Types stored in the service locator must be unique and distinct.");
		_services[id] = manager;
	}

public:

	// Intended to be called once. Can be called multiple times, however all services have to be re-registered every time. Maybe shouldn't be public?
	template <typename... ServiceTypes>
	static void registerServices(ServiceTypes*... services)
	{
		_services.clear();
		_services.resize(sizeof...(ServiceTypes));
		(registerService(services), ...);
	}


	template <typename... ServiceTypes>
	static void appendServices(ServiceTypes*... services)
	{
		_services.resize(_services.size() + sizeof...(ServiceTypes));
		(registerService(services), ...);
	}

	template <typename ServiceType>
	static ServiceType* get()
	{
		const auto index = seq_type_id<ServiceType, My_type>::value();
		auto num_services_registered = type_id_counter<My_type>;
		assert(index < num_services_registered && "Attempted to access undeclared service through the locator.");
		return reinterpret_cast<ServiceType*>(_services[index]);
	}
};