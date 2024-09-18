#!/usr/bin/python3
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-print.py
#
# Intent: Extract and pretty-print rev-fast-printf strings from rev logs
#
# TODO: Consider separate file stream for rev-fast-printf to avoid this
# kind of (slow and inefficient) postprocessing

import argparse
import re

parser = argparse.ArgumentParser(
    prog="rev-print.py",
    description="Extract and render rev-fast-print strings from log file")
parser.add_argument('-l', '--logFile', dest='logFile', required=True,
                    help="path to REV output log file")
parser.add_argument('-ts', '--timeStamp', dest="timeStamp", required=False, default="False",
                    help="show timestamp information for rev-fast-print string")
args = parser.parse_args()

try:
    fn = open(args.logFile)
except Exception:
    print("Cannot open file " + args.logFile)
    exit(1)

inString = False
reQuick = re.compile("rev-pr")
reTimeStamp = re.compile(r":(\d+)\]: <rev-print>")
reStart = re.compile("<rev-print>(.*)")
reEnd = re.compile("(.*)</rev-print>")
reBoth = re.compile("<rev-print>(.*)</rev-print>")
for line in fn:
    # check for early bail out (most of the time)
    q = reQuick.search(line)
    if not q and not inString:
        continue
    b = reBoth.search(line)
    if b:
        inString = False
        print(b.group(1), end="")
        continue
    e = reEnd.search(line)
    if e:
        inString = False
        print(e.group(1), end="")
        continue
    s = reStart.search(line)
    if s:
        inString = True
        if args.timeStamp is True:
            ts = reTimeStamp.search(line)
            if ts:
                print(f"#{ts.group(1)}")
        print(s.group(1))
        continue
    if inString:
        print(line)
