# ============================================================================
# SENTRY UNREAL ENGINE SDK MAKEFILE
# ============================================================================
# Convenience targets for building the Unreal Engine sample project.
# Run 'make help' to see all available commands.
# ============================================================================

.DEFAULT_GOAL := help

# ============================================================================
# CONFIGURATION
# ============================================================================

# Unreal Engine root — auto-detected from Epic Games Launcher manifest,
# can be overridden: make build-macos UNREAL_ENGINE_ROOT=/path/to/UE_5.x
UNREAL_ENGINE_ROOT ?= $(shell cat ~/Library/Application\ Support/Epic/UnrealEngineLauncher/LauncherInstalled.dat 2>/dev/null | jq -r '.InstallationList[-1].InstallLocation // empty' 2>/dev/null)

PROJECT := $(CURDIR)/sample/SentryPlayground.uproject
ARCHIVE_DIR := $(CURDIR)/sample/dist
RUN_UAT := $(UNREAL_ENGINE_ROOT)/Engine/Build/BatchFiles/RunUAT.sh

# ============================================================================
# FRAMEWORK STAGING
# ============================================================================

## Download and stage the SentryObjC framework into ThirdParty directories
#
# Runs scripts/download-cocoa.sh to fetch the SentryObjC xcframework
# and copy iOS/Mac slices into plugin-dev/Source/ThirdParty/.
.PHONY: stage-framework
stage-framework:
	rm -rf modules/sentry-cocoa
	bash scripts/download-cocoa.sh modules/sentry-cocoa plugin-dev/Source/ThirdParty/Mac

# ============================================================================
# BUILDING
# ============================================================================

## Build macOS target
#
# Builds the SentryPlayground sample project for macOS.
# Requires Unreal Engine to be installed.
.PHONY: build-macos
build-macos: _check-engine
	"$(RUN_UAT)" BuildCookRun \
		-project="$(PROJECT)" \
		-archivedirectory="$(ARCHIVE_DIR)" \
		-platform=Mac \
		-clientconfig=Development \
		-build -skipcook -skipstage -nop4

## Build iOS target
#
# Builds the SentryPlayground sample project for iOS.
# Note: code-signing will fail without a dev team configured — that is expected.
# Compile + link succeeding is the meaningful check.
.PHONY: build-ios
build-ios: _check-engine
	"$(RUN_UAT)" BuildCookRun \
		-project="$(PROJECT)" \
		-archivedirectory="$(ARCHIVE_DIR)" \
		-platform=IOS \
		-clientconfig=Development \
		-build -skipcook -skipstage -nop4

# ============================================================================
# HELPERS
# ============================================================================

.PHONY: _check-engine
_check-engine:
	@if [ -z "$(UNREAL_ENGINE_ROOT)" ]; then \
		echo "error: UNREAL_ENGINE_ROOT is not set and could not be auto-detected."; \
		echo "       Set it manually: make build-macos UNREAL_ENGINE_ROOT=/path/to/UE_5.x"; \
		exit 1; \
	fi
	@if [ ! -f "$(RUN_UAT)" ]; then \
		echo "error: RunUAT.sh not found at $(RUN_UAT)"; \
		echo "       Check UNREAL_ENGINE_ROOT (currently: $(UNREAL_ENGINE_ROOT))"; \
		exit 1; \
	fi

# ============================================================================
# HELP
# ============================================================================

## Show this help message
#
# Displays a formatted list of all available make targets with descriptions.
.PHONY: help
help:
	@echo "=============================================="
	@echo " SENTRY UNREAL SDK — DEVELOPMENT COMMANDS"
	@echo "=============================================="
	@echo ""
	@awk 'BEGIN { summary = ""; n = 0; maxlen = 0 } \
	/^## / { summary = substr($$0, 4); next } \
	/^\.PHONY: / && summary != "" { \
		for (i = 2; i <= NF; i++) { \
			if (substr($$i, 1, 1) != "_") { \
				targets[n] = $$i; \
				summaries[n] = summary; \
				if (length($$i) > maxlen) maxlen = length($$i); \
				n++; \
			} \
		} \
		summary = ""; next \
	}  \
	END { \
		for (i = 0; i < n; i++) { \
			printf "\033[36m%-*s\033[0m %s\n", maxlen, targets[i], summaries[i]; \
		} \
	}' $(MAKEFILE_LIST)
	@echo ""
	@echo "Engine: $(or $(UNREAL_ENGINE_ROOT),(not detected — set UNREAL_ENGINE_ROOT))"
	@echo ""
