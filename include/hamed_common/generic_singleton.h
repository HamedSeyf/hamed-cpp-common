#pragma once

#include <cstdint>
#include <functional>


template<typename T>
class TSingleton
{
public:

	static T& GetInstance()
	{
		static T sInstance;
		return sInstance;
	}

protected:
	TSingleton() = default;
	virtual ~TSingleton() = default;

	TSingleton(const TSingleton&) = delete;
	TSingleton& operator=(const TSingleton&) = delete;
	TSingleton(TSingleton&&) = delete;
	TSingleton& operator=(TSingleton&&) = delete;
};

#define ENFORCE_SINGELTON_WITH_DEFAULT_CTOR(Class) \
private: \
Class() = default; \
friend class TSingleton<Class>
