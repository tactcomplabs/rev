#!/bin/bash
#
# scripts/slurm/exec_build.sh
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# This file is a part of the PAN-RUNTIME package.  For license
# information, see the LICENSE file in the top level directory of
# this distribution.
#

#-- execute the job
SCRIPT=$1
SLURM_ID=`sbatch -N1 --export=ALL ./scripts/slurm/$SCRIPT | awk '{print $4}'`

#-- wait for completion
COMPLETE=`squeue -u builduser | grep ${SLURM_ID}`
while [ -n "$COMPLETE" ]; do
  COMPLETE=`squeue -u builduser | grep ${SLURM_ID}`
done

#-- echo the result to the log
cat rev.jenkins.${SLURM_ID}.out

#-- job has completed, test for status
STATE=`cat slurm-${SLURM_ID}.out | grep "FAILED"`

if [ -z "$STATE" ]
then
  echo "TEST PASSED FOR JOB_ID = ${JOB_ID}; SLURM_JOB=${SLURM_ID}"
  exit 0
else
  echo "TEST FAILED FOR JOB_ID = ${JOB_ID}; SLURM_JOB=${SLURM_ID}"
  exit -1
fi;

exit 0
#-- EOF
