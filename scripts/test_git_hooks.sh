#!/bin/sh
#shellcheck enable=all
#shellcheck disable=2059

exec >&2

if [ -z ${NO_COLOR+x} ] && tty -s <&2; then
  RED="\033[91m"
  END="\033[0m"
else
  RED=
  END=
fi

case $(id -nu) in
    builduser|devuser) exit 0 ;;
    *)
esac

hooks=$(git config core.hooksPath)
if [ "${hooks}" != .githooks ]; then
    printf "${RED}\n"
    cat <<EOF
Git is not configured to use Rev Git hooks, which enforce coding guidelines.

To install the Rev Git hooks, run:

git config core.hooksPath .githooks
EOF
    printf "${END}\n"
    exit 1
fi
