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
USER=$(id -un)

#-- execute the job
SCRIPT=$1
SLURM_ID=`sbatch -N1 --export=ALL $SCRIPT | awk '{print $4}'`

#-- wait for completion
COMPLETE=`squeue -u $USER | grep ${SLURM_ID}`
while [ -n "$COMPLETE" ]; do
  sleep 1
  COMPLETE=`squeue -u $USER | grep ${SLURM_ID}`
done

#-- echo the result to the log
cat rev.jenkins.${SLURM_ID}.out

#-- job has completed, test for status
STATE=`cat rev.jenkins.${SLURM_ID}.out | grep "tests failed out of"`
NUM_FAILED=`cat rev.jenkins.${SLURM_ID}.out | grep "tests failed out of" | awk '{print $4}'`


if [ "$NUM_FAILED" -eq "0" ];
then
  echo "TEST PASSED FOR JOB_ID = ${JOB_ID}; SLURM_JOB=${SLURM_ID}"
  echo $STATE
  exit 0
else
  echo "TEST FAILED FOR JOB_ID = ${JOB_ID}; SLURM_JOB=${SLURM_ID}"
  echo $STATE
  exit -1
fi

exit 0
#-- EOF
