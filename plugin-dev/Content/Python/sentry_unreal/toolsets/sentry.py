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


# Settings with dedicated handling that the generic setting tools must not touch: the DSN has its
# own tools, and the debug symbol upload credentials belong in sentry.properties, not the project
# config.
_GUARDED_SETTINGS = {'dsn', 'authtoken', 'projectname', 'orgname'}


def _enum_entry_name(value):
    for name in dir(type(value)):
        if not name.startswith('_') and getattr(type(value), name, None) == value:
            return name
    return str(value)


def _parse_setting_value(current, value):
    if isinstance(current, bool):
        lowered = value.strip().lower()
        if lowered in ('true', '1'):
            return True
        if lowered in ('false', '0'):
            return False
        raise ValueError(f"'{value}' is not a valid boolean.")
    if isinstance(current, unreal.EnumBase):
        normalized = value.replace('_', '').lower()
        for name in dir(type(current)):
            entry = getattr(type(current), name, None)
            if isinstance(entry, type(current)) and name.replace('_', '').lower() == normalized:
                return entry
        raise ValueError(f"'{value}' is not an entry of {type(current).__name__}.")
    if isinstance(current, int):
        return int(value)
    if isinstance(current, float):
        return float(value)
    if isinstance(current, str):
        return value
    raise ValueError('Composite settings are not supported.')


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
    def get_setting(name: str) -> str:
        """Reads the current value of a Sentry plugin setting from the running editor. The DSN and
        the debug symbol upload credentials are never disclosed and are refused here. Composite
        settings (structs and arrays) are not supported.

        Args:
            name: The setting's property name as it appears in the project config, for example
                'EnableTracing'.

        Returns:
            The value in text form, for example 'True', '0.5', or an enum entry name.
        """
        if name.replace('_', '').lower() in _GUARDED_SETTINGS:
            raise ValueError(
                f"'{name}' is not disclosed here: the DSN has dedicated tools, and debug symbol "
                'upload credentials are secrets.')

        value = _settings().get_editor_property(name)

        if isinstance(value, bool):
            return 'True' if value else 'False'
        if isinstance(value, unreal.EnumBase):
            return _enum_entry_name(value)
        if isinstance(value, (int, float, str)):
            return str(value)

        raise ValueError('Composite settings are not supported.')

    @toolset_registry.tool_call
    @staticmethod
    def set_setting(name: str, value: str) -> None:
        """Sets a Sentry plugin setting on the running editor and persists it to the project's
        default engine config, leaving every other entry in that file untouched. Runtime settings
        take effect once the SDK is reinitialized; settings the plugin reads at build time take
        effect on the next C++ build. The DSN and the debug symbol upload credentials have
        dedicated handling and are refused here. Composite settings (structs and arrays) are not
        supported.
        This should ONLY be called after getting explicit direction or permission from the user.

        Args:
            name: The setting's property name as it appears in the project config, for example
                'EnableTracing'.
            value: The new value in text form, for example 'True', '0.5', or an enum entry name.
        """
        if name.replace('_', '').lower() in _GUARDED_SETTINGS:
            raise ValueError(
                f"'{name}' is not handled here: the DSN has dedicated tools, and debug symbol "
                'upload credentials belong in sentry.properties.')

        settings = _settings()
        settings.set_editor_property(name, _parse_setting_value(settings.get_editor_property(name), value))

        if not unreal.SentryEditorLibrary.save_setting_to_config(settings, name):
            raise RuntimeError(f"Setting '{name}' was changed but could not be persisted.")

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
