//
// _RevExt_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVEXT_H_
#define _SST_REVCPU_REVEXT_H_

// -- SST Headers
#include "SST.h"

// -- Standard Headers
#include <string>
#include <cmath>
#include <utility>
#include <vector>

// -- RevCPU Headers
#include "RevInstTable.h"
#include "RevMem.h"
#include "RevFeature.h"

namespace SST::RevCPU{

struct RevExt{
  /// RevExt: standard constructor
  RevExt( std::string Name, RevFeature *Feature,
          RevRegFile *RegFile, RevMem *RevMem,
          SST::Output *Output )
    : feature(Feature), mem(RevMem), name(Name), output(Output) {
        regFile = RegFile;
      }

  /// RevExt: standard destructor. virtual so that Extensions[i] can be deleted
  virtual ~RevExt() = default;

  /// RevExt: sets the internal instruction table
  void SetTable(std::vector<RevInstEntry> InstVect){
    table = std::move(InstVect);
  }

  /// RevExt: sets the internal compressed instruction table
  void SetCTable(std::vector<RevInstEntry> InstVect){
    ctable = std::move(InstVect);
  }

  /// RevExt: sets the optional table (used for variant-specific compressed encodings)
  void SetOTable(std::vector<RevInstEntry> InstVect){
    otable = std::move(InstVect);
  }

  /// RevExt: retrieve the extension name
  const std::string& GetName() const { return name; }

  /// RevExt: baseline execution function
  bool Execute(unsigned Inst, RevInst Payload, uint16_t threadID);

  /// RevExt: retrieves the extension's instruction table
  const std::vector<RevInstEntry>& GetInstTable() { return table; }

  /// RevExt: retrieves the extension's compressed instruction table
  const std::vector<RevInstEntry>& GetCInstTable() { return ctable; }

  /// RevExt: retrieves the extension's optional instruction table
  const std::vector<RevInstEntry>& GetOInstTable() { return otable; }

  /// RevExt: updates the RegFile pointer prior to instruction execution
  ///         such that the currently executing RevThreadCtx is the one
  ///         whose register file is operated on
  void SetRegFile(RevRegFile* RegFile) { regFile = RegFile; }

protected:
  RevFeature *feature;  ///< RevExt: feature object
  RevRegFile* regFile;  ///< RevExt: register file object
  RevMem *mem;          ///< RevExt: memory object

private:
  std::string name;                 ///< RevExt: extension name
  SST::Output *output;              ///< RevExt: output handler
  std::vector<RevInstEntry> table;  ///< RevExt: instruction table
  std::vector<RevInstEntry> ctable; ///< RevExt: compressed instruction table
  std::vector<RevInstEntry> otable; ///< RevExt: optional compressed instruction table

}; // class RevExt
} // namespace SST::RevCPU

#endif
