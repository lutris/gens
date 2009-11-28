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
