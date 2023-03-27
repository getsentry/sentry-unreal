// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

enum class ESentryJavaClassType : uint8
{
	System = 0,
	External
};

struct FSentryJavaClass
{
	FName Name;
	ESentryJavaClassType Type;
};

struct FSentryJavaMethod
{
	FName Name;
	FName Signature;
	jmethodID Method;
};

class FSentryJavaObjectWrapper
{
public:
	FSentryJavaObjectWrapper(FSentryJavaClass ClassData, const char* CtorSignature, ...);
	FSentryJavaObjectWrapper(FSentryJavaClass ClassData, jobject JavaClassInstance);
	virtual ~FSentryJavaObjectWrapper();

	FSentryJavaObjectWrapper(const FSentryJavaObjectWrapper& rhs) = delete;
	FSentryJavaObjectWrapper& operator = (const FSentryJavaObjectWrapper& rhs) = delete;

	FSentryJavaMethod GetMethod(const char* MethodName, const char* FunctionSignature);

	template<typename ReturnType>
	ReturnType CallMethod(FSentryJavaMethod Method, ...) const;

	jobject GetJObject() const;

	void VerifyException() const;

protected:
	jobject Object;
	jclass Class;
};

template<>
void FSentryJavaObjectWrapper::CallMethod<void>(FSentryJavaMethod Method, ...) const;

template<>
bool FSentryJavaObjectWrapper::CallMethod<bool>(FSentryJavaMethod Method, ...) const;

template<>
int FSentryJavaObjectWrapper::CallMethod<int>(FSentryJavaMethod Method, ...) const;

template<>
jobject FSentryJavaObjectWrapper::CallMethod<jobject>(FSentryJavaMethod Method, ...) const;

template<>
jobjectArray FSentryJavaObjectWrapper::CallMethod<jobjectArray>(FSentryJavaMethod Method, ...) const;

template<>
int64 FSentryJavaObjectWrapper::CallMethod<int64>(FSentryJavaMethod Method, ...) const;

template<>
FString FSentryJavaObjectWrapper::CallMethod<FString>(FSentryJavaMethod Method, ...) const;