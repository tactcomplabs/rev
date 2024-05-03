#!/bin/bash
# -- cleans the unnecessary directory reference manpages _*.3

BASE=$1

rm -Rf $BASE/_*.3
