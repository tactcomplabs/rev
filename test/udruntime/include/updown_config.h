/*
 * Copyright (c) 2021 University of Chicago
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author - Andronicus
 * Initial UpStream Program to do a basic operation on data
 * Add and return value
 *
 */
#ifndef UPSTREAM_H
#define UPSTREAM_H

#include <stdint.h>

// Node configuration is controlled in cmake
// Default is 1 node
#ifdef NODE64
#define DEF_NUM_NODES 64  // 64 Node system
#elif NODE32
#define DEF_NUM_NODES 32  // 32 Node system
#elif NODE8
#define DEF_NUM_NODES 8  // 8 node system
#else
#define DEF_NUM_NODES 1  // 1 node system
#endif

#define DEF_NUM_LANES       64     // Number of lanes per CU
#define DEF_NUM_UDS         4      // Number of CUs
#define DEF_NUM_STACKS      8      // Number of Stacks per Node
#define DEF_SPMEM_BANK_SIZE 65536  // Scratchpad Memory size per lane
#define DEF_WORD_SIZE       8      // Wordsize
#define DEF_MAPPED_SIZE     1UL << 32
//#define DEF_GMAPPED_SIZE 1UL << 32
#define DEF_GMAPPED_SIZE    1UL << 36


// This address space can support 64 nodes
#ifndef GEM5_MODE
// Base address for scratchpad memories
// In latest: #define BASE_SPMEM_ADDR 0x7FC0'0000'0000llu // what is this?
#define BASE_SPMEM_ADDR 0x400000000
// Base address for memory mapped control registers
#define BASE_CTRL_ADDR  0x600000000
// Base address for UpDown Program
#define BASE_PROG_ADDR  0x800000000
#else
// Base address for scratchpad memories
#define BASE_SPMEM_ADDR 0x20000000000
// Base address for memory mapped control registers
#define BASE_CTRL_ADDR  0x26000000000
// Base address for UpDown Program
#define BASE_PROG_ADDR  0x28000000000
#endif

// Control signals address space capacity. Number of control registers may
#define CONTROL_CAPACITY_PER_LANE 32

// Scratchpad address space maximum capacity = def currently
#define SPMEM_CAPACITY_PER_LANE   65536

// Number of lanes capacity - Max = Def currently
#define NUM_LANES_CAPACITY        64
// #define NUM_LANES_CAPACITY DEF_NUM_LANES

// Number of UDs capacity - UD capacity
// #define NUM_UDS_CAPACITY DEF_NUM_UDS
#define NUM_UDS_CAPACITY          4

// Number of nodes capacity
// #define NUM_NODES_CAPACITY DEF_NUM_NODES
#define NUM_NODES_CAPACITY        16

// Number of stacks capacity
// #define NUM_STACKS_CAPACITY DEF_NUM_STACKS
#define NUM_STACKS_CAPACITY       8

// Base address mapped memory - This is due to simulation
#define BASE_SYNC_SPACE           0x7FFF0000

// Revisit this
// #ifndef GEM5_MODE
// #define BASE_MAPPED_ADDR 0x7FD0'0000'0000llu
// #define BASE_MAPPED_GLOBAL_ADDR 0x2'0000'0000llu
// #else
// #define BASE_MAPPED_ADDR 0x8000'0000llu
// #define BASE_MAPPED_GLOBAL_ADDR 0x2'0000'0000llu
// #endif
#define BASE_MAPPED_ADDR          0x80000000
#define BASE_MAPPED_GLOBAL_ADDR   0x200000000

// CONTROL SIGNALES OFFSET IN WORDS
#define EVENT_QUEUE_OFFSET        0x0
#define OPERAND_QUEUE_OFFSET      0x1
#define START_EXEC_OFFSET         0x2
#define LOCK_OFFSET               0x3

namespace UpDown {

// Depending of the word size we change the pointer type
#if DEF_WORD_SIZE == 4
typedef uint32_t word_t;
#elif DEF_WORD_SIZE == 8
typedef uint64_t word_t;
#else
#error Unknown default word size
#endif

typedef word_t*          ptr_t;
static constexpr uint8_t CREATE_THREAD = 0xFF;

}  // namespace UpDown

#endif
