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
		scale2x \
		scale3x \
		scale4x

if GENS_X86_ASM
SUBDIRS += \
		hq2x \
		hq3x \
		hq4x \
		2xsai
endif # GENS_X86_ASM
