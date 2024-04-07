#!/bin/bash

rm -f O0.log O2.log O2_WORKAROUND.log

echo -n "Testing -O0 ... "
make clean -s && make CCOPT=-O0 -j >& O0.log && echo "Passed" || echo "Failed"
echo "See O0.log"

echo "Testing -O2 with O2_WORKAROUND"
make clean -s  && make -j -s CCOPT=-O2 DEBUG_FLAGS=-DO2_WORKAROUND >& O2_WORKAROUND.log && echo "Passed" || echo "Failed"
echo "See O2_WORKAROUND.log"

echo "Testing -O2 without O2_WORKAROUND"
make clean -s  && make -j -s CCOPT=-O2 >& O2.log && echo "Passed" || echo "Failed"
echo "See O2.log"

echo "Done with testing"

