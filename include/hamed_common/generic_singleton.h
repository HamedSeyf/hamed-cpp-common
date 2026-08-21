#pragma once

import <cstdint>;
import <functional>;


template<typename T>
class TSingleton
{
public:

	[[nodiscard]] static T& GetInstance()
	{
		static T sInstance;
		return sInstance;
	}

protected:
	TSingleton() = default;
	~TSingleton() = default;

	TSingleton(const TSingleton&) = delete;
	TSingleton& operator=(const TSingleton&) = delete;
	TSingleton(TSingleton&&) = delete;
	TSingleton& operator=(TSingleton&&) = delete;
};

#define ENFORCE_SINGELTON_WITH_DEFAULT_CTOR(Class) \
private: \
Class() = default; \
friend class TSingleton<Class>
