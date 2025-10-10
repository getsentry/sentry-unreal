#!/usr/bin/env python3
# Copyright (c) 2025 Sentry. All Rights Reserved.

"""
Sentry debug symbols upload script for Unreal Engine.
This script handles automatic upload of debug symbols after building.
"""

import sys
import os
import subprocess
import re
from pathlib import Path


def log(message):
    """Print a log message with Sentry prefix."""
    print(f"Sentry: {message}")


def parse_ini_value(file_path, section, key):
    """
    Parse a value from an INI file.

    Args:
        file_path: Path to the INI file
        section: Section name (e.g., '/Script/Sentry.SentrySettings')
        key: Key name to find

    Returns:
        The value if found, None otherwise
    """
    if not os.path.exists(file_path):
        return None

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            in_section = False
            for line in f:
                line = line.strip()

                # Check for section header
                if line.startswith('[') and line.endswith(']'):
                    section_name = line[1:-1]
                    in_section = (section_name == section)
                    continue

                # If we're in the right section, look for the key
                if in_section and '=' in line:
                    line_key, line_value = line.split('=', 1)
                    if line_key.strip() == key:
                        return line_value.strip()

    except Exception as e:
        log(f"Error parsing INI file {file_path}: {e}")

    return None


def parse_properties_file(file_path, key):
    """
    Parse a value from a properties file.

    Args:
        file_path: Path to the properties file
        key: Key name to find

    Returns:
        The value if found, None otherwise
    """
    if not os.path.exists(file_path):
        return None

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line.startswith('#') or not line:
                    continue

                if '=' in line:
                    line_key, line_value = line.split('=', 1)
                    if line_key.strip() == key:
                        return line_value.strip()

    except Exception as e:
        log(f"Error parsing properties file {file_path}: {e}")

    return None


def check_target_enabled(config_path, setting_key, target_value):
    """
    Check if a specific target type or configuration is enabled.

    Args:
        config_path: Path to DefaultEngine.ini
        setting_key: The setting key (e.g., 'EnableBuildTargets')
        target_value: The value to check (e.g., 'bEnableGame=False')

    Returns:
        True if enabled, False if disabled
    """
    enabled_list = parse_ini_value(config_path, '/Script/Sentry.SentrySettings', setting_key)

    if not enabled_list:
        return True  # If not specified, assume enabled

    # Remove parentheses if present
    enabled_list = enabled_list.strip('()')

    # Check if the disable flag is present
    if target_value in enabled_list:
        return False

    return True


def get_cli_executable(plugin_dir):
    """
    Get the path to the sentry-cli executable for the host OS.

    Args:
        plugin_dir: Path to the plugin directory

    Returns:
        Path to sentry-cli or None if not found
    """
    # Determine CLI binary based on host OS (where build is running), not target platform
    # This is important for cross-compilation scenarios (e.g., building Linux on Windows)
    import platform

    host_system = platform.system()

    if host_system == 'Windows':
        cli_name = 'sentry-cli-Windows-x86_64.exe'
    elif host_system == 'Darwin':
        cli_name = 'sentry-cli-Darwin-universal'
    elif host_system == 'Linux':
        cli_name = 'sentry-cli-Linux-x86_64'
    else:
        return None

    cli_path = os.path.join(plugin_dir, 'Source', 'ThirdParty', 'CLI', cli_name)

    if os.path.exists(cli_path):
        return cli_path

    return None


def main():
    """Main entry point for the symbol upload script."""

    # Parse command line arguments
    if len(sys.argv) != 7:
        log("Error: Invalid number of arguments")
        log(f"Usage: {sys.argv[0]} <TargetPlatform> <TargetName> <TargetType> <TargetConfiguration> <ProjectFile> <PluginDir>")
        return 1

    target_platform = sys.argv[1]
    target_name = sys.argv[2]
    target_type = sys.argv[3]
    target_config = sys.argv[4]
    project_file = sys.argv[5]
    plugin_dir = sys.argv[6]

    log("Start debug symbols upload")

    # Skip editor builds
    if target_type == "Editor":
        log("Automatic symbols upload is not required for Editor target. Skipping...")
        return 0

    # Skip Android builds (handled by Gradle plugin)
    if target_platform == "Android":
        log("Debug symbols upload for Android is handled by Sentry's Gradle plugin (if enabled)")
        return 0

    # Determine project directory
    project_dir = os.path.dirname(os.path.abspath(project_file))
    project_binaries_path = os.path.join(project_dir, 'Binaries', target_platform)
    plugin_binaries_path = os.path.join(plugin_dir, 'Source', 'ThirdParty', target_platform)

    config_path = os.path.join(project_dir, 'Config', 'DefaultEngine.ini')

    # Check if upload is enabled
    upload_symbols = parse_ini_value(config_path, '/Script/Sentry.SentrySettings', 'UploadSymbolsAutomatically')

    # Check environment variable override
    env_override = os.environ.get('SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY')
    if env_override:
        upload_symbols = env_override
        log(f"Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '{env_override}'")

    if upload_symbols != "True":
        log("Automatic symbols upload is disabled in plugin settings. Skipping...")
        return 0

    # Check if target type is enabled
    if not check_target_enabled(config_path, 'EnableBuildTargets', f'bEnable{target_type}=False'):
        log(f"Automatic symbols upload is disabled for target type {target_type}. Skipping...")
        return 0

    # Check if build configuration is enabled
    if not check_target_enabled(config_path, 'EnableBuildConfigurations', f'bEnable{target_config}=False'):
        log(f"Automatic symbols upload is disabled for build configuration {target_config}. Skipping...")
        return 0

    # Determine include sources flag
    include_sources = parse_ini_value(config_path, '/Script/Sentry.SentrySettings', 'IncludeSources')
    cli_args = []
    if include_sources == "True":
        cli_args.append('--include-sources')

    # Get CLI log level
    cli_log_level = parse_ini_value(config_path, '/Script/Sentry.SentrySettings', 'DiagnosticLevel')
    if not cli_log_level:
        cli_log_level = "info"

    # Get sentry-cli executable (based on host OS)
    cli_exec = get_cli_executable(plugin_dir)
    if not cli_exec:
        log("Sentry CLI is missing. Skipping...")
        return 0

    # Make CLI executable on Unix systems
    if os.name != 'nt':
        try:
            os.chmod(cli_exec, 0o755)
        except Exception as e:
            log(f"Warning: Failed to set execute permission on sentry-cli: {e}")

    # Check for sentry.properties file
    properties_file = os.path.join(project_dir, 'sentry.properties')

    log(f"Looking for properties file '{properties_file}'")

    if os.path.exists(properties_file):
        log("Properties file found. Starting upload...")

        # Validate required properties
        project_name = parse_properties_file(properties_file, 'defaults.project')
        if not project_name:
            log("Error: Project name is not set. Skipping...")
            return 0

        org_name = parse_properties_file(properties_file, 'defaults.org')
        if not org_name:
            log("Error: Project organization is not set. Skipping...")
            return 0

        auth_token = parse_properties_file(properties_file, 'auth.token')
        if not auth_token:
            log("Error: Auth token is not set. Skipping...")
            return 0

        # Set environment variable for sentry-cli
        os.environ['SENTRY_PROPERTIES'] = properties_file
    else:
        log("Properties file not found. Falling back to environment variables.")

        # Validate required environment variables
        if not os.environ.get('SENTRY_PROJECT'):
            log("Error: SENTRY_PROJECT env var is not set. Skipping...")
            return 0

        if not os.environ.get('SENTRY_ORG'):
            log("Error: SENTRY_ORG env var is not set. Skipping...")
            return 0

        if not os.environ.get('SENTRY_AUTH_TOKEN'):
            log("Error: SENTRY_AUTH_TOKEN env var is not set. Skipping...")
            return 0

    # Build the upload command
    upload_cmd = [
        cli_exec,
        'debug-files',
        'upload',
        *cli_args,
        '--log-level', cli_log_level,
        project_binaries_path,
        plugin_binaries_path
    ]

    try:
        # Execute the upload command
        result = subprocess.run(upload_cmd, check=False)

        log("Upload finished")
        return result.returncode

    except Exception as e:
        log(f"Error executing sentry-cli: {e}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
