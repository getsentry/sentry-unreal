// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

class SentryMethodCallAndroid
{
public:
	static void CallStaticVoidMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
	static FString CallStaticStringMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...);
};
