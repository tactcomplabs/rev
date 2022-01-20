//
// _RevExt_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVEXT_H_
#define _SST_REVCPU_REVEXT_H_

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>

// -- Standard Headers
#include <string>
#include <cmath>

// -- RevCPU Headers
#include "RevInstTable.h"
#include "RevMem.h"
#include "RevFeature.h"

namespace SST::RevCPU{
  class RevExt;
}

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    class RevExt{
    public:
      /// RevExt: standard constructor
      RevExt( std::string Name, RevFeature *Feature,
              RevRegFile *RegFile, RevMem *RevMem,
              SST::Output *Output );

      /// RevExt: standard destructor
      ~RevExt();

      /// RevExt: sets the internal instruction table
      void SetTable(std::vector<RevInstEntry> InstVect);

      /// RevExt: retrieve the extension name
      std::string GetName() { return name; }

      /// RevExt: baseline execution function
      bool Execute(unsigned Inst, RevInst Payload, uint8_t threadID);

      /// RevExt: retrieves the extension's instruction table
      std::vector<RevInstEntry> GetInstTable() { return table; }

    protected:
      RevFeature *feature;  ///< RevExt: feature object
      RevRegFile* regFile;  ///< RevExt: register file object
      RevMem *mem;          ///< RevExt: memory object

    private:
      std::string name;                 ///< RevExt: extension name
      SST::Output *output;              ///< RevExt: output handler
      std::vector<RevInstEntry> table;  ///< RevExt: instruction table

    }; // class RevExt
  } // namespace RevCPU
} // namespace SST

#endif
