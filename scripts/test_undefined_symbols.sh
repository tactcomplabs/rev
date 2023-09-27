#!/bin/sh

exec >&2

if ! [ -f "$1" ]; then
  cat<<EOF
Usage: $0 <sharedlib>

Tests for unresolved Rev symbols in shared library
EOF
  exit 1
fi

if [ -z ${NO_COLOR+x} ] && tty -s; then
  RED="\033[41;97m\n"
  END="\033[0m"
else
  RED=
  END=
fi

printf "${RED}"
if ldd -r "$1" 2>&1 | c++filt | grep "\bundefined\b.*\bSST::RevCPU::"; then
  echo ""
  rc=1
else
  rc=0
fi
printf "${END}"

exit $rc
