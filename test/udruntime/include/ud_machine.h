#pragma once
#ifndef __UD_MACHINE_H__
#define __UD_MACHINE_H__
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <utility>

#include "debug.h"

namespace UpDown {
/**
 * @brief Structure containing the machine configuration.
 *
 * This structure can be used to change the parameters of the runtime, during
 * runtime construction.
 */

struct ud_machine_t {
  // Offsets for addrs space
  uint64_t MapMemBase = BASE_MAPPED_ADDR;  // Base address for memory map
  uint64_t GMapMemBase =
    BASE_MAPPED_GLOBAL_ADDR;                    // Base address for memory map
  uint64_t UDbase           = BASE_SPMEM_ADDR;  // Base address for upstream
  uint64_t SPMemBase        = BASE_SPMEM_ADDR;  // ScratchPad Base address
  uint64_t ControlBase      = BASE_CTRL_ADDR;   // Base for control operands
  uint64_t ProgBase         = BASE_PROG_ADDR;   // Base for control operands
  uint64_t EventQueueOffset = EVENT_QUEUE_OFFSET;  // Offset for Event Queues
  uint64_t OperandQueueOffset =
    OPERAND_QUEUE_OFFSET;                        // Offset for Operands Queues
  uint64_t StartExecOffset = START_EXEC_OFFSET;  // Offset for Start Exec signal
  uint64_t LockOffset      = LOCK_OFFSET;        // Offset for Lock signal

  // Machine config and capacities
  uint64_t CapNumNodes     = NUM_NODES_CAPACITY;       // Max number of UpDowns
  uint64_t CapNumStacks    = NUM_STACKS_CAPACITY;      // Max number of UpDowns
  uint64_t CapNumUDs       = NUM_UDS_CAPACITY;         // Max number of UpDowns
  uint64_t CapNumLanes     = NUM_LANES_CAPACITY;       // Max number of UpDowns
  uint64_t CapSPmemPerLane = SPMEM_CAPACITY_PER_LANE;  // Max bank size per lane
  uint64_t CapControlPerLane =
    CONTROL_CAPACITY_PER_LANE;  // Max Control Sigs and regs per lane
  uint64_t NumUDs           = DEF_NUM_UDS;     // Number of UpDowns
  uint64_t NumLanes         = DEF_NUM_LANES;   // Number of lanes
  uint32_t LocalMemAddrMode = 1;               // Local Memory Mode
  uint64_t NumStacks        = DEF_NUM_STACKS;  // Number of lanes
  uint64_t NumNodes         = DEF_NUM_NODES;   // Number of lanes

  // Sizes for memories
  uint64_t MapMemSize       = DEF_MAPPED_SIZE;   // Mapped Memory size
  uint64_t GMapMemSize      = DEF_GMAPPED_SIZE;  // Mapped Memory size
  uint64_t SPBankSize =
    DEF_SPMEM_BANK_SIZE;  // Local Momory (scratchpad) Bank Size
  uint64_t SPBankSizeWords =
    DEF_SPMEM_BANK_SIZE * DEF_WORD_SIZE;  // LocalMemorySize in Words

  // Scratchpad Memory Size
  uint64_t SPSize() {
    return this->CapNumNodes * this->CapNumStacks * this->CapNumUDs *
           this->CapNumLanes * this->CapSPmemPerLane;
  }
};
}  // namespace UpDown

#endif
