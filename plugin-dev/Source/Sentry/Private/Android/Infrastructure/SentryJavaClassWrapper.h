// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

struct FSentryJavaClassMethod
{
	FName Name;
	FName Signature;
	jmethodID Method;
};

class FSentryJavaClassWrapper
{
public:
	FSentryJavaClassWrapper(FName ClassName, const char* CtorSignature, ...);
	FSentryJavaClassWrapper(FName ClassName, jobject JavaClassInstance);
	virtual ~FSentryJavaClassWrapper();

	FSentryJavaClassWrapper(const FSentryJavaClassWrapper& rhs) = delete;
	FSentryJavaClassWrapper& operator = (const FSentryJavaClassWrapper& rhs) = delete;

	FSentryJavaClassMethod GetClassMethod(const char* MethodName, const char* FunctionSignature);

	template<typename ReturnType>
	ReturnType CallMethod(FSentryJavaClassMethod Method, ...) const;

	jobject GetJObject() const;

	void VerifyException() const;

protected:
	jobject Object;
	jclass Class;
};

template<>
void FSentryJavaClassWrapper::CallMethod<void>(FSentryJavaClassMethod Method, ...) const;

template<>
bool FSentryJavaClassWrapper::CallMethod<bool>(FSentryJavaClassMethod Method, ...) const;

template<>
int FSentryJavaClassWrapper::CallMethod<int>(FSentryJavaClassMethod Method, ...) const;

template<>
jobject FSentryJavaClassWrapper::CallMethod<jobject>(FSentryJavaClassMethod Method, ...) const;

template<>
jobjectArray FSentryJavaClassWrapper::CallMethod<jobjectArray>(FSentryJavaClassMethod Method, ...) const;

template<>
int64 FSentryJavaClassWrapper::CallMethod<int64>(FSentryJavaClassMethod Method, ...) const;

template<>
FString FSentryJavaClassWrapper::CallMethod<FString>(FSentryJavaClassMethod Method, ...) const;