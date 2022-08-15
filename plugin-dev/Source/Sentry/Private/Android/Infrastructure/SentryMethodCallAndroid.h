// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

class SentryMethodCallAndroid
{
public:
	/** Static methods */
	static void CallStaticVoidMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
	static FString CallStaticStringMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
	static jobject CallStaticObjectMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);

	/** Instance methods */
	static void CallVoidMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
	static FString CallStringMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
	static jobject CallObjectMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
};
