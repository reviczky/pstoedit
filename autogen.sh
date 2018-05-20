#!/bin/sh
set -e
LANG=C
export LANG
autoreconf --force --install --warn all $*
