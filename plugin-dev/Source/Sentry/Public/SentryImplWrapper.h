// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"

/**
 * A templatized wrapper to simplify the Unreal <-> Sentry bridging layer.
 */
template<class Interface, class Unreal>
class TSentryImplWrapper
{
public:
	/** Pure virtual destructor to ensure class is abstract. */
	virtual ~TSentryImplWrapper() = 0;

	/**
	 * Method to instantiate an object with a native implementation.
	 *
	 * @param InObject The native implementation.
	 * @return Returns an instance of the Unreal object if instantiation is valid; otherwise nullptr.
	 */
	static Unreal* Create(TSharedPtr<Interface> InObject)
	{
		if (InObject)
		{
			Unreal* OutObject = NewObject<Unreal>();
			StaticCast<TSentryImplWrapper<Interface, Unreal>*>(OutObject)->NativeImpl = InObject;
			return OutObject;
		}
		else
		{
			return nullptr;
		}
	}

	/** Retrieves the underlying native implementation. */
	TSharedPtr<Interface> GetNativeObject() const { return NativeImpl; }

protected:
	TSharedPtr<Interface> NativeImpl;
};

template<class Interface, class Unreal>
TSentryImplWrapper<Interface, Unreal>::~TSentryImplWrapper() {}
