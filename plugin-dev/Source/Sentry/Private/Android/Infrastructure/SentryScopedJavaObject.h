// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidApplication.h"

/**
 * Class that creates new global object reference if needed and automatically deletes it when out of scope
 */
template <typename T>
class FSentryScopedJavaObject
{
public:
	FSentryScopedJavaObject(const T& InObjRef, bool IsNewGlobalRefNeeded = false) 
		: ObjRef(InObjRef)
	{
		check(ObjRef);

		JNIEnv* Env = FAndroidApplication::GetJavaEnv();
		check(Env);

		if(IsNewGlobalRefNeeded)
		{
			ObjRef = (T)Env->NewGlobalRef(ObjRef);
		}
	}

	FSentryScopedJavaObject(FSentryScopedJavaObject&& Other) 
		: ObjRef(Other.ObjRef)
	{
		Other.ObjRef = nullptr;
	}

	FSentryScopedJavaObject(const FSentryScopedJavaObject& Other) = delete;
	FSentryScopedJavaObject& operator=(const FSentryScopedJavaObject& Other) = delete;

	~FSentryScopedJavaObject()
	{
		if (*this)
		{
			JNIEnv* Env = FAndroidApplication::GetJavaEnv();
			Env->DeleteGlobalRef(ObjRef);
		}
		ObjRef = nullptr;
	}

	operator bool() const
	{
		JNIEnv* Env = FAndroidApplication::GetJavaEnv();
		if (!Env || !ObjRef || Env->IsSameObject(ObjRef, NULL))
		{
			return false;
		}

		return true;
	}

	// Returns the underlying JNI pointer
	T operator*() const { return ObjRef; }

private:
	T ObjRef = nullptr;
};

/**
 * Helper function that allows template deduction on the java object type, for example:
 * auto ScopedObject = NewSentryScopedJavaObject(Env, JavaString);
 * instead of FSentryScopedJavaObject<jstring> ScopeObject(Env, JavaString);
 */
template <typename T>
FSentryScopedJavaObject<T> NewSentryScopedJavaObject(const T& InObjRef)
{
	return FSentryScopedJavaObject<T>(InObjRef);
}