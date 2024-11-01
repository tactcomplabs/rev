//
// _RevTracer_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include <iomanip>
#include <sstream>
#include <string>

#include "RevTracer.h"

namespace SST::RevCPU {

RevTracer::RevTracer( std::string Name, SST::Output* o ) : name( Name ), pOutput( o ) {

  enableQ.resize( MAX_ENABLE_Q );
  enableQ.assign( MAX_ENABLE_Q, 0 );
  enableQindex          = 0;

  // Initialize NOP trace controls
  uint32_t cmd_template = s2op.at( TRC_OP_DEFAULT );
  for( unsigned i = 0; i < NOP_COUNT; i++ )
    nops[i] = cmd_template | ( i << TRC_OP_POS );

#if 1
  if( std::getenv( "REV_SPINNER" ) ) {
    uint64_t spinner = 1;
    std::cout << "spinner active" << std::endl;
    while( spinner ) {
      spinner++;
      if( spinner % 10000000 == 0 )  // break here
        std::cout << ".";
    }
    std::cout << std::endl;
  }
#endif
}

RevTracer::~RevTracer() {
#ifdef REV_USE_SPIKE
  if( diasm )
    delete diasm;
  if( isaParser )
    delete isaParser;
#endif
}

int RevTracer::SetDisassembler( std::string machine ) {
#ifdef REV_USE_SPIKE
  try {
    // TODO privelege level options
    isaParser = new isa_parser_t( machine.c_str(), "MSU" );
    diasm     = new disassembler_t( isaParser );
  } catch( ... ) {
    return 1;
  }
  return 0;
#else
  return 1;
#endif
}

void RevTracer::SetTraceSymbols( std::map<uint64_t, std::string>* TraceSymbols ) {
  traceSymbols = TraceSymbols;
}

void RevTracer::SetStartCycle( uint64_t c ) {
  startCycle = c;
}

void RevTracer::SetCycleLimit( uint64_t c ) {
  cycleLimit = c;
}

void RevTracer::SetCmdTemplate( std::string cmd ) {
  if( s2op.find( cmd ) == s2op.end() ) {
    std::stringstream s;
    for( auto it = s2op.begin(); it != s2op.end(); it++ ) {
      s << it->first << " ";
    }
    pOutput->fatal( CALL_INFO, -1, "Unsupported parameter [trcCmd=%s]. Supported values are: %s\n", cmd.c_str(), s.str().c_str() );
  }

  unsigned cmd_template = s2op.at( cmd );
  for( unsigned i = 0; i < NOP_COUNT; i++ )
    nops[i] = cmd_template | ( i << TRC_OP_POS );
}

void RevTracer::CheckUserControls( uint64_t cycle ) {
  // bail out early if disabled
  if( disabled )
    return;
  if( cycleLimit && traceCycles > cycleLimit ) {
    disabled = true;
    if( outputEnabled ) {
      outputEnabled    = false;
      events.f.trc_ctl = 1;
    }
    return;
  }

  // Using a startCycle will override programmatic controls.
  if( startCycle > 0 ) {
    bool enable = startCycle && cycle > startCycle;
    if( enable != outputEnabled ) {
      outputEnabled    = enable;
      events.f.trc_ctl = 1;
    }
    return;
  }

  // programatic controls
  bool nextState = outputEnabled;
  if( insn == nops[safe_static_cast<unsigned>( TRC_CMD_IDX::TRACE_OFF )] ) {
    nextState = false;
  } else if( insn == nops[safe_static_cast<unsigned>( TRC_CMD_IDX::TRACE_ON )] ) {
    nextState = true;
  } else if( insn == nops[safe_static_cast<unsigned>( TRC_CMD_IDX::TRACE_PUSH_OFF )] ) {
    enableQ[enableQindex] = outputEnabled;
    enableQindex          = ( enableQindex + 1 ) % MAX_ENABLE_Q;
    nextState             = false;
  } else if( insn == nops[safe_static_cast<unsigned>( TRC_CMD_IDX::TRACE_PUSH_ON )] ) {
    enableQ[enableQindex] = outputEnabled;
    enableQindex          = ( enableQindex + 1 ) % MAX_ENABLE_Q;
    nextState             = true;
  } else if( insn == nops[safe_static_cast<unsigned>( TRC_CMD_IDX::TRACE_POP )] ) {
    enableQindex = ( enableQindex - 1 ) % MAX_ENABLE_Q;
    nextState    = enableQ[enableQindex];
  }
  // prevent trace clutter with unnecessary events
  if( nextState != outputEnabled ) {
    events.f.trc_ctl = 1;
    outputEnabled    = nextState;
  }
}

void RevTracer::SetFetchedInsn( uint64_t _pc, uint32_t _insn ) {
  insn = _insn;
  pc   = _pc;
}

bool RevTracer::OutputOK() {
  return outputEnabled || events.f.trc_ctl;
}

void RevTracer::regRead( size_t r, uint64_t v ) {
  traceRecs.emplace_back( TraceRec_t( RegRead, r, v ) );
}

void RevTracer::regWrite( size_t r, uint64_t v ) {
  traceRecs.emplace_back( TraceRec_t( RegWrite, r, v ) );
}

void RevTracer::memWrite( uint64_t adr, size_t len, const void* data ) {
  // Only tracing the first 8 bytes. Retaining pointer in case we change that.
  uint64_t d = 0;
  memcpy( &d, data, len > sizeof( d ) ? sizeof( d ) : len );
  traceRecs.emplace_back( TraceRec_t( MemStore, adr, len, d ) );
}

void RevTracer::memRead( uint64_t adr, size_t len, void* data ) {
  uint64_t d = 0;
  memcpy( &d, data, len > sizeof( d ) ? sizeof( d ) : len );
  traceRecs.emplace_back( TraceRec_t( MemLoad, adr, len, d ) );
}

void SST::RevCPU::RevTracer::memhSendRead( uint64_t adr, size_t len, uint16_t reg ) {
  traceRecs.emplace_back( TraceRec_t( MemhSendLoad, adr, len, reg ) );
}

void RevTracer::memReadResponse( size_t len, void* data, const MemReq* req ) {
  if( req->DestReg == 0 )
    return;
  CompletionRec_t c( req->Hart, req->DestReg, len, req->Addr, data, req->RegType );
  completionRecs.emplace_back( c );
}

void RevTracer::pcWrite( uint32_t newpc ) {
  traceRecs.emplace_back( TraceRec_t( PcWrite, newpc, 0, 0 ) );
}

void RevTracer::pcWrite( uint64_t newpc ) {
  traceRecs.emplace_back( TraceRec_t( PcWrite, newpc, 0, 0 ) );
}

void RevTracer::Exec( size_t cycle, unsigned id, unsigned hart, unsigned tid, const std::string& fallbackMnemonic ) {
  instHeader.set( cycle, id, hart, tid, fallbackMnemonic );
}

void RevTracer::Render( size_t cycle ) {
  // Trace on/off controls
  CheckUserControls( cycle );

  // memory completions
  if( completionRecs.size() > 0 ) {
    if( OutputOK() ) {
      for( auto r : completionRecs ) {
        std::string       data_str = fmt_data( r.len, r.data );
        std::stringstream s;
        s << data_str << "<-[0x" << std::hex << r.addr << "," << std::dec << r.len << "] ";
        s << fmt_reg( r.destReg ) << "<-" << data_str << " ";
        pOutput->verbose( CALL_INFO, 5, 0, "Hart %" PRIu32 "; *A %s\n", r.hart, s.str().c_str() );
      }
    }
    // reset completion reqs
    completionRecs.clear();
  }

  // Instruction Trace
  if( instHeader.valid ) {
    if( OutputOK() ) {
      pOutput->verbose(
        CALL_INFO,
        5,
        0,
        "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32 "; *I %s\n",
        instHeader.id,
        instHeader.hart,
        instHeader.tid,
        RenderExec( instHeader.fallbackMnemonic ).c_str()
      );
    }
    InstTraceReset();
  }
}

void SST::RevCPU::RevTracer::Reset() {
  InstTraceReset();
  completionRecs.clear();
}

std::string RevTracer::RenderExec( const std::string& fallbackMnemonic ) {
  // Flow Control Events
  std::stringstream ss_events;
  if( events.v ) {
    if( events.f.trc_ctl ) {
      EVENT_SYMBOL e = outputEnabled ? EVENT_SYMBOL::TRACE_ON : EVENT_SYMBOL::TRACE_OFF;
      ss_events << event2char.at( e );
    }
  }

  // Disassembly
  std::stringstream ss_disasm;
#ifdef REV_USE_SPIKE
  if( diasm )
    ss_disasm << std::hex << diasm->disassemble( insn ) << "\t";
  else
#endif
  {
    ;
// TODO internal rev disassembler
#if 0
        // only show mnemonic
        auto pos = fallbackMnemonic.find(' ');
        if (pos != std::string::npos)
            ss_disasm << fallbackMnemonic.substr(0, pos) << "\t";
        else
            ss_disasm << "?" << "\n";
#else
    // show mnemonic and field format strings.
    ss_disasm << std::left << std::setw( 20 ) << fallbackMnemonic << std::right << "\t";
#endif
  }

  // Initial rendering
  std::stringstream os;
  os << "0x" << std::hex << pc << ":" << std::setfill( '0' );
  if( ~insn & 3 ) {
    os << std::setw( 4 ) << ( insn & 0xffff ) << "    ";
  } else {
    os << std::setw( 8 ) << insn;
  }
  os << " " << std::setfill( ' ' ) << std::setw( 2 ) << ss_events.str() << " " << ss_disasm.str();

  // register and memory read/write events preserving code ordering
  if( traceRecs.empty() )
    return os.str();

  // We got something, count it and render it
  traceCycles++;

  // For Memh, the target register is corrupted after ReadVal (See RevInstHelpers.h::load)
  // This is a transitory value that would only be observed if the register file is accessible
  // by an external agent (e.g. IO or JTAG scan). A functional issue would occur
  // if an error response is received from the network in which case the scoreboard
  // should be cleared and the register would be expected to retain its previous value.
  // Until this is resolved, the trace supresses the extra register write when
  // we encounter a memhSendRead event.
  // TODO remove this if/when extra register write is removed.
  bool squashNextSetX = false;

  std::stringstream ss_rw;
  for( TraceRec_t r : traceRecs ) {
    switch( r.key ) {
    case RegRead:
      // a:reg b:data
      ss_rw << "0x" << std::hex << r.b << "<-";
      ss_rw << fmt_reg( r.a ) << " ";
      break;
    case RegWrite:
      // a:reg b:data
      if( squashNextSetX ) {
        squashNextSetX = false;
      } else {
        ss_rw << fmt_reg( r.a ) << "<-0x" << std::hex << r.b << " ";
      }
      break;
    case MemStore: {
      // a:adr b:len c:data
      ss_rw << "[0x" << std::hex << r.a << "," << std::dec << r.b << "]<-" << fmt_data( r.b, r.c ) << " ";
      break;
    }
    case MemLoad:
      // a:adr b:len c:data
      ss_rw << fmt_data( r.b, r.c ) << "<-[0x" << std::hex << r.a << "," << std::dec << r.b << "]";
      ss_rw << " ";
      break;
    case MemhSendLoad:
      // a:adr b:len c:reg
      ss_rw << fmt_reg( r.c ) << "<-[0x" << std::hex << r.a << "," << std::dec << r.b << "]";
      ss_rw << " ";
      squashNextSetX = true;  // register corrupted after ReadVal in RevInstHelpers.h::load
      break;
    case PcWrite:
      // a:pc
      uint64_t pc = r.a;
      if( lastPC + 4 != pc ) {
        // only render if non-sequential instruction
        ss_rw << "pc<-0x" << std::hex << pc;
        if( traceSymbols and ( traceSymbols->find( pc ) != traceSymbols->end() ) )
          ss_rw << " <" << traceSymbols->at( pc ) << ">";
        ss_rw << " ";
      }
      lastPC = pc;
      break;
    }
  }

  // Finalize string
  os << " " << ss_rw.str();
  return os.str();
}

void RevTracer::InstTraceReset() {
  events.v = 0;
  insn     = 0;
  traceRecs.clear();
  instHeader.clear();
}

std::string RevTracer::fmt_reg( uint8_t r ) {
  std::stringstream s;
#ifdef REV_USE_SPIKE
  if( r < 32 ) {
    s << xpr_name[r];  // defined in disasm.h
    return s.str();
  }
  s << "?" << (unsigned) r;
#else
  s << "x" << std::dec << (uint16_t) r;  // Use SST::RevCPU::RevReg?
#endif
  return s.str();
}

std::string RevTracer::fmt_data( unsigned len, uint64_t d ) {
  std::stringstream s;
  if( len == 0 )
    return "";
  s << "0x" << std::hex << std::setfill( '0' );
  if( len > 8 )
    s << std::setw( 8 * 2 ) << d << "..+" << std::dec << ( 8 - len );
  else if( len == 8 )
    s << std::setw( 8 * 2 ) << d;
  else {
    unsigned shift = ( 8 - len ) * 8;
    uint64_t mask  = ( ~0ULL ) >> shift;
    s << std::setw( len * 2 ) << ( d & mask );
  }
  return s.str();
}

}  // namespace SST::RevCPU
