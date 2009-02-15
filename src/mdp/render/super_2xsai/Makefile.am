# MDP Render Plugin: 2xSaI renderer.

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

lib_LTLIBRARIES = mdp_render_super_2xsai.la

mdp_render_super_2xsai_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_super_2xsai_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

NASM += -I@srcdir@/../

mdp_render_super_2xsai_la_SOURCES = \
		mdp_render_super_2xsai_plugin.c \
		mdp_render_super_2xsai.c \
		mdp_render_super_2xsai_16_x86.asm

noinst_HEADERS = \
		mdp_render_super_2xsai_plugin.h \
		mdp_render_super_2xsai.h \
		mdp_render_super_2xsai_x86.h

mdp_render_super_2xsai_la_LIBADD = \
		$(ASMFILES)

mdp_render_super_2xsai_la_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
