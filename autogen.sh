#!/bin/sh
AMVERSION=1.xxx
AMVERSION=1.7
AMVERSION=local

BINDIR=/usr/bin
BINDIR=/usr/local/bin
set -x
echo running aclocal
if (aclocal-$AMVERSION --version)  < /dev/null > /dev/null 2>&1; then
    aclocal-$AMVERSION ${ACLOCAL_FLAGS}
else
    $BINDIR/aclocal ${ACLOCAL_FLAGS}
fi

echo running automake
if (automake-$AMVERSION --version) < /dev/null > /dev/null 2>&1; then
    automake-$AMVERSION -a
else
    $BINDIR/automake -a
fi

echo running autoconf
$BINDIR/autoconf
# CONFIG_DIR=config
# echo aclocal -I /usr/local/share/aclocal/ --output=${CONFIG_DIR}/aclocal.m4
# aclocal -I /usr/local/share/aclocal --output=${CONFIG_DIR}/aclocal.m4

# echo automake -a
# automake -a --include-deps

# echo autoconf  --localdir=${CONFIG_DIR}
# autoconf  --localdir=${CONFIG_DIR}

