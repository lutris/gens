# MDP: Mega Drive Plugins
# Interface Specification Version 1.0.0

# Miscellaneous plugins

AUTOMAKE_OPTIONS = foreign subdir-objects

# Miscellaneous
SUBDIRS = \
		game_genie		\
		vdp_layer_options	\
		sgens			\
		ips_patcher

if HAVE_DBUS
if HAVE_DBUS_GLIB
SUBDIRS += irc_reporter
endif # HAVE_DBUS_GLIB
endif # HAVE_DBUS

if GENS_OS_LINUX
if GENS_UI_GTK
SUBDIRS += vdp_debugger
endif # GENS_UI_GTK
endif # GENS_OS_LINUX
