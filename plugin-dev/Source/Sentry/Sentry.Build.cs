// Copyright (c) 2022 Sentry. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Diagnostics;
using System.Text;
using System.Collections.Generic;
using System.Text.RegularExpressions;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public static class DateTimeExtensions
{
	// From UE4CMake 
	// Reference commit: b59317c2ee48f8eaeb5d0b1a5f837c3c2c3dd313
	// MIT License
	//
	// Copyright (c) 2020 Krazer
	//
	// 	Permission is hereby granted, free of charge, to any person obtaining a copy
	// 	of this software and associated documentation files (the "Software"), to deal
	// in the Software without restriction, including without limitation the rights
	// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	// copies of the Software, and to permit persons to whom the Software is
	// furnished to do so, subject to the following conditions:
	public static bool EqualsUpToSeconds(this DateTime dt1, DateTime dt2)
	{
		return dt1.Year == dt2.Year && dt1.Month == dt2.Month && dt1.Day == dt2.Day &&
		       dt1.Hour == dt2.Hour && dt1.Minute == dt2.Minute && dt1.Second == dt2.Second;
	}   
}
public class CMakeTargetInst
{
	// Based on UE4CMake with modifications 
	// Reference commit: b59317c2ee48f8eaeb5d0b1a5f837c3c2c3dd313
	// MIT License
	//
	// Copyright (c) 2020 Krazer
	//
	// 	Permission is hereby granted, free of charge, to any person obtaining a copy
	// 	of this software and associated documentation files (the "Software"), to deal
	// in the Software without restriction, including without limitation the rights
	// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	// copies of the Software, and to permit persons to whom the Software is
	// furnished to do so, subject to the following conditions:
	
	private string m_cmakeTargetPath;
	private string m_modulePath;
	private string m_targetName;
	private string m_targetPlatform;
	private string m_targetLocation;
	private string m_targetPath;
	private string m_cmakeArgs;

	private string m_buildDirectory;
	private string m_buildPath;
	private string m_generatedTargetPath;

	private string m_thirdPartyGeneratedPath;

	private string m_buildInfoFile;
	private string m_buildInfoPath;

	private bool m_forceBuild=false;
	private string m_forceBuildType;

	public CMakeTargetInst(string targetName, string targetPlatform, string targetLocation, string args)
	{
		m_targetName=targetName;
		m_targetPlatform=targetPlatform;
		m_targetLocation=targetLocation;
		
		Regex buildTypeRegex=new Regex(@"-DCMAKE_BUILD_TYPE=(\w*)");
		Match buildTypeMatch=buildTypeRegex.Match(args);

		if(buildTypeMatch.Success && (buildTypeMatch.Groups.Count > 1))
		{
			m_forceBuild=true;
			m_forceBuildType=buildTypeMatch.Groups[1].Value;
		}
		
		m_cmakeArgs=args;
	}
	
	private string GetBuildType(ReadOnlyTargetRules target)
	{
		string buildType = "Release";

		if(m_forceBuild)
			return m_forceBuildType;

		switch(target.Configuration)
		{
			case UnrealTargetConfiguration.Debug:
				buildType="Debug";
				break;
			default:
				break;
		}

		return buildType;
	}

	public bool Load(ReadOnlyTargetRules target, ModuleRules rules)
	{
		string buildType = GetBuildType(target);

		Console.WriteLine("Loading cmake target: "+target);

		m_cmakeTargetPath=Path.GetFullPath(rules.Target.ProjectFile.FullName);
		Console.WriteLine("Loading cmake path=: "+Directory .GetParent(m_cmakeTargetPath).FullName);
		m_cmakeTargetPath=Directory .GetParent(m_cmakeTargetPath).FullName+"/Plugins/sentry/sentry-native";

		m_modulePath=Path.GetFullPath(rules.ModuleDirectory);
		m_targetPath=Path.Combine(m_modulePath, m_targetLocation);

		m_thirdPartyGeneratedPath=Path.Combine(rules.Target.ProjectFile.Directory.FullName, "Intermediate", "CMakeTarget");
		m_generatedTargetPath=Path.Combine(m_thirdPartyGeneratedPath, m_targetName, m_targetPlatform);
		m_buildDirectory = "build";
		m_buildPath=Path.Combine(m_generatedTargetPath, m_buildDirectory);

		m_buildInfoFile="buildinfo_"+buildType+".output";
		m_buildInfoPath=Path.Combine(m_buildPath, m_buildInfoFile).Replace("\\", "/");
    
		if(!Directory.Exists(m_generatedTargetPath))
			Directory.CreateDirectory(m_generatedTargetPath);

		if(!Directory.Exists(m_buildPath))
			Directory.CreateDirectory(m_buildPath);

		var moduleBuilt = Build(target, rules, buildType);

		if(!moduleBuilt)
		{
			return false;
		}
		return true;
	}

	private bool Build(ReadOnlyTargetRules target, ModuleRules rules, string buildType)
	{
		string builtFile = Path.Combine(m_generatedTargetPath, buildType+".built");
		string projectCMakeLists=Path.GetFullPath(Path.Combine(m_targetPath, "CMakeLists.txt"));

		bool configCMake=true;

		//check if already built and CMakeList.txt not changed
		if(File.Exists(builtFile))
		{
			DateTime cmakeLastWrite=File.GetLastWriteTime(projectCMakeLists);
			string builtTimeString=System.IO.File.ReadAllText(builtFile);
			DateTime builtTime=DateTime.Parse(builtTimeString);

			if(builtTime.EqualsUpToSeconds(cmakeLastWrite))
				configCMake=false;
		}

		if(configCMake)
		{
			Console.WriteLine("Target "+m_targetName+" CMakeLists.txt out of date, rebuilding");

			var configureCommand = CreateCMakeConfigCommand(target, rules, m_buildPath, buildType);
			var configureCode = ExecuteCommandSync(configureCommand);

			if(configureCode!=0)
			{
				Console.WriteLine("Cannot configure CMake project. Exited with code: "
				                  +configureCode);
				return false;
			}
		}

		var buildCommand = CreateCMakeBuildCommand(m_buildPath, buildType);
		var buildCode = ExecuteCommandSync(buildCommand);

		if(buildCode!=0)
		{
			Console.WriteLine("Cannot build project. Exited with code: "+buildCode);
			return false;
		}
		else
		{
			if(configCMake)
			{
				DateTime cmakeLastWrite=File.GetLastWriteTime(projectCMakeLists);

				File.WriteAllText(builtFile, cmakeLastWrite.ToString());
			}
		}
		return true;
	}
	
	private string GetWindowsGeneratorName(WindowsCompiler compiler)
	{
		string generatorName="";

		switch(compiler)
		{
			case WindowsCompiler.Default:
				break;
			case WindowsCompiler.Clang:
				generatorName="NMake Makefiles";
				break;
			case WindowsCompiler.Intel:
				generatorName="NMake Makefiles";
				break;
#if !UE_5_0_OR_LATER
        case WindowsCompiler.VisualStudio2017:
            generatorName="Visual Studio 15 2017";
        break;
#endif//!UE_5_0_OR_LATER
#if !UE_5_4_OR_LATER
        case WindowsCompiler.VisualStudio2019:
            generatorName="Visual Studio 16 2019";
        break;
#endif//!UE_5_4_OR_LATER
			case WindowsCompiler.VisualStudio2022:
				generatorName="Visual Studio 17 2022";
				break;
		}

		return generatorName;
	}
	
#if UE_5_2_OR_LATER   // UE 5.2 and onwards
	private string GetWindowsGeneratorOptions(WindowsCompiler compiler, UnrealArch architecture)
#else
    private string GetWindowsGeneratorOptions(WindowsCompiler compiler, WindowsArchitecture architecture)
#endif
	{
		string generatorOptions="";

		if((compiler == WindowsCompiler.VisualStudio2022)
#if !UE_5_4_OR_LATER
            || (compiler == WindowsCompiler.VisualStudio2019)
#endif//!UE_5_4_OR_LATER 
#if !UE_5_0_OR_LATER
            || (compiler == WindowsCompiler.VisualStudio2017)
#endif//!UE_5_0_OR_LATER 
		  )
		{
#if UE_5_2_OR_LATER   // UE 5.2 and onwards
			if(architecture == UnrealArch.X64)
				generatorOptions="-A x64";
			else if(architecture == UnrealArch.Arm64)
				generatorOptions="-A ARM64";
#elif UE_5_0_OR_LATER // UE 5.0 to 5.1
			if(architecture == WindowsArchitecture.x64)
                generatorOptions="-A x64";
            else if(architecture == WindowsArchitecture.ARM64)
                generatorOptions="-A ARM64";

#else                 // Everything before UE 5.0
            if(architecture == WindowsArchitecture.x86)
                generatorOptions="-A Win32";
            else if(architecture == WindowsArchitecture.ARM32)
                generatorOptions="-A ARM";
#endif
		}
		return generatorOptions;
	}
	
	String GetGeneratorName(ReadOnlyTargetRules target)
    {
        if((target.Platform == UnrealTargetPlatform.Win64) 
#if !UE_5_0_OR_LATER
            || (target.Platform == UnrealTargetPlatform.Win32)
#endif//!UE_5_0_OR_LATER
            )
        {
            return GetWindowsGeneratorName(target.WindowsPlatform.Compiler);
        
        }
        else if (IsUnixPlatform(target.Platform))
        {
	        return "Unix Makefiles";
        }

        return "";
    }
	
	private string GetCMakeExe()
	{
		string program = "cmake";

		if((BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64) 
#if !UE_5_0_OR_LATER
            || (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win32)
#endif//!UE_5_0_OR_LATER
		  )
		{
			program+=".exe";
		}
		return program;
	}

	private string CreateCMakeConfigCommand(ReadOnlyTargetRules target, ModuleRules rules, string buildDirectory, string buildType)
	{
		string program = GetCMakeExe();
		string options = "";

		if((target.Platform == UnrealTargetPlatform.Win64) 
#if !UE_5_0_OR_LATER
            || (target.Platform == UnrealTargetPlatform.Win32)
#endif//!UE_5_0_OR_LATER
		  )
		{
			options=" -T host=x64";
		}

		if (rules.PublicDefinitions.Contains("USE_SENTRY_BREAKPAD=1"))
		{
			options += " -DSENTRY_BACKEND=breakpad";
		}

		if (rules.PublicDefinitions.Contains("SENTRY_BUILD_STATIC=1"))
		{
			options += " -DSENTRY_BUILD_SHARED_LIBS=Off";
		}

		string cmakeFile = Path.Combine(m_generatedTargetPath, "CMakeLists.txt");

		string buildToolchain = "";
		if (rules.PublicDefinitions.Contains("SENTRY_BUILD_XBOX_TOOLCHAIN=1"))
		{
			buildToolchain = "-DCMAKE_TOOLCHAIN_FILE=" + Path.Combine(m_generatedTargetPath,"toolchains/xbox/gxdk_xs_toolchain.cmake");
		}
 
		var installPath = m_thirdPartyGeneratedPath;

		var arguments = " -G \""+GetGeneratorName(target)+"\""+
		                " -S \""+m_targetLocation+"\""+
		                " -B \""+buildDirectory+"\""+
		                buildToolchain+
		                " -DCMAKE_BUILD_TYPE="+GetBuildType(target)+
		                " -DCMAKE_INSTALL_PREFIX=\""+installPath+"\""+
		                options+
		                " "+m_cmakeArgs;

		Console.WriteLine("CMakeTarget calling cmake with: "+arguments);

		return program+arguments;
	}

    private string CreateCMakeBuildCommand(string buildDirectory, string buildType)
    {
        return GetCMakeExe()+" --build \""+buildDirectory+"\" --config "+buildType;
    }

    private string CreateCMakeInstallCommand(string buildDirectory, string buildType)
    {
        return GetCMakeExe()+" --build \""+buildDirectory+"\" --target install --config "+buildType;
    }

    private Tuple<string, string> GetExecuteCommandSync()
    {
        string cmd = "";
        string options = "";

        if((BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64) 
#if !UE_5_0_OR_LATER
            || (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win32)
#endif//!UE_5_0_OR_LATER
            )
        {
            cmd="cmd.exe";
            options="/c ";
        }
        else if(IsUnixPlatform(BuildHostPlatform.Current.Platform)) 
        {
            cmd="bash";
            options="-c ";
        }
        return Tuple.Create(cmd, options);
    }

    private int ExecuteCommandSync(string command)
    {
        var cmdInfo=GetExecuteCommandSync();

        if(IsUnixPlatform(BuildHostPlatform.Current.Platform)) 
        {
            command=" \""+command.Replace("\"", "\\\"")+" \"";
        }

        Console.WriteLine("Calling: "+cmdInfo.Item1+" "+cmdInfo.Item2+command);

        var processInfo = new ProcessStartInfo(cmdInfo.Item1, cmdInfo.Item2+command)
        {
            CreateNoWindow=true,
            UseShellExecute=false,
            RedirectStandardError=true,
            RedirectStandardOutput=true,
            WorkingDirectory=m_modulePath
        };

        StringBuilder outputString = new StringBuilder();
        Process p = Process.Start(processInfo);

        p.OutputDataReceived+=(sender, args) => {outputString.Append(args.Data); Console.WriteLine(args.Data);};
        p.ErrorDataReceived+=(sender, args) => {outputString.Append(args.Data); Console.WriteLine(args.Data);};
        p.BeginOutputReadLine();
        p.BeginErrorReadLine();
        p.WaitForExit();

        if(p.ExitCode != 0)
        {
             Console.WriteLine(outputString);
        }
        return p.ExitCode;
    }

    private bool IsUnixPlatform(UnrealTargetPlatform platform) {
        return platform == UnrealTargetPlatform.Linux || platform == UnrealTargetPlatform.Mac;
    }
}

public class Sentry : ModuleRules
{
	[CommandLine("-forceBreakpad")]
	public bool bForceBreakpad = false;
	
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
				"Json",
				"HTTP"
				// ... add private dependencies that you statically link with here ...	
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
		
		UnrealTargetPlatform XboxXPlatform;
		UnrealTargetPlatform.TryParse("XSX", out XboxXPlatform);
		UnrealTargetPlatform XboxOnePlatform;
		UnrealTargetPlatform.TryParse("XB1", out XboxOnePlatform);
			
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));
			
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Android"));
			
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));
			
			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");

			if (bForceBreakpad)
			{
				PublicDefinitions.Add("USE_SENTRY_BREAKPAD=1");
			}
		}
#if UE_5_0_OR_LATER
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
#else
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxAArch64)
#endif
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));
			
			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));
			
			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		}
		else if (Target.Platform == XboxXPlatform || Target.Platform == XboxOnePlatform)
		{
			// Note: We may need a new xbox platform include
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));
			
			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
			PublicDefinitions.Add("SENTRY_BUILD_XBOX_TOOLCHAIN=1");
		}
		else
		{
			Console.WriteLine("Sentry Unreal SDK does not support platform: " + Target.Platform);
		}
	

		if (PublicDefinitions.Contains("USE_SENTRY_NATIVE=1"))
		{
			var cmakeTargetPath = Path.GetFullPath(Target.ProjectFile.FullName);
			var targetLocation = Directory.GetParent(cmakeTargetPath).FullName + "/Plugins/sentry/sentry-native";

			CMakeTargetInst cmakeTarget =
				new CMakeTargetInst("sentry-native", Target.Platform.ToString(), targetLocation, "");
			cmakeTarget.Load(Target, this);

			string intermediatePath =
				Path.Combine(Target.ProjectFile.Directory.FullName, "Intermediate", "CMakeTarget", "sentry-native");
			
			Console.WriteLine("Adding include path: "+targetLocation+"/include");
			PublicIncludePaths.Add(targetLocation + "/include");
			
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				string buildPath = Path.Combine(intermediatePath, "Win64", "build");
				if(Target.Configuration == UnrealTargetConfiguration.Debug)
				{
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Debug", "sentry.lib"));
				}
				else
				{ 
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Release", "sentry.lib"));
				}

				if (!PublicDefinitions.Contains("USE_SENTRY_BREAKPAD=1"))
				{
					string crashpadBuildPath = Path.Combine(buildPath, "crashpad_build");
					if(Target.Configuration == UnrealTargetConfiguration.Debug)
					{
						RuntimeDependencies.Add(Path.Combine(crashpadBuildPath, "handler", "Debug", "crashpad_handler.exe"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Debug", "crashpad_handler_lib.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Debug", "crashpad_client.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Debug", "crashpad_compat.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Debug", "crashpad_minidump.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Debug", "crashpad_snapshot.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt", "Debug", "crashpad_getopt.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium", "Debug", "mini_chromium.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Debug", "crashpad_zlib.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Debug", "crashpad_tools.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Debug", "crashpad_util.lib"));
					}
					else
					{
						RuntimeDependencies.Add(Path.Combine(crashpadBuildPath, "handler","Release", "crashpad_handler.exe"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Release", "crashpad_handler_lib.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Release", "crashpad_client.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Release", "crashpad_compat.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Release", "crashpad_minidump.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Release", "crashpad_snapshot.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt", "Release", "crashpad_getopt.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium", "Release", "mini_chromium.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Release", "crashpad_zlib.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Release", "crashpad_tools.lib"));
						PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Release", "crashpad_util.lib"));
					}
				}
			}
#if UE_5_0_OR_LATER
			else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
#else
			else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxAArch64)
#endif
			{
				string buildPath = Path.Combine(intermediatePath, "Linux", "build");
				if(Target.Configuration == UnrealTargetConfiguration.Debug)
				{
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Debug", "sentry.a"));
				}
				else
				{
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Release", "sentry.a"));
				}
				
				string crashpadBuildPath = Path.Combine(buildPath, "crashpad_build", "handler");
				if(Target.Configuration == UnrealTargetConfiguration.Debug)
				{
					RuntimeDependencies.Add(Path.Combine(crashpadBuildPath, "handler", "Debug", "crashpad_handler"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Debug", "crashpad_handler_lib.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Debug", "crashpad_client.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Debug", "crashpad_compat.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Debug", "crashpad_minidump.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Debug", "crashpad_snapshot.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt", "Debug", "crashpad_getopt.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium", "Debug", "mini_chromium.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Debug", "crashpad_zlib.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Debug", "crashpad_tools.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Debug", "crashpad_util.a"));
				}
				else
				{
					RuntimeDependencies.Add(Path.Combine(crashpadBuildPath, "handler", "Release", "crashpad_handler"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Release", "crashpad_handler_lib.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Release", "crashpad_client.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Release", "crashpad_compat.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Release", "crashpad_minidump.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Release", "crashpad_snapshot.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt", "Release", "crashpad_getopt.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium", "Release", "mini_chromium.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Release", "crashpad_zlib.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Release", "crashpad_tools.a"));
					PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Release", "crashpad_util.a"));
				}
			}
			else if (Target.Platform == XboxXPlatform || Target.Platform == XboxOnePlatform)
			{
				string buildPath = Path.Combine(intermediatePath, "Xbox", "build");
				if(Target.Configuration == UnrealTargetConfiguration.Debug)
				{
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Debug", "sentry.lib"));
				}
				else
				{
					PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Release", "sentry.lib"));
				}
			}
			else
			{
				Console.WriteLine("Platform not currently supported: " + Target.Platform);
			}
		}
	}
}
