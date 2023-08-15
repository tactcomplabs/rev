//
// _RevTracer_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include <sstream>
#include <iomanip>
#include <string>

#include "../include/RevTracer.h"
#include "../include/kg_globals.h"
#include "RevTracer.h"

using namespace SST::RevCPU;
using namespace std;

RevTracer::RevTracer(unsigned Verbosity, std::string Name)
: name(Name), outputEnabled(true), memTraceEnable(false), insn(0)
{
    //cout << "Creating RevTracer(" << Name << ")" << endl;
}

SST::RevCPU::RevTracer::~RevTracer()
{
    if (diasm) delete diasm;
    if (isaParser) delete isaParser;
}

int SST::RevCPU::RevTracer::SetDisassembler(std::string machine)
{
    try {
        // TODO what are options for privelege level (eg. MSU)
        isaParser = new isa_parser_t(machine.c_str(),"MSU");
        diasm = new disassembler_t(isaParser);
    } catch (...) {
        return 1;
    }
    return 0;
}

void SST::RevCPU::RevTracer::CheckUserControls()
{
    if (insn == MAGIC_INST) {
        outputEnabled = !outputEnabled;
        events.f.trc_ctl = 1; 
    }
}

void SST::RevCPU::RevTracer::SetFetchedInsn(uint64_t _pc, uint32_t _insn)
{
    insn = _insn;
    pc = _pc;
}

bool SST::RevCPU::RevTracer::OutputEnabled()
{
    return outputEnabled or events.f.trc_ctl;
}

void SST::RevCPU::RevTracer::regRead(uint8_t r, uint64_t v)
{
    traceRecs.emplace_back(TraceRec_t(RegRead,r,v));
}

void SST::RevCPU::RevTracer::regRead(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1)
{
   //regReads.emplace_back(TraceRegister_t(r0,v0), TraceRegister_t(r1,v1));
   traceRecs.emplace_back(TraceRec_t(RegRead,r0,v0));
   traceRecs.emplace_back(TraceRec_t(RegRead,r1,v1));
}

void SST::RevCPU::RevTracer::regRead(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1, uint8_t r2, uint64_t v2)
{
    traceRecs.emplace_back(TraceRec_t(RegRead,r0,v0));
    traceRecs.emplace_back(TraceRec_t(RegRead,r1,v1));
    traceRecs.emplace_back(TraceRec_t(RegRead,r2,v2));
}

void SST::RevCPU::RevTracer::regWrite(uint8_t r, uint64_t v)
{
    traceRecs.emplace_back(TraceRec_t(RegWrite,r,v));
}

void SST::RevCPU::RevTracer::regRead4Mem(uint8_t r0, uint64_t v0, uint8_t r1, uint64_t v1)
{
    regRead(r0,v0,r1,v1);
    memTraceEnable = true;
}

void SST::RevCPU::RevTracer::memWrite(uint64_t adr, unsigned len, void *data)
{
    if (!memTraceEnable) return;
    memTraceEnable=false;
    // Only tracing the first 64 bytes. Retaining pointer in case we change that.
    uint64_t d = *((uint64_t*) data);
    if (len<8) {
        // zero out garbage bytes
        unsigned shift = (8-len)*8;
        uint64_t mask = (~0ULL) >> shift;
        d = d & mask;
    }
    traceRecs.emplace_back(TraceRec_t(MemStore,adr,len,d));
}

void SST::RevCPU::RevTracer::memRead(uint64_t adr, unsigned len, void *data)
{
    if (!memTraceEnable) return;
    memTraceEnable=false;
    traceRecs.emplace_back(TraceRec_t(MemLoad,adr,len,0)); 
}

void SST::RevCPU::RevTracer::pcWrite(uint64_t newpc)
{
    traceRecs.emplace_back(TraceRec_t(PcWrite,newpc,0,0));
}

std::string SST::RevCPU::RevTracer::RenderOneLiner()
{
    // Flow Control Events
    std::stringstream ss_events;
    if (events.v) {
        if (events.f.trc_ctl) {
            EVENT_SYMBOL e = outputEnabled ? EVENT_SYMBOL::TRACE_ON : EVENT_SYMBOL::TRACE_OFF;
            ss_events << event2char.at(e);
        }
    }
    
    // Disassembly
    std::stringstream ss_disasm;
    if (diasm)
        ss_disasm << hex << diasm->disassemble(insn) << "\t";
    else
        ss_disasm << hex << setw(8) << setfill('0') << hex << "0x" << insn << "\t";

    // Initial rendering
    stringstream os;
    os << "0x" << hex << pc << ":" << setfill('0') << setw(8) << insn;
    os << " " << setfill(' ') << setw(2) << ss_events.str() << " " << ss_disasm.str();

    // register and memory read/write events preserving code ordering
    if (traceRecs.empty()) 
        return os.str();

    std::stringstream ss_rw;
    for (TraceRec_t r : traceRecs) {
        switch (r.key) {
            case RegRead:
                // a:reg b:data
                ss_rw << "0x" << hex << r.b << "<-";
                fmt_reg(r.a, ss_rw);
                ss_rw << " ";
                break;
            case RegWrite:
                // a:reg b:data
                fmt_reg(r.a, ss_rw);
                ss_rw << "<-0x" << hex << r.b << " ";
                break;
            case MemStore:
            {
                // a:adr b:len c:data
                ss_rw << "[0x" << hex << r.a << "," << dec << r.b << "]<-";
                fmt_data(r.b, r.c, ss_rw);
                ss_rw << " ";
                break;
            }
            case MemLoad:
                // a:adr b:len c:data
                fmt_data(r.b, r.c, ss_rw);
                ss_rw << "<-[0x" << hex << r.a << "," << dec << r.b << "]";
                ss_rw << " ";
                break;
            case PcWrite:
                // a:pc
                ss_rw << "pc<-0x" << hex << r.a << " ";
            // default:
            //     ss_rw << "<?>";
            //     break;
        }
    }

    // Finalize string
    os << " " << ss_rw.str();
    return os.str();
}

void SST::RevCPU::RevTracer::Reset()
{
    events.v = 0;
    // save processing time and only clear the essentials.
    // pc = 0;
    // insn = 0;
    traceRecs.clear();
}

void SST::RevCPU::RevTracer::fmt_reg(uint8_t r, std::stringstream& s)
{
    if (r<32) {
        s<<xpr_name[r];
        return;
    }
    s << "?" << (unsigned)r;
}

void SST::RevCPU::RevTracer::fmt_data(unsigned len, uint64_t d, std::stringstream &s)
{
    if (len==0) return;
    s << "0x" << hex << setfill('0');
    if (len > 8)
        s << setw(8 * 2) << d << "..+" << dec << (8-len);
    else if (len == 8)
        s << setw(8 * 2) << d;
    else {
        unsigned shift = (8-len) * 8;
        uint64_t mask = (~0ULL) >> shift;
        s << setw(len * 2) << (d & mask);
    }
}
