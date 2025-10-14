#!/usr/bin/env python3

import sys
import os
import platform
import subprocess
import time


def log(message):
    print(f"Sentry: {message}", flush=True)


def parse_config_value(file_path, key, section=None):
    if not os.path.exists(file_path):
        return None

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            in_section = (section is None)

            for line in f:
                line = line.strip()

                if line.startswith('#') or line.startswith(';') or not line:
                    continue

                if section and line.startswith('[') and line.endswith(']'):
                    section_name = line[1:-1]
                    in_section = (section_name == section)
                    continue

                if in_section and '=' in line:
                    line_key, line_value = line.split('=', 1)
                    if line_key.strip() == key:
                        return line_value.strip()

    except Exception as e:
        log(f"Error parsing config file {file_path}: {e}")

    return None


def check_target_enabled(config_path, target_setting_key, target_name):
    targets_list = parse_config_value(config_path, target_setting_key, '/Script/Sentry.SentrySettings')

    if not targets_list:
        return True  # If not specified, assume enabled

    targets_list = targets_list.strip('()')

    # Check if the disabled flag for specified target is present in list (e.g., 'bEnableGame=False' or 'bEnableDevelopment=False')
    disabled_taget_flag = f'bEnable{target_name}=False'
    if disabled_taget_flag in targets_list:
        return False

    return True


def get_cli_executable(plugin_dir):
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


def run_cmd_with_retry(cmd, max_retries=3, retry_delay_sec=2):
    for attempt in range(max_retries):
        result = subprocess.run(cmd, check=False, capture_output=True, text=True)

        if result.returncode != 0 and "os error 32" in result.stderr:
            if attempt < max_retries - 1:
                log(f"Debug file is locked, retrying in {retry_delay_sec} seconds... (attempt {attempt + 1}/{max_retries})")
                time.sleep(retry_delay_sec)
                continue
            else:
                log("Error: Debug files are still locked after multiple retries")

        break

    return result


def main():
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
    upload_symbols = parse_config_value(config_path, 'UploadSymbolsAutomatically', '/Script/Sentry.SentrySettings')

    # Check environment variable override
    env_override = os.environ.get('SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY')
    if env_override:
        upload_symbols = env_override
        log(f"Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '{env_override}'")

    # Check if upload is enabled
    if not upload_symbols or upload_symbols.lower() != "true":
        log("Automatic symbols upload is disabled in plugin settings. Skipping...")
        return 0

    # Check if target type is enabled
    if not check_target_enabled(config_path, 'EnableBuildTargets', target_type):
        log(f"Automatic symbols upload is disabled for target type {target_type}. Skipping...")
        return 0

    # Check if build configuration is enabled
    if not check_target_enabled(config_path, 'EnableBuildConfigurations', target_config):
        log(f"Automatic symbols upload is disabled for build configuration {target_config}. Skipping...")
        return 0

    # Determine include sources flag
    include_sources = parse_config_value(config_path, 'IncludeSources', '/Script/Sentry.SentrySettings')
    cli_args = []
    if include_sources == "True":
        cli_args.append('--include-sources')

    # Get CLI log level
    cli_log_level = parse_config_value(config_path, 'DiagnosticLevel', '/Script/Sentry.SentrySettings')
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
        project_name = parse_config_value(properties_file, 'defaults.project')
        if not project_name:
            log("Error: Project name is not set. Skipping...")
            return 0

        org_name = parse_config_value(properties_file, 'defaults.org')
        if not org_name:
            log("Error: Project organization is not set. Skipping...")
            return 0

        auth_token = parse_config_value(properties_file, 'auth.token')
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

    # Construct the upload command
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
        # Execute the upload command with retry logic to avoid linker file locking issues
        # See https://github.com/getsentry/sentry-unreal/issues/542
        result = run_cmd_with_retry(upload_cmd)

        # Output stdout/stderr
        if result.stdout:
            print(result.stdout, end='')
        if result.stderr:
            print(result.stderr, end='')

        log("Upload finished")
        return result.returncode

    except Exception as e:
        log(f"Error executing sentry-cli: {e}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
