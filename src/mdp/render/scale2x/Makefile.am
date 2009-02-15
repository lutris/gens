# MDP Render Plugin: Scale2x renderer.

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

lib_LTLIBRARIES = mdp_render_scale2x.la

mdp_render_scale2x_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scale2x_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scale2x_la_SOURCES = \
		mdp_render_scale2x_plugin.c \
		mdp_render_scale2x.c \
		scale2x.c \
		scalebit_2x.c \
		scalebit_2x_mmx.c

noinst_HEADERS = \
		mdp_render_scale2x_plugin.h \
		mdp_render_scale2x_icon.h \
		mdp_render_scale2x.h \
		scale2x.h \
		scalebit_2x.h \
		scalebit_2x_mmx.h
