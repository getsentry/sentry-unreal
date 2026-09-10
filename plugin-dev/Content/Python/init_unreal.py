# Copyright (c) 2026 Sentry. All Rights Reserved.

# The AI toolset registry is an optional editor plugin. When it isn't enabled its Python package is
# absent, so skip registration instead of failing the editor's start-up scripts.
try:
    from sentry_unreal import toolsets
    from sentry_unreal import skills  # noqa: F401 - importing registers the skills
except ImportError:
    pass
else:
    toolsets._registration.register()
