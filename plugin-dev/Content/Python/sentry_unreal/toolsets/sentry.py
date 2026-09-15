# Copyright (c) 2026 Sentry. All Rights Reserved.

import unreal

import toolset_registry


def _subsystem():
    subsystem = unreal.get_engine_subsystem(unreal.SentrySubsystem)
    if subsystem is None:
        raise RuntimeError('Sentry subsystem is not available.')
    return subsystem


def _settings():
    # Not the class default object - the SDK reads its own instance, and changes to the default
    # object would never reach it.
    settings = unreal.SentryEditorLibrary.get_active_settings()
    if settings is None:
        raise RuntimeError('Sentry settings are not available.')
    return settings


@unreal.uclass()
class SentryTools(unreal.ToolsetDefinition):
    """Inspects and configures the Sentry SDK in the running editor process."""

    @toolset_registry.tool_call
    @staticmethod
    def is_sentry_enabled() -> bool:
        """Checks whether the Sentry SDK is initialized and capturing events.

        Returns:
            True if the SDK is running.
        """
        return _subsystem().is_enabled()

    @toolset_registry.tool_call
    @staticmethod
    def get_dsn_source() -> str:
        """Reports where the DSN used by the SDK in the editor comes from, without disclosing its
        value. An editor DSN overrides the project DSN in the editor, and the project DSN
        overrides the SENTRY_DSN environment variable.

        Returns:
            One of 'editor_dsn', 'project_dsn', 'environment_variable', or 'none' if no DSN is
            available.
        """
        return {
            unreal.SentryDsnSource.EDITOR_DSN: 'editor_dsn',
            unreal.SentryDsnSource.DSN: 'project_dsn',
            unreal.SentryDsnSource.ENVIRONMENT_VARIABLE: 'environment_variable',
        }.get(unreal.SentryEditorLibrary.get_dsn_source(), 'none')

    @toolset_registry.tool_call
    @staticmethod
    def set_dsn(dsn: str) -> None:
        """Sets the project DSN and persists it to the project's default engine config, leaving every
        other entry in that file untouched. The SDK keeps running with its previous settings until
        it is reinitialized. Fails if an editor DSN is set, since it would keep overriding the
        project DSN in the editor.
        This should ONLY be called after getting explicit direction or permission from the user.

        Args:
            dsn: The DSN to store. Must not be empty.
        """
        if not dsn:
            raise ValueError('dsn must not be empty.')

        if unreal.SentryEditorLibrary.get_dsn_source() == unreal.SentryDsnSource.EDITOR_DSN:
            raise RuntimeError(
                'An editor DSN is set and overrides the project DSN in the editor, so the new DSN '
                'would not take effect here. The editor DSN has to be changed or cleared in the '
                'plugin settings first.')

        settings = _settings()
        settings.set_editor_property('dsn', dsn)
        unreal.SentryEditorLibrary.save_setting_to_config(settings, 'Dsn')

    @toolset_registry.tool_call
    @staticmethod
    def reinitialize_sentry() -> bool:
        """Restarts the SDK so that changes to the plugin settings take effect, shutting down the
        current session first if one is running. Avoids having to restart the editor.
        This should ONLY be called after getting explicit direction or permission from the user.

        Returns:
            True if the SDK is running once reinitialization completes.
        """
        subsystem = _subsystem()
        subsystem.initialize()
        return subsystem.is_enabled()

    @toolset_registry.tool_call
    @staticmethod
    def capture_test_message(message: str) -> str:
        """Captures a message event from the editor, for confirming that events reach Sentry.

        Args:
            message: The message to send. Must not be empty.

        Returns:
            The ID of the captured event, usable to look the event up in Sentry.
        """
        if not message:
            raise ValueError('message must not be empty.')

        subsystem = _subsystem()
        if not subsystem.is_enabled():
            raise RuntimeError('Sentry SDK is not running, so no event can be captured.')

        return subsystem.capture_message(message)
