# MDP: Mega Drive Plugins
# Interface Specification Version 0.1.0

# Render plugins

AUTOMAKE_OPTIONS = subdir-objects

# Render plugins
SUBDIRS = \
		scanline \
		scanline_25 \
		scanline_50 \
		interpolated \
		interpolated_scanline \
		interpolated_scanline_25 \
		interpolated_scanline_50 \
		scale2x

if GENS_X86_ASM
SUBDIRS += \
		hq2x \
		2xsai
endif # GENS_X86_ASM
