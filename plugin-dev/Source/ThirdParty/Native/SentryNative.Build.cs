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

public class CMakeTargetInst
{
	// Based on UE4CMake with modifications 
	// Reference commit: b59317c2ee48f8eaeb5d0b1a5f837c3c2c3dd313
	// MIT License
	//
	// Copyright (c) 2020 Krazer
	//
	// Permission is hereby granted, free of charge, to any person obtaining a copy
	// of this software and associated documentation files (the "Software"), to deal
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
	private string m_installDirectory;
	private string m_installPath;
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
#if UE_5_1_OR_LATER
		Regex buildTypeRegex=new Regex(@"-DCMAKE_BUILD_TYPE=(\w*)");
		Match buildTypeMatch=buildTypeRegex.Match(args);

		if(buildTypeMatch.Success && (buildTypeMatch.Groups.Count > 1))
		{
			m_forceBuild=true;
			m_forceBuildType=buildTypeMatch.Groups[1].Value;
		}
#endif
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
		m_installDirectory = "install";
		m_installPath=Path.Combine(m_generatedTargetPath, m_installDirectory);

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

			if(builtTime.Equals(cmakeLastWrite))
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

		var installCommand = CreateCMakeInstallCommand(m_buildPath, m_installPath, buildType);
		var installCode = ExecuteCommandSync(installCommand);

		if(installCode!=0)
		{
			Console.WriteLine("Cannot perform install. Exited with code: "+buildCode);
			return false;
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
#else // Everything before UE 5.0
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


		if(target.Platform.ToString() == "XSX"
			|| (target.Platform == UnrealTargetPlatform.Win64) 
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

		if(BuildHostPlatform.Current.Platform.ToString() == "XSX"
			|| (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64) 
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

		string buildToolchain = "";
		if(target.Platform.ToString() == "XSX")
		{
			buildToolchain = "-DCMAKE_TOOLCHAIN_FILE=" + Path.Combine(m_generatedTargetPath,"toolchains/xbox/gxdk_xs_toolchain.cmake");
		}
 
		var installPath = m_thirdPartyGeneratedPath;

		var arguments = " -G \""+GetGeneratorName(target)+"\""+
			buildToolchain+
			" -DCMAKE_BUILD_TYPE="+GetBuildType(target)+
			" -DCMAKE_INSTALL_PREFIX=\""+installPath+"\""+
			" -DSENTRY_BUILD_SHARED_LIBS=OFF"+
			" -DSENTRY_SDK_NAME=sentry.native.unreal"+
			options+
			" "+m_cmakeArgs+
			" -B \""+buildDirectory+"\""+
			" -S \""+m_targetLocation+"\"";

		Console.WriteLine("CMakeTarget calling cmake with: "+arguments);

		return program+arguments;
	}

	private string CreateCMakeBuildCommand(string buildDirectory, string buildType)
	{
		return GetCMakeExe()+" --build \""+buildDirectory+"\" --config "+buildType;
	}

	private string CreateCMakeInstallCommand(string buildDirectory, string installDirectory, string buildType)
	{
		return GetCMakeExe()+" --install \""+buildDirectory+"\" --prefix \""+installDirectory+"\" --config "+buildType;
	}

	private Tuple<string, string> GetExecuteCommandSync()
	{
		string cmd = "";
		string options = "";

		if(BuildHostPlatform.Current.Platform.ToString() == "XSX"
			|| (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64) 
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

	private bool IsUnixPlatform(UnrealTargetPlatform platform)
	{
		return platform == UnrealTargetPlatform.Linux || platform == UnrealTargetPlatform.Mac;
	}
}

public class SentryNative : ModuleRules
{
	public SentryNative(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string projectIntermediatePath = Path.Combine(Target.ProjectFile.Directory.FullName, "Intermediate");
		string platformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			var nativeSourcePath = Path.Combine(PluginDirectory, "Source", "ThirdParty", "Native", "sentry-native");

			PublicIncludePaths.Add(Path.Combine(nativeSourcePath, "include"));

			CMakeTargetInst cmakeTarget = new CMakeTargetInst("sentry-native", Target.Platform.ToString(), nativeSourcePath, "");
			cmakeTarget.Load(Target, this);

			string installPath = Path.Combine(projectIntermediatePath, "CMakeTarget", "sentry-native", Target.Platform.ToString(), "install");

			RuntimeDependencies.Add(Path.Combine(platformBinariesPath, "crashpad_handler.exe"), Path.Combine(installPath, "bin", "crashpad_handler.exe"));

			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_handler_lib.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_client.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_compat.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_minidump.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_snapshot.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_getopt.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "mini_chromium.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_zlib.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_tools.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "crashpad_util.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(installPath, "lib", "sentry.lib"));
		}
	}
}
