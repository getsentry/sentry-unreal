// Copyright (c) 2022 Sentry. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class SentrySDK : ModuleRules
{
	public SentrySDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		// Additional routine for iOS
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/IOS"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", "../ThirdParty/IOS/Sentry.embeddedframework.zip"));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "SentrySDK_IOS_UPL.xml"));
		}

		// Additional routine for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "SentrySdk_Android_UPL.xml"));
		}

		// Additional routine for Desktop platforms
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty", Target.Platform.ToString(), "include"));

			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Desktop"));
		}

		// Additional routine for Windows
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			LoadThirdPartyLibrary("sentry", Target);
			LoadCrashpadHandler("crashpad_handler.exe", Target);
		}

		// Additional routine for Mac
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			LoadThirdPartyLibrary("libsentry", Target);
			LoadCrashpadHandler("crashpad_handler", Target);
		}
	}

	public void LoadThirdPartyLibrary(string libname, ReadOnlyTargetRules Target)
	{
		string StaticLibExtension = string.Empty;
		string DynamicLibExtension = string.Empty;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			StaticLibExtension = ".lib";
			DynamicLibExtension = ".dll";
		}
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			StaticLibExtension = ".a";
			DynamicLibExtension = ".dylib";
		}

		// Link libraries
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "../ThirdParty", Target.Platform.ToString());

		string StaticLibrariesPath = Path.Combine(ThirdPartyPath, "lib");
		string DynamicLibrariesPath = Path.Combine(ThirdPartyPath, "bin");

		// Copy dynamic libraries to Binaries folder
		string BinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries", Target.Platform.ToString()));

		if (!Directory.Exists(BinariesPath))
		{
			Directory.CreateDirectory(BinariesPath);
		}

		if (!File.Exists(Path.Combine(BinariesPath, libname + DynamicLibExtension)))
		{
			File.Copy(Path.Combine(DynamicLibrariesPath, libname + DynamicLibExtension), Path.Combine(BinariesPath, libname + DynamicLibExtension), true);
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(StaticLibrariesPath, libname + StaticLibExtension));
			PublicDelayLoadDLLs.Add(libname + DynamicLibExtension);
		}
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, libname + DynamicLibExtension));
		}

		RuntimeDependencies.Add(Path.Combine(BinariesPath, libname + DynamicLibExtension));
	}

	public void LoadCrashpadHandler(string HandlerName, ReadOnlyTargetRules Target)
	{
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "../ThirdParty", Target.Platform.ToString());

		string HandlerPath = Path.Combine(ThirdPartyPath, "bin");

		// Copy crashpad handler executable to Binaries folder
		string BinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries", Target.Platform.ToString()));

		if (!Directory.Exists(BinariesPath))
		{
			Directory.CreateDirectory(BinariesPath);
		}

		if (!File.Exists(Path.Combine(BinariesPath, HandlerName)))
		{
			File.Copy(Path.Combine(HandlerPath, HandlerName), Path.Combine(BinariesPath, HandlerName), true);
		}

		RuntimeDependencies.Add(Path.Combine(BinariesPath, HandlerName));
	}
}