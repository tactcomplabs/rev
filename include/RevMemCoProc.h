//
// _RevMemCoProc_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEMCOPROC_H_
#define _SST_REVCPU_REVMEMCOPROC_H_

#include <string>

#include "RevCoProc.h"  // includes SST.h, which has StandardMem

namespace SST::RevCPU {

/// A co-processor with a memory port of its own.
///
/// The port is a StandardMem interface, normally connected in the Python configuration to the same
/// cache (or bus) as the core's, so the co-processor sees the program's addresses and what the core
/// has written. This class does what such a co-processor always has to:
///
///  - load the memory interface from the subcomponent slot "memory" (the slot name is a parameter
///    of the constructor; a derived class documents the slot in its own SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS)
///    and route its responses to HandleMemory();
///  - register a clock of its own, at a frequency of its own, which calls MemClock(), and give the
///    memory interface that clock as its time base;
///  - forward init(), setup(), complete() and finish() to the memory interface. SST calls these
///    only on top-level components; RevCPU forwards them to its co-processor (see RevCPU::init),
///    and this class passes them on. A derived class that overrides them calls this class's first.
///
/// What the co-processor does with the port, and how it takes instructions (IssueInst, or the
/// Xcoproc extension's Command), is up to the derived class.
class RevMemCoProc : public RevCoProc {
public:
  using StdMem = SST::Interfaces::StandardMem;

  /// RevMemCoProc: clock is the frequency of the co-processor's own clock (for example "1GHz")
  RevMemCoProc(
    ComponentId_t id, Params& params, RevCore* parent, const std::string& clock, const std::string& memorySlot = "memory"
  );

  ~RevMemCoProc() override = default;

  void init( unsigned int phase ) override { memory->init( phase ); }
  void setup() override { memory->setup(); }
  void complete( unsigned int phase ) override { memory->complete( phase ); }
  void finish() override { memory->finish(); }

protected:
  /// RevMemCoProc: a response from the memory interface
  virtual void HandleMemory( StdMem::Request* req ) = 0;

  /// RevMemCoProc: the co-processor's own clock. Return true to unregister it, false to go on.
  /// (RevCoProc::ClockTick is a different thing: Rev calls it once per core cycle.)
  virtual bool MemClock( SST::Cycle_t cycle ) { return false; }

  StdMem* memory{};  ///< RevMemCoProc: the co-processor's memory interface

private:
  bool ClockHandler( SST::Cycle_t cycle ) { return MemClock( cycle ); }
  void MemResponse( StdMem::Request* req ) { HandleMemory( req ); }
};  // class RevMemCoProc

}  // namespace SST::RevCPU

#endif
