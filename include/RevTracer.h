//
// _RevTracer_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#ifndef _SST_REVCPU_REVTRACER_H_
#define _SST_REVCPU_REVTRACER_H_

// -- SST Headers
#include "SST.h"

// -- Standard Headers
#include <cstdint>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

// -- Rev Headers
#include "RevCommon.h"

// Integrated Disassembler (toolchain dependent)
#ifdef REV_USE_SPIKE
// sst and spike cause redefinition warning for macro UNUSED.
// sst: #define UNUSED(x) x __attribute__((unused))
// spike: # define UNUSED __attribute__ ((unused))
// TODO can these be uniquified?
#ifdef __GNUC__
#undef UNUSED
#endif
#include "riscv/disasm.h"
#endif

// Tracing macros
// clang-format off
#ifndef NO_REV_TRACER
#define TRACE_REG_READ(R,V)  do{ if (Tracer) Tracer->regRead( (R), (V) ); }while(0)
#define TRACE_REG_WRITE(R,V) do{ if (Tracer) Tracer->regWrite( (R), (V) ); }while(0)
#define TRACE_PC_WRITE(PC)   do{ if (Tracer) Tracer->pcWrite( (PC) ); }while(0)
#define TRACE_MEM_WRITE(ADR, LEN, DATA)    do{ if (Tracer) Tracer->memWrite( (ADR), (LEN), (DATA) ); }while(0)
#define TRACE_MEM_READ(ADR, LEN, DATA)     do{ if (Tracer) Tracer->memRead(  (ADR), (LEN), (DATA) ); }while(0)
#define TRACE_MEMH_SENDREAD(ADR, LEN, REG) do{ if (Tracer) Tracer->memhSendRead( (ADR), (LEN), (REG) ); }while(0)
#define TRACE_MEM_READ_RESPONSE(LEN, DATA, REQ) do{ if (Tracer) Tracer->memReadResponse( (LEN), (DATA), (REQ) ); }while(0)
#else
#define TRACE_REG_READ(R,V)
#define TRACE_REG_WRITE(R,V)
#define TRACE_PC_WRITE(PC)
#define TRACE_MEM_WRITE(ADR, LEN, DATA)
#define TRACE_MEM_READ(ADR, LEN, DATA)
#define TRACE_MEMH_SENDREAD(ADR, LEN, REG)
#define TRACE_MEM_READ_RESPONSE(LEN, DATA, REQ)
#endif
// clang-format on

namespace SST::RevCPU {

// Tracing controls are using custom hint SLTI rd = x0
// See unpriv-isa-asiidoc.pdf Section 2.9 HINT Instruction
// OP     RD    Space  Assembly           Encoding
// SLTI   rd=x0 2^17   slti  x0, x0, ?    0x00?02013  //
// SLTIU  rd=x0 2^17   sltiu x0, x0, ?    0x00?03013  //
// SLLI   rd=x0 2^10   slli  x0, x0, ?    0x00?01013  // Default
// SRLI   rd=x0 2^10   srli  x0, x0, ?    0x00?05013  //
// SRAI   rd=x0 2^10   srai  x0, x0, ?    0x40?05013  //
// SLT    rd=x0 2^10   slt   x0, x0, x?   0x00?02033  // Not supported
// SLTU   rd=x0 2^10   sltu  x0, x0, x?   0x00?03033  // Not supported
const std::map<std::string, uint32_t> s2op{
  { "slti", 0x00002013},
  {"sltiu", 0x00003013},
  { "slli", 0x00001013},
  { "srli", 0x00005013},
  { "srai", 0x40005013},
};
const std::string TRC_OP_DEFAULT = "slli";
const int         TRC_OP_POS     = 20;

// Position of fully formed instruction in 'nops' array
enum class TRC_CMD_IDX : unsigned {
  TRACE_OFF      = 0,
  TRACE_ON       = 1,
  TRACE_PUSH_OFF = 2,
  TRACE_PUSH_ON  = 3,
  TRACE_POP      = 4,
};

constexpr unsigned NOP_COUNT = 5;  // must match TRC_CMD_IDX size

enum class EVENT_SYMBOL : unsigned {
  OK        = 0x0,
  STALL     = 0x1,
  FLUSH     = 0x2,
  TRACE_ON  = 0x100,
  TRACE_OFF = 0x102,
};

const std::map<EVENT_SYMBOL, char> event2char{
  {       EVENT_SYMBOL::OK, ' '},
  {    EVENT_SYMBOL::STALL, '#'},
  {    EVENT_SYMBOL::FLUSH, '!'},
  { EVENT_SYMBOL::TRACE_ON, '+'},
  {EVENT_SYMBOL::TRACE_OFF, '-'}
};

union TraceEvents_t {
  uint64_t v = 0;

  struct {
    // define 1 bit per stall type
    uint64_t stall         : 1;   // [0]
    uint64_t stall_sources : 15;  // [15:1]
    uint64_t flush         : 1;   // [16]
    uint64_t flush_sources : 15;  // [31:17]
    uint64_t spare         : 31;  // [62:32]
    uint64_t trc_ctl       : 1;   // [63] indicate change in trace controls
  } f;
};

enum TraceKeyword_t {
  RegRead,       // register read (non-fp)
  RegWrite,      // register write (non-fp)
  MemLoad,       // issue load request (simple mem)
  MemStore,      // issue store request (simple mem)
  MemhSendLoad,  // issue load request (memh)
  PcWrite,       // write program counter
};

// Generic record so we can preserve code ordering of events in trace
//     // register       memory                   memh
// a;  // reg             adr                     adr
// b;  // value           len                     len
// c;  // origin(TODO)    data (limit 8 bytes)    reg
struct TraceRec_t {
  TraceKeyword_t key;
  // register        memory                  memh
  uint64_t a;  // reg             adr                     adr
  uint64_t b;  // value           len                     len
  uint64_t c;  // origin(TODO)    data (limited 8 bytes)  reg
  TraceRec_t( TraceKeyword_t Key, uint64_t A, uint64_t B, uint64_t C = 0 ) : key( Key ), a( A ), b( B ), c( C ) {};
};

struct InstHeader_t {
  size_t       cycle;
  unsigned     id;
  unsigned     hart;
  unsigned     tid;
  std::string  defaultMnem      = "?";
  std::string& fallbackMnemonic = defaultMnem;
  bool         valid            = false;

  void set( size_t _cycle, unsigned _id, unsigned _hart, unsigned _tid, const std::string& _fallback ) {
    cycle            = _cycle;
    id               = _id;
    hart             = _hart;
    tid              = _tid;
    fallbackMnemonic = _fallback;
    valid            = true;
  };

  void clear() { valid = false; }
};

// aggregate read completion (memh)
struct CompletionRec_t {
  unsigned int hart;
  uint16_t     destReg;
  size_t       len;
  uint64_t     addr;
  uint64_t     data;  // first 8 bytes max
  bool         isFloat         = false;
  bool         wait4Completion = false;

  CompletionRec_t( unsigned int hart, uint16_t destReg, size_t len, uint64_t addr, void* data, RevRegClass regClass )
    : hart( hart ), destReg( destReg ), len( len ), addr( addr ), isFloat( regClass == RevRegClass::RegFLOAT ),
      wait4Completion( true ) {
    memcpy( &this->data, data, len > sizeof( this->data ) ? sizeof( this->data ) : len );
  }
};

class RevTracer {
public:
  /// RevTracer: standard constructor standard constructor
  RevTracer( std::string Name, SST::Output* output );
  /// RevTracer:standard destructor
  ~RevTracer();

  /// RevTracer: assign disassembler. Returns 0 if successful
  int SetDisassembler( std::string machine );
  /// RevTracer: assign trace symbol lookup map
  void SetTraceSymbols( std::map<uint64_t, std::string>* TraceSymbols );
  /// RevTracer: assign cycle where trace will start (user param)
  void SetStartCycle( uint64_t c );
  /// RevTracer: assign maximum output lines (user param)
  void SetCycleLimit( uint64_t c );
  /// RevTracer:assign instruction used for trace controls
  void SetCmdTemplate( std::string cmd );
  /// RevTracer: capture instruction to be traced
  void SetFetchedInsn( uint64_t _pc, uint32_t _insn );
  /// RevTracer: capture register read
  void regRead( size_t r, uint64_t v );
  /// RevTracer: capture register write.
  void regWrite( size_t r, uint64_t v );
  /// RevTracer: capture memory write.
  void memWrite( uint64_t adr, size_t len, const void* data );
  /// RevTracer: capture memory read
  void memRead( uint64_t adr, size_t len, void* data );
  /// RevTracer: memh read request
  void memhSendRead( uint64_t addr, size_t len, uint16_t reg );
  /// RevTracer: data returning from memory read (memh)
  void memReadResponse( size_t len, void* data, const MemReq* req );
  /// RevTracer: capture 32-bit program counter
  void pcWrite( uint32_t newpc );
  /// RevTracer: capture 64-bit program counter
  void pcWrite( uint64_t newpc );
  /// RevTracer: render instruction execution trace to output stream and update tracer state
  void Exec( size_t cycle, unsigned id, unsigned hart, unsigned tid, const std::string& fallbackMnemonic );
  /// RevTracer: Render captured states
  void Render( size_t cycle );

  /// RevTracer: control whether to render captured states
  void SetOutputEnable( bool e ) { outputEnabled = e; }

  /// Reset trace state
  void Reset();

private:
  /// RevTracer: clear instruction trace capture buffer and reset trace state
  void InstTraceReset();

  /// RevTracer: instance name
  std::string name;
#ifdef REV_USE_SPIKE
  /// RevTracer: instruction parser used by disassembler
  isa_parser_t* isaParser;
  /// RevTracer: disassembler
  disassembler_t* diasm;
#endif
  /// RevTracer: pointer to output stream
  SST::Output* pOutput;
  /// RevTracer: control whether output is printed or not ( sampling continues )
  bool outputEnabled = false;
  /// RevTracer: Instruction header captured at execution phase.
  InstHeader_t instHeader;
  /// RevTracer: Special affecting trace output
  TraceEvents_t events;
  /// RevTracer: buffer for captured states
  std::vector<TraceRec_t> traceRecs;
  /// RevTracer: Completion records
  std::vector<CompletionRec_t> completionRecs;
  /// RevTracer: saved program counter
  uint64_t pc                                   = 0;
  /// RevTracer: previous program counter for branch determination
  uint64_t lastPC                               = 0;
  /// RevTracer: saved instruction
  uint32_t insn                                 = 0;
  /// RevTracer: map of instruction addresses to symbols
  std::map<uint64_t, std::string>* traceSymbols = nullptr;
  /// RevTracer: Array of supported "NOP" instructions avaible for trace controls
  uint32_t nops[NOP_COUNT];
  /// RevTracer: Check current state against user settings and update state
  void CheckUserControls( uint64_t cycle );
  /// RevTracer: determine if this buffer should be rendered
  bool OutputOK();
  /// RevTracer: format register address for rendering
  std::string fmt_reg( uint8_t r );
  /// RevTracer: Format data associated with memory access
  std::string fmt_data( unsigned len, uint64_t data );
  /// RevTracer: Generate string from captured state
  std::string RenderExec( const std::string& fallbackMnemonic );
  /// RevTracer: User setting: starting cycle of trace (overrides programmtic control)
  uint64_t startCycle = 0;
  /// RevTracer: User setting: maximum number of lines to print
  uint64_t cycleLimit = 0;
  /// RevTracer: support for trace control push/pop
  std::vector<bool> enableQ;
  /// RevTracer: current pointer into trace controls queue
  unsigned enableQindex;
  /// RevTracer: wraparound limit for trace controls queue
  const unsigned MAX_ENABLE_Q = 100;
  /// RevTracer: count of lines rendered
  uint64_t traceCycles        = 0;
  /// RevTracer: Hard disable for output
  bool disabled               = 0;

};  // class RevTracer

}  //namespace SST::RevCPU

#endif  //  _SST_REVCPU_REVTRACER_H_
