//
// _RevExt_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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
#include <cmath>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// -- RevCPU Headers
#include "RevFeature.h"
#include "RevFenv.h"
#include "RevInstTable.h"
#include "RevMem.h"

namespace SST::RevCPU {

struct RevExt {
  /// RevExt: standard constructor
  RevExt( std::string_view name, const RevFeature* feature, RevMem* mem, SST::Output* output )
    : name( name ), feature( feature ), mem( mem ), output( output ) {}

  /// RevExt: standard destructor. virtual so that Extensions[i] can be deleted
  virtual ~RevExt()                  = default;

  // We do not allow copying, moving or assigning
  RevExt( const RevExt& )            = delete;
  RevExt( RevExt&& )                 = delete;
  RevExt& operator=( const RevExt& ) = delete;
  RevExt& operator=( RevExt&& )      = delete;

  /// RevExt: sets the internal instruction table
  // Note: && means the argument should be an rvalue or std::move(lvalue)
  // This avoids deep std::vector copies and uses only one std::vector move.
  void SetTable( std::vector<RevInstEntry>&& InstVect ) { table = std::move( InstVect ); }

  /// RevExt: sets the internal compressed instruction table
  void SetCTable( std::vector<RevInstEntry>&& InstVect ) { ctable = std::move( InstVect ); }

  /// RevExt: retrieve the extension name
  std::string_view GetName() const { return name; }

  /// RevExt: baseline execution function
  bool Execute( unsigned Inst, const RevInst& Payload, uint16_t HartID, RevRegFile* regFile ) const;

  /// RevExt: retrieves the extension's instruction table
  const std::vector<RevInstEntry>& GetTable() const { return table; }

  /// RevExt: retrieves the extension's compressed instruction table
  const std::vector<RevInstEntry>& GetCTable() const { return ctable; }

private:
  std::string_view const    name;      ///< RevExt: extension name
  const RevFeature* const   feature;   ///< RevExt: feature object
  RevMem* const             mem;       ///< RevExt: memory object
  SST::Output* const        output;    ///< RevExt: output handler
  std::vector<RevInstEntry> table{};   ///< RevExt: instruction table
  std::vector<RevInstEntry> ctable{};  ///< RevExt: compressed instruction table

};  // class RevExt

}  // namespace SST::RevCPU

#endif
