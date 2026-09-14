#!/usr/bin/env python3

import os
import platform
import shutil
import sys

# Don't leave a __pycache__ dir behind in the plugin's Scripts dir
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import upload_debug_symbols


def make_executable(path):
    # Some plugin distribution methods don't preserve file permissions
    if os.path.isfile(path):
        os.chmod(path, 0o755)


def stage_binary(src, dst):
    # Epic obfuscates any extra binaries when pre-building the plugin, so they are restored
    # to the location the packaged plugin expects to ensure the FAB version works correctly
    if not os.path.isfile(src) or os.path.isfile(dst):
        return

    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy2(src, dst)


def run_windows_host_steps(target_platform, plugin_dir):
    third_party_dir = os.path.join(plugin_dir, 'Source', 'ThirdParty')
    binaries_dir = os.path.join(plugin_dir, 'Binaries', target_platform)

    if target_platform == 'Win64':
        crashpad_dir = os.path.join(third_party_dir, 'Win64', 'Crashpad', 'bin')
        stage_binary(os.path.join(crashpad_dir, 'crashpad_handler.exe'), os.path.join(binaries_dir, 'crashpad_handler.exe'))
        stage_binary(os.path.join(crashpad_dir, 'crashpad_wer.dll'), os.path.join(binaries_dir, 'crashpad_wer.dll'))
        stage_binary(os.path.join(third_party_dir, 'Win64', 'Sentry.CrashReporter.exe'), os.path.join(binaries_dir, 'Sentry.CrashReporter.exe'))

    # Cross-compilation for Linux on Windows
    if target_platform == 'Linux':
        stage_binary(os.path.join(third_party_dir, 'Linux', 'Crashpad', 'bin', 'crashpad_handler'), os.path.join(binaries_dir, 'crashpad_handler'))
        stage_binary(os.path.join(third_party_dir, 'Linux', 'Sentry.CrashReporter'), os.path.join(binaries_dir, 'Sentry.CrashReporter'))


def run_mac_host_steps(target_platform, plugin_dir):
    third_party_dir = os.path.join(plugin_dir, 'Source', 'ThirdParty')
    binaries_dir = os.path.join(plugin_dir, 'Binaries', target_platform)

    if target_platform == 'Mac':
        stage_binary(os.path.join(third_party_dir, 'Mac', 'Cocoa', 'bin', 'SentryObjC.dylib'), os.path.join(binaries_dir, 'SentryObjC.dylib'))

    make_executable(os.path.join(third_party_dir, 'CLI', 'sentry-cli-Darwin-universal'))
    make_executable(os.path.join(binaries_dir, 'sentry-crash'))
    make_executable(os.path.join(binaries_dir, 'Sentry.CrashReporter.app', 'Contents', 'MacOS', 'Sentry.CrashReporter'))


def run_linux_host_steps(target_platform, plugin_dir):
    third_party_dir = os.path.join(plugin_dir, 'Source', 'ThirdParty')
    binaries_dir = os.path.join(plugin_dir, 'Binaries', target_platform)

    make_executable(os.path.join(third_party_dir, 'CLI', 'sentry-cli-Linux-x86_64'))
    make_executable(os.path.join(binaries_dir, 'crashpad_handler'))
    make_executable(os.path.join(binaries_dir, 'sentry-crash'))
    make_executable(os.path.join(binaries_dir, 'Sentry.CrashReporter'))


def main():
    target_platform, target_name, target_type, target_config, project_file, plugin_dir, engine_dir = sys.argv[1:8]

    host_system = platform.system()

    if host_system == 'Windows':
        run_windows_host_steps(target_platform, plugin_dir)
    elif host_system == 'Darwin':
        run_mac_host_steps(target_platform, plugin_dir)
    elif host_system == 'Linux':
        run_linux_host_steps(target_platform, plugin_dir)

    return upload_debug_symbols.run(target_platform, target_name, target_type, target_config, project_file, plugin_dir, engine_dir)


if __name__ == '__main__':
    sys.exit(main())
