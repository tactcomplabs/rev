//
// _libRevComponent_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "SST.h"
#include "../include/RevCPU.h"

namespace SST::RevCPU{

char pyrevcpu[] = {
#include "../include/pyrevcpu.inc"
  0x00};

class RevCPUPyModule : public SSTElementPythonModule {
public:

  /// Constructor
  explicit RevCPUPyModule(std::string library) :
    SSTElementPythonModule(std::move(library)) {
    createPrimaryModule(pyrevcpu, "pyrevcpu.py");
  }

  // Register the library with ELI
  SST_ELI_REGISTER_PYTHON_MODULE(
    SST::RevCPU::RevCPUPyModule,      // python class
    "revcpu",                         // component library
    SST_ELI_ELEMENT_VERSION(1, 0, 0)
    )

  // Export the library via ELI
  SST_ELI_EXPORT(SST::RevCPU::RevCPUPyModule)

}; // RevCPUPyModule

} // namespace SST::RevCPU

// EOF
