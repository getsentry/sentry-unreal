// Copyright (c) 2022 Sentry. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class Sentry : ModuleRules
{
	public Sentry(ReadOnlyTargetRules Target) : base(Target)
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
				"Projects",
				"Json"
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

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));
		}

		// Additional routine for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
		}

		// Additional routine for Desktop platforms
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Linux)
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

		// Additional routine for Linux
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			LoadThirdPartyLibrary("libsentry", Target);
			LoadCrashpadHandler("crashpad_handler", Target);
		}
	}

	public void LoadThirdPartyLibrary(string libname, ReadOnlyTargetRules Target)
	{
		string StaticLibExtension = string.Empty;
		string DynamicLibExtension = string.Empty;
		string DebugSymbolsExtension = string.Empty;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			StaticLibExtension = ".lib";
			DynamicLibExtension = ".dll";
			DebugSymbolsExtension = ".pdb";
		}
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			StaticLibExtension = ".a";
			DynamicLibExtension = ".dylib";
			DebugSymbolsExtension = ".dylib.dSYM";
		}
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			StaticLibExtension = ".a";
			DynamicLibExtension = ".so";
			DebugSymbolsExtension = ".dbg.so";
		}

		// Link libraries
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "../ThirdParty", Target.Platform.ToString());

		string BinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries", Target.Platform.ToString()));

		string SourceStaticLibPath = Path.Combine(ThirdPartyPath, "lib", libname + StaticLibExtension);
		string SourceDynamicLibPath = Path.Combine(ThirdPartyPath, "bin", libname + DynamicLibExtension);
		string SourceSymbolsPath = Path.Combine(ThirdPartyPath, "bin", libname + DebugSymbolsExtension);
		string BinariesDynamicLibPath = Path.Combine(BinariesPath, libname + DynamicLibExtension);
		string BinariesSymbolsPath = Path.Combine(BinariesPath, libname + DebugSymbolsExtension);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(SourceStaticLibPath);
			PublicDelayLoadDLLs.Add(libname + DynamicLibExtension);

			RuntimeDependencies.Add(BinariesSymbolsPath, SourceSymbolsPath);

			// Make debug symbols writeable to avoid issues with UGS Binary Zips during sync (Perforce is usually read-only by default)
			File.SetAttributes(BinariesSymbolsPath, File.GetAttributes(BinariesSymbolsPath) & ~FileAttributes.ReadOnly);
		}
		if (Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add(BinariesDynamicLibPath);
		}

		RuntimeDependencies.Add(BinariesDynamicLibPath, SourceDynamicLibPath);

		// Make Sentry library writeable to avoid issues with UGS Binary Zips during sync (Perforce is usually read-only by default)
		File.SetAttributes(BinariesDynamicLibPath, File.GetAttributes(BinariesDynamicLibPath) & ~FileAttributes.ReadOnly);
	}

	public void LoadCrashpadHandler(string HandlerName, ReadOnlyTargetRules Target)
	{
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "../ThirdParty", Target.Platform.ToString());

		string BinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries", Target.Platform.ToString()));

		string SourceHandlerPath = Path.Combine(ThirdPartyPath, "bin", HandlerName);
		string BinariesHandlerPath = Path.Combine(BinariesPath, HandlerName);

		RuntimeDependencies.Add(BinariesHandlerPath, SourceHandlerPath);

		// Make crashpad handler writeable to avoid issues with UGS Binary Zips during sync (Perforce is usually read-only by default)
		File.SetAttributes(BinariesHandlerPath, File.GetAttributes(BinariesHandlerPath) & ~FileAttributes.ReadOnly);
	}
}
