#!/bin/sh
set -eu
cd "$(dirname "$0")"
binary=$(mktemp "${TMPDIR:-/tmp}/virtualcam-lifecycle.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
${CC:-cc} -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter \
    -Wno-unused-variable ${CFLAGS:-} -I. lifecycle.c -o "$binary"
"$binary"
