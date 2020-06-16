pipeline {
  agent {
    node {
      label 'bo_sst'
    }

  }
  stages {
    stage('Build') {
      steps {
        sh '''#!/bin/bash -xe

#-- setup the workspace
cd $WORKSPACE

#-- setup the SST 9.1.0 environment
export OLD_PATH=$PATH
export PATH=$PATH:/rev/sst/sst-9.1.0/bin:/rev/riscv/bin

#-- build & install the SST 9.1.0 version
make
make doc
make install
make clean

# --------------- DONE WITH SST 9.1.0
# --------------- BEGIN SST 10.0.0
export PATH=$OLD_PATH
cd $WORKSPACE
export PATH=$PATH:/rev/sst/sst-10.0.0/bin:/rev/riscv/bin

#-- build & install the SST 10.0.0 version
make
make doc
make install'''
      }
    }

  }
}