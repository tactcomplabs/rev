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
#include "RevFenv.h"

namespace SST::RevCPU{

struct RevExt{
  /// RevExt: standard constructor
  RevExt( std::string_view name,
          RevFeature* feature,
          RevMem *mem,
          SST::Output *output )
    : name(name), feature(feature), mem(mem), output(output){
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
  std::string_view GetName() const { return name; }

  /// RevExt: baseline execution function
  bool Execute(unsigned Inst, const RevInst& Payload, uint16_t HartID, RevRegFile* regFile);

  /// RevExt: retrieves the extension's instruction table
  const std::vector<RevInstEntry>& GetInstTable(){ return table; }

  /// RevExt: retrieves the extension's compressed instruction table
  const std::vector<RevInstEntry>& GetCInstTable(){ return ctable; }

  /// RevExt: retrieves the extension's optional instruction table
  const std::vector<RevInstEntry>& GetOInstTable(){ return otable; }

private:
  std::string_view const name;      ///< RevExt: extension name
  RevFeature *const feature;        ///< RevExt: feature object
  RevMem *const mem;                ///< RevExt: memory object
  SST::Output *const output;        ///< RevExt: output handler

  std::vector<RevInstEntry>  table; ///< RevExt: instruction table
  std::vector<RevInstEntry> ctable; ///< RevExt: compressed instruction table
  std::vector<RevInstEntry> otable; ///< RevExt: optional compressed instruction table

  auto SetFPEnv(unsigned Inst, const RevInst& Payload, uint16_t threadID, RevRegFile* regFile);
}; // class RevExt
} // namespace SST::RevCPU

#endif
