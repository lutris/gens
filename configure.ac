#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.5])
AC_INIT(gens, 2.16.7, gerbilsoft@verizon.net)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Gens/GS version numbers.
AC_DEFINE(GENS_VERSION_WIN32, [2,16,7,0], Gens Win32 Version Number)
AC_DEFINE(GENS_GS_VERSION, "r7", Gens/GS Version Number)

# Automake build features.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Initialize automake.
AM_INIT_AUTOMAKE([1.10])

# Disable the Fortran check.
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])

# Work around autoconf's default "-g -O2" CFLAGS and CXXFLAGS.
if test "$CFLAGS" = "-g -O2"; then
	CFLAGS=""
fi
if test "$CXXFLAGS" = "-g -O2"; then
	CXXFLAGS=""
fi

# Check for programs.
AM_PROG_AS
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_LIBTOOL

# Enable C99.
AC_PROG_CC_C99

# Check operating system.
nasm_debug=""
yasm_debug=""
case $host in
	i?86*mingw32*)
		os=win32;
		obj_format=win32;
		AC_DEFINE(GENS_OS_WIN32, 1, Win32 operating system)
		AC_DEFINE(GENS_PLATFORM, "Win32", Platform ID)
		ldflags_win32="-static-libgcc ${ldflags_win32}"
		nasm_debug="-g"
		yasm_debug="-g stabs";
		cflags_debug="-gstabs"
		cflags_optimization="-march=i686"
		;;
	i?86*cygwin*)
		os=win32;
		obj_format=win32;
		AC_DEFINE(GENS_OS_WIN32, 1, Win32 operating system)
		AC_DEFINE(GENS_PLATFORM, "Win32", Platform ID)
		cflags_win32="-mno-cygwin -mwin32 ${cflags_win32}";
		ldflags_win32="-mno-cygwin -mwin32 -static-libgcc ${ldflags_win32}"
		nasm_debug="-g"
		yasm_debug="-g stabs";
		cflags_debug="-gstabs"
		cflags_optimization="-march=i686"
		;;
	i?86*darwin*)
		os=macosx;
		obj_format=macho;
		AC_DEFINE(GENS_OS_MACOSX, 1, MacOS X operating system)
		AC_DEFINE(GENS_OS_UNIX, 1, Unix operating system)
		AC_DEFINE(GENS_PLATFORM, "MacOS X", Platform ID)
		nasm_debug="-g -F null";
		yasm_debug="-g null";
		cflags_debug="-ggdb"
		cflags_optimization="-march=i686"
		;;
	i?86*linux*)
		os=linux;
		obj_format=elf32;
		AC_DEFINE(GENS_OS_LINUX, 1, Linux operating system)
		AC_DEFINE(GENS_OS_UNIX, 1, Unix operating system)
		AC_DEFINE(GENS_PLATFORM, "Linux", Platform ID)
		nasm_debug="-g -F dwarf";
		yasm_debug="-g dwarf2";
		cflags_debug="-ggdb"
		cflags_optimization="-march=i686"
		;;
	i?86*haiku*)
		os=haiku
		obj_format=elf32;
		AC_DEFINE(GENS_OS_HAIKU, 1, Haiku operating system)
		AC_DEFINE(GENS_PLATFORM, "Haiku", Platform ID)
		nasm_debug="-g -F dwarf";
		yasm_debug="-g dwarf2";
		cflags_debug="-ggdb";
		cflags_optimization="-march=i686"
		;;
	i?86*)
		os=unix;
		obj_format=elf32;
		AC_DEFINE(GENS_OS_UNIX, 1, Unix operating system)
		AC_DEFINE(GENS_PLATFORM, "Unix", Platform ID)
		nasm_debug="-g -F dwarf";
		yasm_debug="-g dwarf2";
		cflags_debug="-ggdb"
		cflags_optimization="-march=i686"
		;;
	x86_64*)
		obj_format=elf64;
		AC_MSG_ERROR([64-bit is currently not supported.]);
		nasm_debug="-g -F dwarf";
		yasm_debug="-g dwarf2";
		cflags_debug="-ggdb"
		;;
	*)
		AC_MSG_ERROR([Unsupported system architecture: $host]);
		;;
esac

# Operating system
AM_CONDITIONAL([GENS_OS_LINUX],  [test x$os = xlinux])
AM_CONDITIONAL([GENS_OS_HAIKU],  [test x$os = xhaiku])
AM_CONDITIONAL([GENS_OS_MACOSX], [test x$os = xmacosx])
AM_CONDITIONAL([GENS_OS_UNIX],   [test x$os = xlinux -o x$os = xmacosx -o x$os = xunix])
AM_CONDITIONAL([GENS_OS_WIN32],  [test x$os = xwin32])

# Linker format
AM_CONDITIONAL([GENS_OBJ_WIN32], [test x$obj_format = xwin32])
AM_CONDITIONAL([GENS_OBJ_ELF32], [test x$obj_format = xelf32])
AM_CONDITIONAL([GENS_OBJ_ELF64], [test x$obj_format = xelf64])
AM_CONDITIONAL([GENS_OBJ_MACHO], [test x$obj_format = xmacho])

# Debugging symbols and warnings.
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Build with debugging symbols. [[default=yes]]]), enable_debug=$enableval, enable_debug=yes)
if test x$enable_debug = xyes -o x$enable_debug = xfull; then
	# Debugging symbols enabled.
	cflags_optimization="-O0"
	AC_DEFINE(GENS_DEBUG, 1, Enable debugging)	
else
	# Debugging disabled.
	nasm_debug=""
	yasm_debug=""
	cflags_debug=""
	cflags_optimization="$cflags_optimization -O2"
fi
AM_CONDITIONAL([GENS_DEBUG], [test x$enable_debug = xyes -o x$enable_debug = xfull])

# BEGIN: Unix/Linux (and MacOS X for now)

if test x$os = xunix -o x$os = xlinux -o x$os = xmacosx; then

# Unix or Unix-like operating system.

# Check for libX11.
PKG_CHECK_MODULES([X11], [x11], [has_libX11=yes], [has_libX11=no])
if test x$has_libX11 = xyes; then
	AC_DEFINE(HAVE_LIBX11, 1, Define to 1 if you have the `X11' library (-lX11).)
fi

# TODO: Allow UI selection later on.
#AC_ARG_ENABLE(ui, AC_HELP_STRING([--enable-ui=[[ui]]], [Enable either the 'gtk' or 'qt4' UI [[default=gtk]]]), ui=$enableval, ui=gtk)
#if test x$ui != xgtk && test x$ui != xqt4; then
#	ui=gtk
#fi
ui=gtk

# Check which UI was selected.

if test x$ui = xgtk; then

# GTK+
# Note: If AM_PATH_GTK_2_0 isn't defined, GTK+ is assumed to be installed.
GTK_VERSION=2.4.0
ifdef(
	[AM_PATH_GTK_2_0],
	[
		AM_PATH_GTK_2_0(
			$GTK_VERSION,
			:,
			AC_MSG_ERROR([[*** GTK+ version $GTK_VERSION or later not found!]])
		)
	],
	[AC_MSG_WARN([[*** AM_PATH_GTK_2_0 is not defined. Assuming GTK+ is installed.]])]
)
AC_DEFINE(GENS_UI_GTK, 1, Enable the GTK+ UI)

# TODO: Disable deprecated GTK+ features.
GTK_CFLAGS="$GTK_CFLAGS -DGTK_DISABLE_DEPRECATED -DDISABLE_DEPRECATED -DGSEAL_ENABLE"

#elif test x$ui = xqt4; then
#
# Qt4
#PKG_CHECK_MODULES([Qt4], QtCore >= 4.2.0 QtGui >= 4.2.0)
#AC_DEFINE(GENS_UI_QT4, 1, Enable the Qt4 UI)
#Qt4_moc_location=`pkg-config QtCore --variable=moc_location`
#Qt4_uic_location=`pkg-config QtCore --variable=uic_location`
#AC_SUBST(Qt4_moc_location)
#AC_SUBST(Qt4_uic_location)
#
fi

# SDL
# Note: If AM_PATH_SDL isn't defined, SDL is assumed to be installed.
SDL_VERSION=1.1.3
ifdef(
	[AM_PATH_SDL],
	[
		AM_PATH_SDL(
			$SDL_VERSION,
			:,
			AC_MSG_ERROR([[*** SDL version $SDL_VERSION or later not found!]])
		)
	],
	[AC_MSG_WARN([[*** AM_PATH_SDL is not defined. Assuming SDL is installed.]])]
)

# END: Unix / Linux (and MacOS X for now)

elif test x$os = xwin32; then

# Win32
AC_DEFINE(GENS_UI_WIN32, 1, Enable the Win32 UI)

# Check for a Win32 resource compiler.
RC=""
if test x${host_alias} != x; then
	# Check with a host alias.
	AC_PATH_PROG(RC, ${host_alias}-windres, no)
fi;
if test x$RC = x -o x$RC = xno; then
	# Check without a host alias.
	AC_PATH_PROG(RC, windres, no)
	if test x$RC = xno; then
		AC_MSG_ERROR([*** Win32 resource compiler not found!])
	fi
fi
AC_SUBST(RC)

fi

AM_CONDITIONAL([GENS_UI_GTK], [test x$ui = xgtk])
#AM_CONDITIONAL([GENS_UI_QT4], [test x$ui = xqt4])
AM_CONDITIONAL([GENS_UI_WIN32], [test x$os = xwin32])

# X11 conditional and substitutions.
AM_CONDITIONAL([HAVE_LIBX11], [test x$has_libX11 = xyes])
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

# Check for nasm.
AC_ARG_WITH(nasm, AC_HELP_STRING([--with-nasm[[=nasm]]], [Select nasm or yasm [[default=nasm]]]), with_nasm=$withval, with_nasm=nasm)

# nasm optimization level.
AC_ARG_ENABLE(nasm_opt, AC_HELP_STRING([--enable-nasm-opt[[=99]]], [Set nasm optimization level [[default=99]]]), enable_nasm_opt=$enableval, enable_nasm_opt=99)

# Set output format.
if test x$obj_format = xwin32; then
	nasm_obj_format="-f win32"
elif test x$obj_format = xwin64; then
	nasm_obj_format="-f win64"
elif test x$obj_format = xmacho; then
	nasm_obj_format="-f macho -D __GCC2"
elif test x$obj_format = xelf32; then
	nasm_obj_format="-f elf -D __GCC2"
elif test x$obj_format = xelf64; then
	nasm_obj_format="-f elf64 -D __GCC2"
fi

if test x$with_nasm != x -a x$with_nasm != xnasm -a x$with_nasm != xyasm; then
	AC_MSG_WARN([Unsupported nasm-compatible assembler $with_nasm; using nasm instead.])
	with_nasm=nasm
fi

if test x$with_nasm = xyasm; then
	# yasm specified as nasm-compatible assembler.
	AC_PATH_PROG(YASM, yasm, no)
	if test x$YASM = xno; then
		# yasm wasn't found.
		AC_MSG_ERROR([*** yasm not found!])
	fi
	
# Set the NASM path to YASM.
	NASM="${YASM} ${nasm_obj_format} ${yasm_debug} -I\$(top_srcdir)/src/ -w"
elif test x$with_nasm = xnasm; then
	# nasm specified as nasm-compatible assembler.
	AC_PATH_PROG(NASM, nasm, no)
	if test x$NASM = xno; then
		# nasm wasn't found.
		AC_MSG_ERROR([*** nasm not found!])
	fi
	
	# nasm optimization level
	if test x$enable_nasm_opt = xyes; then
		enable_nasm_opt=99
	elif test x$enable_nasm_opt = xno; then
		enable_nasm_opt=1;
	fi
	
	# Set the NASM path to NASM.
	NASM="${NASM} -O${enable_nasm_opt} ${nasm_obj_format} ${nasm_debug} -I\$(top_srcdir)/src/ -w-orphan-labels"
fi

AC_ARG_ENABLE(x86_asm, AC_HELP_STRING([--enable-x86-asm], [Enable x86 assembly optimizations [[default=yes]]]), enable_x86_asm=$enableval, enable_x86_asm=yes)
if test x$enable_x86_asm = xyes; then
	AC_DEFINE(GENS_X86_ASM, 1, Enable x86 assembly optimizations)
fi
AM_CONDITIONAL([GENS_X86_ASM], [test x$enable_x86_asm = xyes])

AC_ARG_ENABLE(debugger, AC_HELP_STRING([--enable-debugger], [Enable the built-in debugger [[default=yes]]]), enable_debugger=$enableval, enable_debugger=yes)
if test x$enable_debugger = xyes; then
	AC_DEFINE(GENS_DEBUGGER, 1, Enable the built-in debugger)
fi
AM_CONDITIONAL([GENS_DEBUGGER], [test x$enable_debugger = xyes])

# Physical CD-ROM drive support. (Linux/Win32 only.)
AC_ARG_ENABLE(cdrom, AC_HELP_STRING([--enable-cdrom], [Enable CD-ROM drive access for SegaCD games [[default=yes]]]), enable_cdrom=$enableval, enable_cdrom=yes)
if test x$enable_cdrom = xyes; then
	if test x$os = xlinux -o x$os = xwin32; then
		AC_DEFINE(GENS_CDROM, 1, Enable CD-ROM drive access for SegaCD games)
	else
		enable_cdrom=no
		AC_MSG_WARN([Physical CD-ROM drive support is currently only available on Linux and Windows. Disabling.])
	fi
fi
AM_CONDITIONAL([GENS_CDROM], [test x$enable_cdrom = xyes])

# SegaCD MP3 support.
AC_ARG_ENABLE(mp3, AC_HELP_STRING([--enable-mp3], [Enable MP3 support for SegaCD games [[default=yes]]]), enable_mp3=$enableval, enable_mp3=yes)
if test x$enable_mp3 = xyes; then
	AC_DEFINE(GENS_MP3, 1, Enable MP3 support for SegaCD games)
fi
AM_CONDITIONAL([GENS_MP3], [test x$enable_mp3 = xyes])

AC_ARG_ENABLE(win32_console, AC_HELP_STRING([--enable-win32-console], [Enable console output on Windows [[default=no]]]), enable_win32_console=$enableval, enable_win32_console=no)
if test x$os = xwin32; then
	if test x$enable_win32_console = xyes; then
		# Enable console output on Win32.
		AC_DEFINE(GENS_WIN32_CONSOLE, 1, Enable console output on Windows)
	fi
else
	# Not Windows - ignore this option.
	if test x$enable_win32_console = xyes; then
		# No, you can't enable a Windows console if you're not building for Windows.
		# Not yours.
		AC_MSG_WARN([Not building for Windows; ignoring --enable-win32-console])
		enable_win32_console=no
	fi
fi
AM_CONDITIONAL([GENS_WIN32_CONSOLE], [test x$enable_win32_console = xyes])

AC_ARG_WITH(opengl, AC_HELP_STRING([--without-opengl], [Build without OpenGL support [[default=no]]]), with_opengl=$withval, with_opengl=yes)
GL_LIBS=""
if test x$with_opengl = xyes; then
	AC_MSG_CHECKING(for OpenGL support)
	with_opengl=no
	if test x$os = xwin32; then
		AC_MSG_WARN([OpenGL support is not currently available on Win32. Disabling.])
	elif test x$os = xhaiku; then
		AC_MSG_WARN([OpenGL support is not currently available on Haiku. Disabling.])
	else
		AC_TRY_COMPILE([
			#ifndef __APPLE__
			#include <GL/gl.h>
			#else
			#include <OpenGL/gl.h>
			#endif
		],[
			GLint g;
		],[
			with_opengl=yes
		])      
		AC_MSG_RESULT($with_opengl)
		if test x$with_opengl = xyes; then
			if test x$os = xmacosx; then
				GL_LIBS="-framework OpenGL"
			else
				GL_LIBS="-lGL"
			fi
			AC_DEFINE(GENS_OPENGL, 1, OpenGL support)
		fi
	fi
fi
AM_CONDITIONAL([GENS_OPENGL], [test x$with_opengl = xyes])

###
# zlib
###

AC_ARG_WITH(zlib, AC_HELP_STRING([--without-zlib], [Build without zlib support [[default=no]]]), with_zlib=$withval, with_zlib=yes)
AC_ARG_WITH(internal_zlib, AC_HELP_STRING([--with-internal-zlib], [Use internal zlib [[default=no]]]), with_internal_zlib=$withval, with_internal_zlib=no)

# If internal zlib was enabled, but zlib was disabled, enable zlib.
# The user may have thought that the plain zlib option was for shared zlib.
if test x$with_internal_zlib = xyes; then
	with_zlib=yes
fi

if test x$with_zlib = xno; then
	# zlib disabled; disable internal zlib library.
	with_internal_zlib=no
else
	# zlib enabled; check if internal zlib library is eanbled.
if test x$with_internal_zlib = xno; then
AC_MSG_CHECKING(for system zlib)
	with_zlib=no
	AC_TRY_COMPILE([
		#include <zlib.h>
	],[
		gzFile gzfd;
	],[
		with_zlib=yes
	])
	AC_MSG_RESULT($with_zlib)
	if test x$with_zlib = xno; then
		# External zlib not found. Use the internal version.
		with_zlib=yes
		with_internal_zlib=yes
	fi
fi

fi

if test x$with_zlib = xyes; then
AC_DEFINE(GENS_ZLIB, 1, Compression support via zlib)
if test x$with_internal_zlib = xyes; then
AC_DEFINE(GENS_ZLIB_INTERNAL, 1, Use internal zlib instead of system zlib)
fi # GENS_ZLIB_INTERNAL
fi # GENS_ZLIB
AM_CONDITIONAL([GENS_ZLIB], [test x$with_zlib = xyes])
AM_CONDITIONAL([GENS_ZLIB_INTERNAL], [test x$with_internal_zlib = xyes])

###
# libpng
###

AC_ARG_WITH(png, AC_HELP_STRING([--without-png], [Build without PNG support [[default=no]]]), with_png=$withval, with_png=yes)
AC_ARG_WITH(internal_png, AC_HELP_STRING([--with-internal-png], [Use internal libpng [[default=no]]]), with_internal_png=$withval, with_internal_png=no)

# If internal libpng was enabled, but PNG was disabled, enable PNG.
# The user may have thought that the plain libpng option was for shared libpng.
if test x$with_internal_png = xyes; then
	with_png=yes
fi

PNG_LIBS=""
with_png_dlopen=no
if test x$with_png = xno; then
	# libpng disabled; disable internal libpng library.
	with_internal_png=no
else
	# libpng enabled; check if internal libpng library is eanbled.
if test x$with_internal_png = xno; then
AC_MSG_CHECKING(for system libpng)
	with_png=no
	AC_TRY_COMPILE([
		#include <png.h>
	],[
		png_structp png_ptr;
	],[
		with_png=yes
	])
	AC_MSG_RESULT($with_png)
	
	if test x$with_png = xno; then
		# External libpng not found. Use the internal version.
		with_png=yes
		with_internal_png=yes
		with_png_dlopen=no
	else
		# External libpng found.
		
		# Check if libpng should be linked at compile time.
		# On Win32, do not link at compile time; instead, libpng will be dlopen()'d.
		AC_MSG_CHECKING(if libpng should be linked at compile time)
		if test x$os != xwin32; then
			# Not Win32. Link at compile time.
			AC_MSG_RESULT(yes);
			PNG_LIBS="-lpng"
			with_png_dlopen=no
		else
			# Win32. Use dlopen().
			AC_MSG_RESULT(no);
			with_png_dlopen=yes
			AC_DEFINE(GENS_PNG_DLOPEN, 1, [Use dlopen(), or LoadLibrary() on Win32, to load libpng.])
		fi
	fi
fi

AC_SUBST(PNG_LIBS)
AM_CONDITIONAL([GENS_PNG_DLOPEN], [test x$with_png_dlopen = xyes])

fi

if test x$with_png = xyes; then
	AC_DEFINE(GENS_PNG, 1, PNG support via libpng)
	if test x$with_internal_png = xyes; then
		AC_DEFINE(GENS_PNG_INTERNAL, 1, Use internal libpng instead of system libpng)
	fi # GENS_PNG_INTERNAL
fi # GENS_PNG

AM_CONDITIONAL([GENS_PNG], [test x$with_png = xyes])
AM_CONDITIONAL([GENS_PNG_INTERNAL], [test x$with_internal_png = xyes])


###
# LZMA / 7z. (internal only)
###

AC_ARG_WITH(7z, AC_HELP_STRING([--without-7z], [Build without internal 7z support [[default=no]]]), with_7z=$withval, with_7z=yes)
if test x$with_7z = xyes; then
	AC_DEFINE(GENS_LZMA, 1, LZMA/7z support via internal LZMA SDK)
fi # GENS_LZMA

AM_CONDITIONAL([GENS_LZMA], [test x$with_7z = xyes])


###
# gcc -fvisibility-inlines-hidden option
###

AC_MSG_CHECKING(for gcc -fvisibility-inlines-hidden support)

if test x$obj_format = xwin32; then
	# Symbol visibility isn't applicable on Win32.
	AC_MSG_RESULT(not applicable on Win32)
else
	AC_COMPILE_IFELSE(
		AC_LANG_PROGRAM(
			[[
				#include <stdio.h>
				#if defined(__GNUC__) && (__GNUC__ >= 4)
					// gcc-4.x supports visibility settings.
				#else
					// Not gcc-4.x or higher.
					#error Not gcc-4.x or higher.
				#endif
			]],
			[[printf("Test for gcc visibility support.\n");]]),
		[fvisibility=yes],
		[fvisibility=no]
	)
	
	AC_MSG_RESULT($fvisibility)
	
	if test x$fvisibility = xyes; then
		cxxflags_visibility="-fvisibility-inlines-hidden"
	else
		cxxflags_visibility=""
	fi
fi


###
# C++ map type
###

AC_MSG_CHECKING(for hash table type)

# These tests use C++.
AC_LANG(C++)

# Check if the TR1 unordered map works. (gcc-4.x and later)
AC_COMPILE_IFELSE(
	AC_LANG_PROGRAM(
		[[#include <tr1/unordered_map>]],
		[[std::tr1::unordered_map<int, int>::iterator testIterator;]]),
	[hashtable=tr1_unordered_map],
	[hashtable=no]
	)

if test x$hashtable = xno; then
	# Check if the gcc hash map works. (gcc-3.x and earlier)
	AC_COMPILE_IFELSE(
		AC_LANG_PROGRAM(
			[[#include <ext/hash_map>]],
			[[__gnu_cxx::hash_map<int, int>::iterator testIterator;]]),
		[hashtable=gnu_hash_map],
		[hashtable=no]
		)
	
	if test x$hashtable = xno; then
		# Check if the standard C++ map works.
		AC_COMPILE_IFELSE(
			AC_LANG_PROGRAM(
				[[#include <map>]],
				[[std::map<int, int>::iterator testIterator;]]),
			[hashtable=std_map],
			[hashtable=no]
			)
		
		if test x$hashtable = xno; then
			AC_MSG_ERROR(Could not find a usable hash table implementation.)
		fi
	fi
fi

if test x$hashtable = xtr1_unordered_map; then
	AC_MSG_RESULT(TR1 unordered_map)
	AC_DEFINE(GSFT_HASHTABLE_TR1_UNORDERED_MAP, 1, Use the TR1 unordered_map class.)
elif test x$hashtable = xgnu_hash_map; then
	AC_MSG_RESULT(GNU hash_map)
	AC_DEFINE(GSFT_HASHTABLE_GNU_HASH_MAP, 1, Use the GNU hash_map class.)
elif test x$hashtable = xstd_map; then
	AC_MSG_RESULT(Standard C++ map)
	AC_MSG_WARN(SLOW std::map is being used; consider upgrading your compiler.)
	AC_DEFINE(GSFT_HASHTABLE_STD_MAP, 1, Use the standard C++ std::map class. (SLOW))
fi

# Reset test language to C.
AC_LANG(C)


###
# Miscellaneous
###

# Windows-specific options.
if test x$os = xwin32; then
	# Check if the Windows console should be enabled.
	if test x$enable_win32_console = xyes; then
		# Enable the Windows console.
		ldflags_win32="$ldflags_win32 -Wl,-subsystem,console"
	else
		# Disable the Windows console.
		ldflags_win32="$ldflags_win32 -Wl,-subsystem,windows"
	fi
	
	# Windows NT target version and MSIE target version.
	# (Seriously, MSIE target version?)
	cflags_win32="${cflags_win32} -D_WIN32_WINNT=0x0400 -D_WIN32_IE=0x0300"
else
	# Not Win32.
	ldflags_win32=""
	cflags_win32=""
fi

# gcc CFLAGS check function.
# References:
# - http://www.nabble.com/Compiler-characteristics-td25316247.html
# - http://www.nongnu.org/autoconf-archive/ax_cflags_gcc_option.html
AC_DEFUN([GSFT_CFLAGS_GCC_OPTION], [
	AS_VAR_PUSHDEF([FLAGS], [CFLAGS])
	AS_VAR_PUSHDEF([VAR], [gsft_cv_cflags_gcc_option_$1])
	AC_CACHE_CHECK(
		[whether $CC accepts $1], VAR,
		[
			VAR="no, unknown"
			AC_LANG_SAVE
			AC_LANG_C
			ac_save_[]FLAGS="$[]FLAGS"
			FLAGS="$1"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])], VAR=yes, VAR=no)
			FLAGS="$ac_save_[]FLAGS"
			AC_LANG_RESTORE
		]
		);
	
	if test x$VAR = xyes; then
		$2="$$2 $1"
	fi
	AS_VAR_POPDEF([VAR])
	AS_VAR_POPDEF([FLAGS])
])

# g++ CXXFLAGS check function.
# References:
# - http://www.nabble.com/Compiler-characteristics-td25316247.html
# - http://www.nongnu.org/autoconf-archive/ax_cflags_gcc_option.html
AC_DEFUN([GSFT_CXXFLAGS_GCC_OPTION], [
	AS_VAR_PUSHDEF([FLAGS], [CXXFLAGS])
	AS_VAR_PUSHDEF([VAR], [gsft_cv_cxxflags_gcc_option_$1])
	AC_CACHE_CHECK(
		[whether $CXX accepts $1], VAR,
		[
			VAR="no, unknown"
			AC_LANG_SAVE
			AC_LANG_CPLUSPLUS
			ac_save_[]FLAGS="$[]FLAGS"
			FLAGS="$1"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])], VAR=yes, VAR=no)
			FLAGS="$ac_save_[]FLAGS"
			AC_LANG_RESTORE
		]
		)
	
	if test x$VAR = xyes; then
		$2="$$2 $1"
	fi
	AS_VAR_POPDEF([VAR])
	AS_VAR_POPDEF([FLAGS])
])

# gcc LDFLAGS check function.
# References:
# - http://www.nabble.com/Compiler-characteristics-td25316247.html
# - http://www.nongnu.org/autoconf-archive/ax_cflags_gcc_option.html
AC_DEFUN([GSFT_LDFLAGS_GCC_OPTION], [
	AS_VAR_PUSHDEF([FLAGS], [LDFLAGS])
	AS_VAR_PUSHDEF([VAR], [gsft_cv_ldflags_gcc_option_$1])
	AC_CACHE_CHECK(
		[whether $LD accepts $1], VAR,
		[
			VAR="no, unknown"
			AC_LANG_SAVE
			AC_LANG_C
			ac_save_LDFLAGS="$LDFLAGS"
			LDFLAGS="-Wl,$1"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])], VAR=yes, VAR=no)
			LDFLAGS="$ac_save_LDFLAGS"
			AC_LANG_RESTORE
		]
		);
	
	if test x$VAR = xyes; then
		$2="$$2 -Wl,$1"
	fi
	AS_VAR_POPDEF([VAR])
	AS_VAR_POPDEF([FLAGS])
])

# Check if the compiler supports various warning flags.
if test "x$GCC" = "xyes"; then
	GSFT_CFLAGS_GCC_OPTION([-Wall], AM_CFLAGS)
	GSFT_CXXFLAGS_GCC_OPTION([-Wall], AM_CXXFLAGS)
	
	GSFT_CFLAGS_GCC_OPTION([-Wextra], AM_CFLAGS)
	GSFT_CXXFLAGS_GCC_OPTION([-Wextra], AM_CXXFLAGS)
fi

# Check for various linker flags.
GSFT_LDFLAGS_GCC_OPTION([--as-needed], AM_LDFLAGS)
GSFT_LDFLAGS_GCC_OPTION([--hash-style=both], AM_LDFLAGS)

AM_CFLAGS="${AM_CFLAGS} ${cflags_optimization} ${cflags_debug} ${cflags_win32}"
AM_CXXFLAGS="${AM_CXXFLAGS} ${cflags_optimization} ${cflags_debug} ${cflags_win32} ${cxxflags_visibility}"
AM_LDFLAGS="${AM_LDFLAGS} ${ldflags_as_needed} ${ldflags_win32}"

# Check for clock_gettime.
AC_CHECK_LIB(rt, clock_gettime, has_librt=yes, has_librt=no)
AM_CONDITIONAL([HAVE_LIBRT], [test x$has_librt = xyes])
if test x$has_librt = xyes; then
	AC_DEFINE(HAVE_LIBRT, 1, Define to 1 if you have the `rt' library (-lrt).)
	RT_LIBS="-lrt"
else
	RT_LIBS=""
fi

# TODO: Check for getopt_long() and implement a fallback getopt() parser.
#AC_CHECK_LIB(c, getopt)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Check for various functions.
AC_CHECK_FUNCS(strdup, has_strdup=yes, has_strdup=no)
AM_CONDITIONAL([HAVE_STRDUP], [test x$has_strdup = xyes])

AC_CHECK_FUNCS(strsep, has_strsep=yes, has_strsep=no)
AM_CONDITIONAL([HAVE_STRSEP], [test x$has_strsep = xyes])

AC_CHECK_FUNCS(strlcpy, has_strlcat=yes, has_strlcpy=no)
AM_CONDITIONAL([HAVE_STRLCPY], [test x$has_strlcpy = xyes])

# TODO: Check for BSD semantics. (Solaris has a slightly different implementation.)
AC_CHECK_FUNCS(strlcat, has_strlcat=yes, has_strlcat=no)
AM_CONDITIONAL([HAVE_STRLCAT], [test x$has_strlcat = xyes])

AC_CHECK_FUNCS(sigaction, has_sigaction=yes, has_sigaction=no)
AM_CONDITIONAL([HAVE_SIGACTION], [test x$has_sigaction = xyes])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h wchar.h])

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([getcwd gettimeofday memmove memset mkdir pow rint])
AC_CHECK_FUNCS([setenv sqrt strcasecmp strchr strerror strncasecmp])
AC_CHECK_FUNCS([strpbrk strrchr strspn strstr strtol strtoul])

# Check for iconv().
# First, check for iconv() in the C library.
ICONV_LIBS=""
AC_CHECK_FUNCS(iconv, has_iconv=yes, has_iconv=no)
if test x$has_iconv = xno; then
	# iconv() was not found in the C library.
	# Check for libiconv.
	# TODO: Verify that the libiconv check works.
	AC_CHECK_LIB(iconv, iconv, has_iconv=yes, has_iconv=no)
	if test x$has_iconv = xyes; then
		# iconv() was found in libiconv.
		AC_DEFINE(HAVE_LIBICONV, 1, Define to 1 if you have the `iconv' library (-liconv).)
		AC_DEFINE(HAVE_ICONV, 1, Define to 1 if you have the `iconv' function.)
		ICONV_LIBS="-liconv"
	fi
fi
AM_CONDITIONAL([HAVE_ICONV], [test x$has_iconv = xyes])
AC_SUBST(ICONV_LIBS)

# Autoconf substitutions.

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GL_LIBS)
AC_SUBST(RT_LIBS)
AC_SUBST(LIBS)
AC_SUBST(GETTIMEFLAG)

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(libdir)

AC_CONFIG_FILES([
	Makefile
	doc/Makefile
	images/Makefile
	xdg/Makefile
	src/Makefile
	src/extlib/Makefile
	src/extlib/libpng/Makefile
	src/extlib/lzma/Makefile
	src/extlib/minizip/Makefile
	src/extlib/mp3_dec/Makefile
	src/extlib/zlib/Makefile
	src/starscream/Makefile
	src/mdZ80/Makefile
	src/libgsft/Makefile
	src/cbob/Makefile
	src/gens/Makefile
	src/gens/plugins/Makefile
	src/gens/plugins/render/Makefile
	src/gens/plugins/render/double/Makefile
	src/gens/plugins/render/normal/Makefile
	src/gens/fw/Makefile
	src/mdp/Makefile
	src/mdp/mdp_test/Makefile
	src/mdp/misc/Makefile
	src/mdp/misc/game_genie/Makefile
	src/mdp/misc/ips_patcher/Makefile
	src/mdp/misc/sgens/Makefile
	src/mdp/misc/vdp_layer_options/Makefile
	src/mdp/render/2xsai/Makefile
	src/mdp/render/Makefile
	src/mdp/render/blargg_ntsc/Makefile
	src/mdp/render/epx/Makefile
	src/mdp/render/epx_plus/Makefile
	src/mdp/render/hq2x/Makefile
	src/mdp/render/hq3x/Makefile
	src/mdp/render/hq4x/Makefile
	src/mdp/render/interpolated/Makefile
	src/mdp/render/interpolated_scanline/Makefile
	src/mdp/render/interpolated_scanline_25/Makefile
	src/mdp/render/interpolated_scanline_50/Makefile
	src/mdp/render/scale2x/Makefile
	src/mdp/render/scale3x/Makefile
	src/mdp/render/scale4x/Makefile
	src/mdp/render/scanline/Makefile
	src/mdp/render/scanline_25/Makefile
	src/mdp/render/scanline_50/Makefile
	src/mdp/render/super_2xsai/Makefile
	src/mdp/render/super_eagle/Makefile
	])
AC_OUTPUT
