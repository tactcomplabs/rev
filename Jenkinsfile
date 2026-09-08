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

#-- setup the SST 16.0.0 environment
#   NOTE: confirm the SST 16.0.0 install prefix on the CI node.
export PATH=$PATH:/rev/sst/sst-16.0.0/bin:/rev/riscv/bin
export RISCV=/rev/riscv

#-- build, test & install against SST 16.0.0 (CMake build)
rm -rf build
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
cmake --build build --target install'''
      }
    }

  }
}
