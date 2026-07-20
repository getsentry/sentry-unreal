// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJavaEnv.h"

// Plugin-owned RAII wrapper for Java local references.
//
// Mirrors the implementation of FScopedJavaObject in Unreal Engine versions prior to 5.7.
template<typename T>
class FSentryScopedJavaObject
{
public:
	FSentryScopedJavaObject() = default;

	FSentryScopedJavaObject(JNIEnv* InEnv, T InObjRef)
		: Env(InEnv)
		, ObjRef(InObjRef)
	{
	}

	FSentryScopedJavaObject(FSentryScopedJavaObject&& Other)
		: Env(Other.Env)
		, ObjRef(Other.ObjRef)
	{
		Other.Env = nullptr;
		Other.ObjRef = nullptr;
	}

	FSentryScopedJavaObject(const FSentryScopedJavaObject&) = delete;

	~FSentryScopedJavaObject()
	{
		if (Env && ObjRef)
		{
			Env->DeleteLocalRef(ObjRef);
		}
	}

	FSentryScopedJavaObject& operator=(FSentryScopedJavaObject&& Other)
	{
		if (this != &Other)
		{
			if (Env && ObjRef)
			{
				Env->DeleteLocalRef(ObjRef);
			}

			Env = Other.Env;
			ObjRef = Other.ObjRef;
			Other.Env = nullptr;
			Other.ObjRef = nullptr;
		}
		return *this;
	}

	FSentryScopedJavaObject& operator=(const FSentryScopedJavaObject&) = delete;

	T operator*() const { return ObjRef; }

	operator bool() const { return ObjRef != nullptr; }

private:
	JNIEnv* Env = nullptr;
	T ObjRef = nullptr;
};

template<typename T>
FSentryScopedJavaObject<T> NewSentryScopedJavaObject(JNIEnv* InEnv, T InObjRef)
{
	return FSentryScopedJavaObject<T>(InEnv, InObjRef);
}
