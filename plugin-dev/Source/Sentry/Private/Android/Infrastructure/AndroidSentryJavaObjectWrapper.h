// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "AndroidSentryDataTypes.h"

#include "Android/AndroidJavaEnv.h"

class FSentryJavaObjectWrapper
{
protected:
	FSentryJavaObjectWrapper(FSentryJavaClass ClassData);

public:
	FSentryJavaObjectWrapper(FSentryJavaClass ClassData, const char* CtorSignature, ...);
	FSentryJavaObjectWrapper(FSentryJavaClass ClassData, jobject JavaClassInstance);
	virtual ~FSentryJavaObjectWrapper();

	FSentryJavaObjectWrapper(const FSentryJavaObjectWrapper& rhs) = delete;
	FSentryJavaObjectWrapper& operator=(const FSentryJavaObjectWrapper& rhs) = delete;

	FSentryJavaMethod GetMethod(const char* MethodName, const char* FunctionSignature);
	static FSentryJavaMethod GetStaticMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature);

	template<typename ReturnType>
	ReturnType CallMethod(FSentryJavaMethod Method, ...) const;
	template<typename ReturnType>
	FScopedJavaObject<ReturnType> CallObjectMethod(FSentryJavaMethod Method, ...) const;

	template<typename ReturnType>
	static ReturnType CallStaticMethod(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...);
	template<typename ReturnType>
	static ReturnType CallStaticMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...);
	template<typename ReturnType>
	static FScopedJavaObject<ReturnType> CallStaticObjectMethod(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...);
	template<typename ReturnType>
	static FScopedJavaObject<ReturnType> CallStaticObjectMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...);

	jobject GetJObject() const;

	static FScopedJavaObject<jstring> GetJString(const FString& String);

	static bool IsInstanceOf(FSentryJavaClass ClassData, jobject JavaClassInstance);

private:
	void VerifyMethodCall(FSentryJavaMethod Method) const;
	void VerifyException() const;

	template<typename ReturnType>
	ReturnType CallMethodInternal(FSentryJavaMethod Method, va_list Params) const;
	template<typename ReturnType>
	FScopedJavaObject<ReturnType> CallObjectMethodInternal(FSentryJavaMethod Method, va_list Params) const;

protected:
	jobject Object;
	jclass Class;
};

template<typename ReturnType>
ReturnType FSentryJavaObjectWrapper::CallMethod(FSentryJavaMethod Method, ...) const
{
	va_list Params;
	va_start(Params, Method);
	ReturnType RetVal = CallMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<typename ReturnType>
FScopedJavaObject<ReturnType> FSentryJavaObjectWrapper::CallObjectMethod(FSentryJavaMethod Method, ...) const
{
	va_list Params;
	va_start(Params, Method);
	FScopedJavaObject<ReturnType> RetVal = CallObjectMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<>
void FSentryJavaObjectWrapper::CallMethod<void>(FSentryJavaMethod Method, ...) const;

template<typename ReturnType>
ReturnType FSentryJavaObjectWrapper::CallStaticMethod(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	va_list Params;
	va_start(Params, Method);
	ReturnType RetVal = StaticInst.CallMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<typename ReturnType>
ReturnType FSentryJavaObjectWrapper::CallStaticMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	FSentryJavaMethod Method = GetStaticMethod(ClassData, MethodName, FunctionSignature);
	va_list Params;
	va_start(Params, FunctionSignature);
	ReturnType RetVal = StaticInst.CallMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<typename ReturnType>
FScopedJavaObject<ReturnType> FSentryJavaObjectWrapper::CallStaticObjectMethod(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	va_list Params;
	va_start(Params, Method);
	FScopedJavaObject<ReturnType> RetVal = StaticInst.CallObjectMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<typename ReturnType>
FScopedJavaObject<ReturnType> FSentryJavaObjectWrapper::CallStaticObjectMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	FSentryJavaMethod Method = GetStaticMethod(ClassData, MethodName, FunctionSignature);
	va_list Params;
	va_start(Params, FunctionSignature);
	FScopedJavaObject<ReturnType> RetVal = StaticInst.CallObjectMethodInternal<ReturnType>(Method, Params);
	va_end(Params);
	return RetVal;
}

template<>
void FSentryJavaObjectWrapper::CallStaticMethod<void>(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...);

template<>
void FSentryJavaObjectWrapper::CallStaticMethod<void>(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...);

template<>
void FSentryJavaObjectWrapper::CallMethodInternal<void>(FSentryJavaMethod Method, va_list Params) const;

template<>
bool FSentryJavaObjectWrapper::CallMethodInternal<bool>(FSentryJavaMethod Method, va_list Params) const;

template<>
int FSentryJavaObjectWrapper::CallMethodInternal<int>(FSentryJavaMethod Method, va_list Params) const;

template<>
float FSentryJavaObjectWrapper::CallMethodInternal<float>(FSentryJavaMethod Method, va_list Params) const;

template<>
int64 FSentryJavaObjectWrapper::CallMethodInternal<int64>(FSentryJavaMethod Method, va_list Params) const;

template<>
FString FSentryJavaObjectWrapper::CallMethodInternal<FString>(FSentryJavaMethod Method, va_list Params) const;

template<>
FScopedJavaObject<jobject> FSentryJavaObjectWrapper::CallObjectMethodInternal<jobject>(FSentryJavaMethod Method, va_list Params) const;

template<>
FScopedJavaObject<jobjectArray> FSentryJavaObjectWrapper::CallObjectMethodInternal<jobjectArray>(FSentryJavaMethod Method, va_list Params) const;