#!/bin/sh

set_git_hooks()
{
    # assume that the current directory is the source tree
    if [ -d ".git" ] ; then
        for hook in $(ls -1 .git-hooks) ; do
            cd .git/hooks
            if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
                rm -f "${hook?}"
                ln -sf "../../.git-hooks/${hook?}" "${hook?}"
            fi
            cd - > /dev/null
        done
    fi
}

TESTLIBTOOLIZE="glibtoolize libtoolize"

LIBTOOLIZEFOUND="0"

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

set_git_hooks

aclocal --version > /dev/null 2> /dev/null || {
    echo "error: aclocal not found"
    exit 1
}
automake --version > /dev/null 2> /dev/null || {
    echo "error: automake not found"
    exit 1
}

for i in $TESTLIBTOOLIZE; do
    if which $i > /dev/null 2>&1; then
        LIBTOOLIZE=$i
        LIBTOOLIZEFOUND="1"
        break
    fi
done

if [ "$LIBTOOLIZEFOUND" = "0" ]; then
    echo "$0: need libtoolize tool to build libvisio" >&2
    exit 1
fi

amcheck=`automake --version | grep 'automake (GNU automake) 1.5'`
if test "x$amcheck" = "xautomake (GNU automake) 1.5"; then
    echo "warning: you appear to be using automake 1.5"
    echo "         this version has a bug - GNUmakefile.am dependencies are not generated"
fi

rm -rf autom4te*.cache

$LIBTOOLIZE --force --copy || {
    echo "error: libtoolize failed"
    exit 1
}
aclocal $ACLOCAL_FLAGS || {
    echo "error: aclocal $ACLOCAL_FLAGS failed"
    exit 1
}
autoheader || {
    echo "error: autoheader failed"
    exit 1
}
automake -a -c --foreign || {
    echo "warning: automake failed"
}
autoconf || {
    echo "error: autoconf failed"
    exit 1
}
