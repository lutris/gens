#!/bin/sh
#
# libtool assumes that the compiler can handle the -fPIC flag
# This isn't always true (for example, nasm can't handle it)
pic="no"
command=""
while [ $# -gt 0 ]; do
    case "$1" in
        -[fK][pP][iI][cC])
            # PIC - define __PIC in nasm.
            if test "x$pic" = "xno"; then
                command="$command -D __PIC"
                pic="yes"
            fi
            ;;
	-[dD][pP][iI][cC])
            # DPIC - ignore.
            # This variant is used by libtool on mingw.
            ;;
        -I)
            command="$command $1 $2/"
            shift
            ;;
	-fno-common)
	    # MacOS X's compiler is stupid.
	    ;;
        *)
            command="$command $1"
            ;;
    esac
    shift
done
echo $command
exec $command

