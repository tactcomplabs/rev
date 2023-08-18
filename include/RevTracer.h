//
// _RevTracer_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTRACER_H_
#define _SST_REVCPU_REVTRACER_H_

// -- Standard Headers
#include <cstdint>
#include <ostream>
#include <string>
#include <iostream>
#include <map>
#include <vector>

// -- Rev Headers

// -- Other headers to be subsumed (TODO)
#include "rvdiasm/disasm.h"

namespace SST{
  namespace RevCPU{

  const unsigned MAGIC_INST = 0x4033; // xor zero,zero,zero

  enum class EVENT_SYMBOL : unsigned {
    OK = 0x0,
    STALL = 0x1,
    FLUSH = 0x2,
    TRACE_ON = 0x100,
    TRACE_OFF = 0x102,
  };

  const std::map<EVENT_SYMBOL,char> event2char {
    {EVENT_SYMBOL::OK,' '},
    {EVENT_SYMBOL::STALL,'#'},
    {EVENT_SYMBOL::FLUSH,'!'},
    {EVENT_SYMBOL::TRACE_ON,'+'},
    {EVENT_SYMBOL::TRACE_OFF,'-'}
  };
    
  union TraceEvents_t {
    uint64_t v = 0;
    struct {
      // define 1 bit per stall type
      uint64_t stall : 1;           // [0]
      uint64_t stall_sources : 15;  // [15:1]
      uint64_t flush : 1;           // [16]
      uint64_t flush_sources : 15;  // [31:17]
      uint64_t spare : 31;          // [62:32]
      uint64_t trc_ctl : 1;         // [63] indicate change in trace controls
    } f;
  };

  enum TraceKeyword_t { RegRead, RegWrite, MemLoad, MemStore, PcWrite };

  // Generic record so we can preserve code ordering of events in trace
  //     // register       memory
  // a;  // reg             adr
  // b;  // value           len
  // c;  // origin(TODO)    data (limit 8 bytes)
  struct TraceRec_t {
    TraceKeyword_t key;
                 // register        memory
    uint64_t a;  // reg             adr
    uint64_t b;  // value           len
    uint64_t c;  // origin(TODO)    data (limited to show 8 bytes)
    TraceRec_t() {};
    TraceRec_t(TraceKeyword_t Key, uint64_t A, uint64_t B, uint64_t C=0)
      : key(Key), a(A), b(B), c(C) {};
  };

  
  class RevTracer{
    public:
      RevTracer(unsigned Verbosity, std::string Name);
      ~RevTracer();

      int SetDisassembler(std::string machine);
      int SetTraceSymbols(std::map<uint64_t,std::string>* TraceSymbols);
      void CheckUserControls();
      void SetFetchedInsn(uint64_t _pc, uint32_t _insn);
      bool OutputEnabled();

      // 32 or 64 bit integer register file captures
      // Assumes max 2 read ports and 1 write port
      void regRead(uint8_t r, uint64_t v);
      void regRead(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1);
      void regRead(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1, uint8_t r2, uint64_t v2);
      void regWrite(uint8_t r, uint64_t v);

      // Within the generic read and write functions in RevMem
      // I don't know how to differentiate initial loader, cache
      // and other activity not initiated by user assembly code.
      // For the protype, reading the source register will enable
      // the memory trace. The memory trace capture will then 
      // disable it again.

      void regRead4Mem(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1);
      void memWrite(uint64_t adr, unsigned len, void *data);
      void memRead(uint64_t adr, unsigned len, void *data);
      void pcWrite(uint64_t newpc);

      // output 
      std::string RenderOneLiner();
      void SetOutputEnable(bool e) {outputEnabled=e;}
      void Reset();

    private:
      std::string name;
      isa_parser_t* isaParser;
      disassembler_t* diasm;
      bool outputEnabled;   // disable output but continue capturing
      bool memTraceEnable; // used to avoid memory tracing not related to instruction.
      TraceEvents_t events;
      std::vector<TraceRec_t> traceRecs;
      uint64_t pc;
      uint32_t insn;
      std::map<uint64_t,std::string>* traceSymbols;
    
      // formatters
      void fmt_reg(uint8_t r, std::stringstream& s);
      void fmt_data(unsigned len, uint64_t data, std::stringstream& s);

    }; // class RevTracer

  } //namespace RevCPU
} //namespace SST

#endif //  _SST_REVCPU_REVTRACER_H_
