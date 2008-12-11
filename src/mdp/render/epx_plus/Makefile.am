# MDP Render Plugin: EPX Plus renderer.

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

lib_LTLIBRARIES = mdp_render_epx_plus.la

mdp_render_epx_plus_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_render_epx_plus_la_SOURCES = \
		mdp_render_epx_plus_plugin.c \
		mdp_render_epx_plus.cpp

noinst_HEADERS = \
		mdp_render_epx_plus_plugin.h \
		mdp_render_epx_plus.hpp
