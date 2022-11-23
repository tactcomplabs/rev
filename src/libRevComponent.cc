//
// _libRevComponent_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <sst/core/sst_config.h>
#include "RevCPU.h"

// Install the python library
#include <sst/core/model/element_python.h>

namespace SST {
  namespace RevCPU {

    char pyrevcpu[] = {
      #include "pyrevcpu.inc"
      0x00};

    class RevCPUPyModule : public SSTElementPythonModule {
    public:

      /// Constructor
      RevCPUPyModule(std::string library) :
        SSTElementPythonModule(library) {
        auto primary_module = createPrimaryModule(pyrevcpu,
                                                  "pyrevcpu.py");
      }

      // Register the library with ELI
      SST_ELI_REGISTER_PYTHON_MODULE(
        SST::RevCPU::RevCPUPyModule,      // python class
        "revcpu",                         // component library
        SST_ELI_ELEMENT_VERSION(1,0,0)
      )

      // Export the library via ELI
      SST_ELI_EXPORT(SST::RevCPU::RevCPUPyModule)

    }; // RevCPUPyModule
  } // namespace RevCPU
} // namespace SST

// EOF
