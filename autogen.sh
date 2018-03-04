#!/bin/sh
AMVERSION=1.xxx
AMVERSION=1.7
AMVERSION=local
AMVERSION=1.9
AMVERSION=1.11

AMVERSION=any
ACLOCALVERSION=any
ACLOCAL_FLAGS=--force

BINDIR=/usr/local/bin
BINDIR=/usr/bin
set -x


echo "running libtoolize"

libtoolize --force --copy 

echo "running aclocal"
if (aclocal-$ACLOCALVERSION --version)  < /dev/null > /dev/null 2>&1; then
    aclocal-$ACLOCALVERSION ${ACLOCAL_FLAGS}
else
    $BINDIR/aclocal ${ACLOCAL_FLAGS}
fi

echo "Running autoheader -f"
autoheader -f 

echo "running automake"
if (automake-$AMVERSION --version) < /dev/null > /dev/null 2>&1; then
    automake-$AMVERSION -a
else
    $BINDIR/automake -a
fi

echo "running autoconf"
$BINDIR/autoconf
