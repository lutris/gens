# MDP: Mega Drive Plugins
# Interface Specification Version 1.0.0

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
		scale4x \
		epx \
		epx_plus \
		blargg_ntsc

if GENS_X86_ASM
SUBDIRS += \
		2xsai \
		super_eagle \
		super_2xsai
if !GENS_OS_MACOSX	# nasm doesn't support PIC on MacOS X.
SUBDIRS += \
		hq2x \
		hq3x \
		hq4x
endif #!GENS_OS_MACOSX
endif # GENS_X86_ASM
