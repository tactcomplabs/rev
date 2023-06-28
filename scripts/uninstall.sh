#!/bin/bash
#
# Unregisters Rev from the SST infrastructure
#

#-- unregister it
sst-register -u revcpu

#-- forcible remove it from the local script
CONFIG=~/.sst/sstsimulator.conf
if test -f "$CONFIG"; then
  echo "Removing configuration from local config=$CONFIG"
  sed -i.bak '/revcpu/d' $CONFIG
fi

