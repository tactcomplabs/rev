//
// _RevCoProcCmd_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVCOPROCCMD_H_
#define _SST_REVCPU_REVCOPROCCMD_H_

#include <cstdint>

namespace SST::RevCPU {

class RevMem;

/// A command for the attached co-processor, from an instruction of the Xcoproc extension: R-type on
/// one of the custom opcodes, funct3 = { xd, xs1, xs2 }, funct7 the command (the encoding of RoCC).
struct RevCoProcCmd {
  unsigned custom = 0;      ///< which custom opcode: 0..3 for custom-0..custom-3
  unsigned funct7 = 0;      ///< the command; its meaning belongs to the co-processor
  bool     xd     = false;  ///< the co-processor answers: the core writes the result to rd
  bool     xs1    = false;  ///< rs1 is used
  bool     xs2    = false;  ///< rs2 is used
  uint64_t rs1    = 0;      ///< value of rs1 (0 if not used)
  uint64_t rs2    = 0;      ///< value of rs2 (0 if not used)
  unsigned hart   = 0;      ///< the hart that executed the instruction
  RevMem*  mem    = nullptr;
};

}  // namespace SST::RevCPU

#endif
