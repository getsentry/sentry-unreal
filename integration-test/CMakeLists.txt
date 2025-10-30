cmake_minimum_required(VERSION 3.20)
project(SentryUnrealIntegrationTests NONE)

include(FetchContent)

# Fetch PowerShell modules for Sentry API integration
FetchContent_Declare(
    app-runner
    GIT_REPOSITORY https://github.com/getsentry/app-runner.git
    GIT_TAG 288991e2096a39bda7aaa40f1baf2a65952a5f23
)

FetchContent_MakeAvailable(app-runner)

# Generate test configuration with app-runner path
set(configFile "${CMAKE_CURRENT_SOURCE_DIR}/TestConfig.local.ps1")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/TestConfig.ps1.in"
    "${configFile}"
    @ONLY
)

message(STATUS "Integration test environment configured")
message(STATUS "  app-runner path: ${app-runner_SOURCE_DIR}")
message(STATUS "  Config file: ${configFile}")
