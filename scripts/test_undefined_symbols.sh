#!/bin/sh

if ! [ -f "$1" ]; then
   cat>&2 <<EOF
usage: $0 librevcpu.so

Tests for unresolved Rev symbols in shared library
EOF
   exit 1
fi

if ldd -r "$1" 2>&1 | c++filt | grep "\bundefined\b.*\bSST::RevCPU::" >&2; then
    exit 1
else
    exit 0
fi
