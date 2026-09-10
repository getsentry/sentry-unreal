# Copyright (c) 2026 Sentry. All Rights Reserved.

from toolset_registry.registration import Registration
from sentry_unreal.toolsets import sentry

_registration = Registration([
    sentry.SentryTools,
])
