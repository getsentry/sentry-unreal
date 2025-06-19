// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryJavaObjectWrapper.h"

#include "Android/AndroidJNI.h"

FSentryJavaObjectWrapper::FSentryJavaObjectWrapper(FSentryJavaClass ClassData)
	: Object(nullptr)
	, Class(nullptr)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassData.Name.GetPlainANSIString(AnsiClassName);

	if (ClassData.Type == ESentryJavaClassType::System)
		Class = FJavaWrapper::FindClassGlobalRef(JEnv, AnsiClassName, false);
	if (ClassData.Type == ESentryJavaClassType::External)
		Class = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);

	check(Class);
}

FSentryJavaObjectWrapper::FSentryJavaObjectWrapper(FSentryJavaClass ClassData, const char* CtorSignature, ...)
	: FSentryJavaObjectWrapper(ClassData)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	jmethodID Constructor = JEnv->GetMethodID(Class, "<init>", CtorSignature);
	check(Constructor);

	va_list Params;
	va_start(Params, CtorSignature);
	auto LocalObject = NewScopedJavaObject(JEnv, JEnv->NewObjectV(Class, Constructor, Params));
	va_end(Params);
	VerifyException();
	check(LocalObject);

	Object = JEnv->NewGlobalRef(*LocalObject);
}

FSentryJavaObjectWrapper::FSentryJavaObjectWrapper(FSentryJavaClass ClassData, jobject JavaClassInstance)
	: FSentryJavaObjectWrapper(ClassData)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	if (!JEnv->IsInstanceOf(JavaClassInstance, Class))
	{
		verify(false && "Java class instance type doesn't match the specified class.");
	}

	Object = JEnv->NewGlobalRef(JavaClassInstance);
}

FSentryJavaObjectWrapper::~FSentryJavaObjectWrapper()
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	if (Object)
		JEnv->DeleteGlobalRef(Object);
	if (Class)
		JEnv->DeleteGlobalRef(Class);
}

FSentryJavaMethod FSentryJavaObjectWrapper::GetMethod(const char* MethodName, const char* FunctionSignature)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	FSentryJavaMethod Method;
	Method.Method = JEnv->GetMethodID(Class, MethodName, FunctionSignature);
	Method.Name = MethodName;
	Method.Signature = FunctionSignature;
	Method.IsStatic = false;
	checkf(Method.Method, TEXT("Unable to find Java Method %s with Signature %s"), UTF8_TO_TCHAR(MethodName), UTF8_TO_TCHAR(FunctionSignature));
	return Method;
}

FSentryJavaMethod FSentryJavaObjectWrapper::GetStaticMethod(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature)
{
	FSentryJavaObjectWrapper StaticClass(ClassData);

	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	FSentryJavaMethod Method;
	Method.Method = JEnv->GetStaticMethodID(StaticClass.Class, MethodName, FunctionSignature);
	Method.Name = MethodName;
	Method.Signature = FunctionSignature;
	Method.IsStatic = true;
	checkf(Method.Method, TEXT("Unable to find Java Method %s with Signature %s"), UTF8_TO_TCHAR(MethodName), UTF8_TO_TCHAR(FunctionSignature));
	return Method;
}

jobject FSentryJavaObjectWrapper::GetJObject() const
{
	check(Object);
	return Object;
}

FScopedJavaObject<jstring> FSentryJavaObjectWrapper::GetJString(const FString& String)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	return FJavaHelper::ToJavaString(JEnv, String);
}

bool FSentryJavaObjectWrapper::IsInstanceOf(FSentryJavaClass ClassData, jobject JavaClassInstance)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	ANSICHAR AnsiClassName[NAME_SIZE];
	ClassData.Name.GetPlainANSIString(AnsiClassName);

	jclass ClassGlobalRef;

	if (ClassData.Type == ESentryJavaClassType::System)
		ClassGlobalRef = FJavaWrapper::FindClassGlobalRef(JEnv, AnsiClassName, false);
	if (ClassData.Type == ESentryJavaClassType::External)
		ClassGlobalRef = AndroidJavaEnv::FindJavaClassGlobalRef(AnsiClassName);

	check(ClassGlobalRef);

	return JEnv->IsInstanceOf(JavaClassInstance, ClassGlobalRef);
}

void FSentryJavaObjectWrapper::VerifyMethodCall(FSentryJavaMethod Method) const
{
	if (Method.IsStatic && Object)
	{
		verify(false && "Calling static method using class instance is not allowed. Try CallStaticMethod/CallStaticObjectMethod instead.");
	}

	if (!Method.IsStatic && !Object)
	{
		verify(false && "Calling instance method in static context is not allowed. Check method config or try CallMethod/CallObjectMethod instead.");
	}
}

void FSentryJavaObjectWrapper::VerifyException() const
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	if (JEnv->ExceptionCheck())
	{
		JEnv->ExceptionDescribe();
		JEnv->ExceptionClear();
		verify(false && "Java JNI call failed with an exception.");
	}
}

template<>
void FSentryJavaObjectWrapper::CallMethod<void>(FSentryJavaMethod Method, ...) const
{
	va_list Params;
	va_start(Params, Method);
	CallMethodInternal<void>(Method, Params);
	va_end(Params);
}

template<>
void FSentryJavaObjectWrapper::CallStaticMethod<void>(FSentryJavaClass ClassData, FSentryJavaMethod Method, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	va_list Params;
	va_start(Params, Method);
	StaticInst.CallMethodInternal<void>(Method, Params);
	va_end(Params);
}

template<>
void FSentryJavaObjectWrapper::CallStaticMethod<void>(FSentryJavaClass ClassData, const char* MethodName, const char* FunctionSignature, ...)
{
	FSentryJavaObjectWrapper StaticInst(ClassData);
	FSentryJavaMethod Method = GetStaticMethod(ClassData, MethodName, FunctionSignature);
	va_list Params;
	va_start(Params, FunctionSignature);
	StaticInst.CallMethodInternal<void>(Method, Params);
	va_end(Params);
}

template<>
void FSentryJavaObjectWrapper::CallMethodInternal<void>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	!Method.IsStatic
		? JEnv->CallVoidMethodV(Object, Method.Method, Params)
		: JEnv->CallStaticVoidMethodV(Class, Method.Method, Params);

	VerifyException();
}

template<>
bool FSentryJavaObjectWrapper::CallMethodInternal<bool>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	bool RetVal =
		!Method.IsStatic
			? JEnv->CallBooleanMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticBooleanMethodV(Class, Method.Method, Params);

	VerifyException();
	return RetVal;
}

template<>
int FSentryJavaObjectWrapper::CallMethodInternal<int>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	int RetVal =
		!Method.IsStatic
			? JEnv->CallIntMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticIntMethodV(Class, Method.Method, Params);

	VerifyException();
	return RetVal;
}

template<>
float FSentryJavaObjectWrapper::CallMethodInternal<float>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	float RetVal =
		!Method.IsStatic
			? JEnv->CallFloatMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticFloatMethodV(Class, Method.Method, Params);

	VerifyException();
	return RetVal;
}

template<>
int64 FSentryJavaObjectWrapper::CallMethodInternal<int64>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	int64 RetVal =
		!Method.IsStatic
			? JEnv->CallLongMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticLongMethodV(Class, Method.Method, Params);

	VerifyException();
	return RetVal;
}

template<>
FString FSentryJavaObjectWrapper::CallMethodInternal<FString>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	jstring RetVal =
		!Method.IsStatic
			? static_cast<jstring>(JEnv->CallObjectMethodV(Object, Method.Method, Params))
			: static_cast<jstring>(JEnv->CallStaticObjectMethodV(Class, Method.Method, Params));

	VerifyException();
	auto Result = FJavaHelper::FStringFromLocalRef(JEnv, RetVal);
	return Result;
}

template<>
FScopedJavaObject<jobject> FSentryJavaObjectWrapper::CallObjectMethodInternal<jobject>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	jobject RetVal =
		!Method.IsStatic
			? JEnv->CallObjectMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticObjectMethodV(Class, Method.Method, Params);

	VerifyException();
	return NewScopedJavaObject(JEnv, RetVal);
}

template<>
FScopedJavaObject<jobjectArray> FSentryJavaObjectWrapper::CallObjectMethodInternal<jobjectArray>(FSentryJavaMethod Method, va_list Params) const
{
	VerifyMethodCall(Method);
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();

	jobject RetVal =
		!Method.IsStatic
			? JEnv->CallObjectMethodV(Object, Method.Method, Params)
			: JEnv->CallStaticObjectMethodV(Class, Method.Method, Params);

	VerifyException();
	return NewScopedJavaObject(JEnv, (jobjectArray)RetVal);
}