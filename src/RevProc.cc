//
// _RevProc_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevProc.h"
#include "RevInstTable.h"
#include "RevSysCalls.cc"
#include <bitset>
#include <filesystem>
#include <sys/xattr.h>

RevProc::RevProc( unsigned Id,
                  RevOpts *Opts,
                  RevMem *Mem,
                  RevLoader *Loader,
                  std::vector<std::shared_ptr<RevThread>>& AssignedThreads,
                  RevCoProc* CoProc,
                  SST::Output *Output )
  : Halted(false), Stalled(false), SingleStep(false),
    CrackFault(false), ALUFault(false), fault_width(0),
    id(Id), HartToDecode(0), HartToExec(0), Retired(0x00ull),
    opts(Opts), mem(Mem), loader(Loader), AssignedThreads(AssignedThreads),
    output(Output), feature(nullptr), PExec(nullptr), sfetch(nullptr) {

  // initialize the machine model for the target core
  std::string Machine;
  if( !Opts->GetMachineModel(id,Machine) )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve the machine model for core=%d\n", id);

  unsigned MinCost = 0;
  unsigned MaxCost = 0;

  Opts->GetMemCost(Id,MinCost,MaxCost);

  if(CoProc){
    coProc = CoProc;
  }else{
    coProc = NULL;
  }

  feature = new RevFeature(Machine,output,MinCost,MaxCost,Id);
  if( !feature )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevFeature object for core=%d\n", id);

  unsigned Depth = 0;
  Opts->GetPrefetchDepth(Id, Depth);
  if( Depth == 0 ){
    Depth = 16;
  }

  sfetch = new RevPrefetcher(Mem,feature,Depth);
  if( !sfetch )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevPrefetcher object for core=%d\n", id);
 
  // load the instruction tables
  if( !LoadInstructionTable() )
    output->fatal(CALL_INFO, -1,
                  "Error : failed to load instruction table for core=%d\n", id );

  // Initialize EcallTable
  InitEcallTable();
  if( Ecalls.size() <= 0 )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to initialize the Ecall Table for core=%d\n", id );

  ECALL_buf = new char[64];
  ECALL_buf[0] = '\0';
  ECALL_string = "";
  ECALL_bytesRead = 0;

  // reset the core
  if( !Reset() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to reset the core resources for core=%d\n", id );

  Stats.totalCycles = 0;
  Stats.cyclesBusy = 0;
  Stats.cyclesIdle_Total = 0;
  Stats.cyclesIdle_Pipeline = 0;
  Stats.cyclesIdle_MemoryFetch= 0;
  Stats.cyclesStalled = 0;
  Stats.percentEff = 0.0;
  Stats.floatsExec = 0;
}

RevProc::~RevProc(){
  for( unsigned i=0; i<Extensions.size(); i++ )
    delete Extensions[i];
  delete sfetch;
  delete feature;
  delete[] ECALL_buf;
}

RevProc::RevProcStats RevProc::GetStats(){
  Stats.memStats.bytesRead      = mem->memStats.bytesRead;
  Stats.memStats.bytesWritten   = mem->memStats.bytesWritten;
  Stats.memStats.doublesRead    = mem->memStats.doublesRead;
  Stats.memStats.doublesWritten = mem->memStats.doublesWritten;
  Stats.memStats.floatsRead     = mem->memStats.floatsRead;
  Stats.memStats.floatsWritten  = mem->memStats.floatsWritten;
  Stats.memStats.TLBMisses      = mem->memStats.TLBMisses;
  Stats.memStats.TLBHits        = mem->memStats.TLBHits;
  return Stats;
}

bool RevProc::Halt(){
  if( Halted )
    return false;
  Halted = true;
  SingleStep = false;
  return true;
}

bool RevProc::Resume(){
  if( Halted ){
    Halted = false;
    SingleStep = false;
    return true;
  }
  return false;
}

bool RevProc::SingleStepHart(){
  if( SingleStep )
    return true;
  if( Halted ){
    Halted = false;
    SingleStep = true;
    return true;
  }else{
    // must be halted to single step
    return false;
  }
}

bool RevProc::EnableExt(RevExt *Ext, bool Opt){
  if( !Ext )
    output->fatal(CALL_INFO, -1, "Error: failed to initialize RISC-V extensions\n");

  output->verbose(CALL_INFO, 6, 0,
                  "Core %d ; Enabling extension=%s\n",
                  id, Ext->GetName().c_str());

  // add the extension to our vector of enabled objects
  Extensions.push_back(Ext);

  // retrieve all the target instructions
  std::vector<RevInstEntry> IT = Ext->GetInstTable();

  // setup the mapping of InstTable to Ext objects
  InstTable.reserve(InstTable.size() + IT.size());

  for( unsigned i=0; i<IT.size(); i++ ){
    InstTable.push_back(IT[i]);
    std::pair<unsigned,unsigned> ExtObj =
      std::pair<unsigned,unsigned>(Extensions.size()-1,i);
    EntryToExt.insert(
      std::pair<unsigned,
        std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
  }

  // load the compressed instructions
  if( feature->IsModeEnabled(RV_C) ){
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Enabling compressed extension=%s\n",
                    id, Ext->GetName().c_str());

    std::vector<RevInstEntry> CT = Ext->GetCInstTable();
    InstTable.reserve(InstTable.size() + CT.size());

    for( unsigned i=0; i<CT.size(); i++ ){
      InstTable.push_back(CT[i]);
      std::pair<unsigned,unsigned> ExtObj =
        std::pair<unsigned,unsigned>(Extensions.size()-1,i);
      EntryToExt.insert(
        std::pair<unsigned,
          std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
    }
    // load the optional compressed instructions
    if( Opt ){
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Enabling optional compressed extension=%s\n",
                      id, Ext->GetName().c_str());
      CT = Ext->GetOInstTable();

      InstTable.reserve(InstTable.size() + CT.size());

      for( unsigned i=0; i<CT.size(); i++ ){
        InstTable.push_back(CT[i]);
        std::pair<unsigned,unsigned> ExtObj =
          std::pair<unsigned,unsigned>(Extensions.size()-1,i);
        EntryToExt.insert(
          std::pair<unsigned,
            std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
      }
    }
  }

  return true;
}

bool RevProc::SeedInstTable(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Seeding instruction table for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  
  // I-Extension
  if( feature->IsModeEnabled(RV_I) ){
    if( feature->GetXlen() == 64 ){
      // load RV32I & RV64; no optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile,mem,output)),false);
      EnableExt(static_cast<RevExt *>(new RV64I(feature,RegFile,mem,output)),false);
    }else{
      // load RV32I w/ optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile,mem,output)),true);
    }
  }

  // M-Extension
  if( feature->IsModeEnabled(RV_M) ){
    EnableExt(static_cast<RevExt *>(new RV32M(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64M(feature,RegFile,mem,output)),false);
    }
  }

  // A-Extension
  if( feature->IsModeEnabled(RV_A) ){
    EnableExt(static_cast<RevExt *>(new RV32A(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64A(feature,RegFile,mem,output)),false);
    }
  }

  // F-Extension
  if( feature->IsModeEnabled(RV_F) ){
    if( (!feature->IsModeEnabled(RV_D)) && (feature->GetXlen() == 32) ){
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile,mem,output)),true);
    }else{
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile,mem,output)),false);
      EnableExt(static_cast<RevExt *>(new RV64F(feature,RegFile,mem,output)),false);

    }
#if 0
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)));
    }
#endif
  }

  // D-Extension
  if( feature->IsModeEnabled(RV_D) ){
    EnableExt(static_cast<RevExt *>(new RV32D(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)),false);
    }
  }

  // PAN Extension
  if( feature->IsModeEnabled(RV_P) ){
    EnableExt(static_cast<RevExt *>(new RV64P(feature,RegFile,mem,output)),false);
  }

  return true;
}

uint32_t RevProc::CompressCEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= (uint32_t)(Entry.opcode);
  Value |= (uint32_t)((uint32_t)(Entry.funct2)<<2);
  Value |= (uint32_t)((uint32_t)(Entry.funct3)<<4);
  Value |= (uint32_t)((uint32_t)(Entry.funct4)<<8);
  Value |= (uint32_t)((uint32_t)(Entry.funct6)<<12);

  return Value;
}

uint32_t RevProc::CompressEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= (uint32_t)(Entry.opcode);
  Value |= (uint32_t)((uint32_t)(Entry.funct3)<<8);
  Value |= (uint32_t)((uint32_t)(Entry.funct7)<<11);
  Value |= (uint32_t)((uint32_t)(Entry.imm12)<<18);
  Value |= (uint32_t)((uint32_t)(Entry.fpcvtOp)<<30);  //this is a 5 bit field, but only the lower two bits are used, so it *just* fits 
                                                          //without going to a uint64

  return Value;
}

void RevProc::splitStr(const std::string& s,
                       char c,
                       std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);

  // catch strings with no delims
  if( j == std::string::npos ){
    v.push_back(s);
  }

  // break up the rest of the string
  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }
}

std::string RevProc::ExtractMnemonic(RevInstEntry Entry){
  std::string Tmp = Entry.mnemonic;
  std::vector<std::string> vstr;
  splitStr(Tmp,' ',vstr);

  return vstr[0];
}

bool RevProc::InitTableMapping(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Initializing table mapping for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  for( unsigned i=0; i<InstTable.size(); i++ ){
    NameToEntry.insert(
      std::pair<std::string,unsigned>(ExtractMnemonic(InstTable[i]),i) );
    if( !InstTable[i].compressed ){
      // map normal instruction
      EncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Table Entry %d = %s\n",
                      id,
                      CompressEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }else{
      // map compressed instruction
      CEncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressCEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Compressed Table Entry %d = %s\n",
                      id,
                      CompressCEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }
  }
  return true;
}

bool RevProc::ReadOverrideTables(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Reading override tables for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  std::string Table;
  if( !opts->GetInstTable(id, Table) )
    return false;

  // if the length of the file name is 0, just return
  if( Table == "_REV_INTERNAL_" )
    return true;

  // open the file
  std::ifstream infile(Table);
  if( !infile.is_open() )
    output->fatal(CALL_INFO, -1, "Error: failed to read instruction table for core=%d\n", id);

  // read all the values
  std::string Inst;
  std::string Cost;
  unsigned Entry;
  std::map<std::string,unsigned>::iterator it;
  while( infile >> Inst >> Cost ){
    it = NameToEntry.find(Inst);
    if( it == NameToEntry.end() )
      output->fatal(CALL_INFO, -1, "Error: could not find instruction in table for map value=%s\n", Inst.c_str() );

    Entry = it->second;
    InstTable[Entry].cost = (unsigned)(std::stoi(Cost,nullptr,0));
  }

  // close the file
  infile.close();

  // std::cout << "Done reading the override tables" << std::endl;
  return true;
}

bool RevProc::LoadInstructionTable(){
  // Stage 1: load the instruction table for each enable feature
  if( !SeedInstTable() )
    return false;

  // Stage 2: setup the internal mapping tables for performance
  if( !InitTableMapping() )
    return false;

  // Stage 3: examine the user-defined cost tables to see if we need to override the defaults
  if( !ReadOverrideTables() )
    return false;

  return true;
}

bool RevProc::Reset(){
  // Reset the AssignedThreads
  for( auto& Thread : AssignedThreads ){
    // TODO: Make sure this is the correct way to reset the thread
    Thread.reset();
  }

  // reset the register file
  // for (int t=0;  t < _REV_HART_COUNT_; t++){
  //   RevRegFile* regFile = GetRegFile(t);
  //   regFile->RV32_PC = 0x00l;
  //   regFile->RV64_PC = 0x00ull;
  //   for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
  //     regFile->RV32[i] = 0x00l;
  //     regFile->RV64[i] = 0x00ull;
  //     regFile->SPF[i]  = 0.f;
  //     regFile->DPF[i]  = 0.f;
  //     regFile->RV32_Scoreboard[i] = false;
  //     regFile->RV64_Scoreboard[i] = false;
  //     regFile->SPF_Scoreboard[i] = false;
  //     regFile->DPF_Scoreboard[i] = false;
  //   }

  //   // initialize all the relevant program registers
  //   // -- x2 : stack pointer
  //   regFile->RV32[2] = (uint32_t)(mem->GetStackTop());
  //   regFile->RV64[2] = mem->GetStackTop();

  //   // -- x3 : global pointer
  //   regFile->RV32[3] = (uint32_t)(loader->GetSymbolAddr("__global_pointer$"));
  //   regFile->RV64[3] = loader->GetSymbolAddr("__global_pointer$");

  //   // -- x8 : frame pointer
  //   regFile->RV32[8] = regFile->RV32[3];
  //   regFile->RV64[8] = regFile->RV64[3];

  //   regFile->cost = 0;

    Pipeline.clear();
  // }

  // set the pc
  // uint64_t StartAddr = 0x00ull;
  // if( !opts->GetStartAddr( id, StartAddr ) )
  //   output->fatal(CALL_INFO, -1,
  //                 "Error: failed to init the start address for core=%d\n", id);
  // std::string StartSymbol = "main";
  // //std::string StartSymbol = "_start";
  // if( StartAddr == 0x00ull ){
  //   if( !opts->GetStartSymbol( id, StartSymbol ) )
  //     output->fatal(CALL_INFO, -1,
  //                   "Error: failed to init the start symbol address for core=%d\n", id);

  //   StartAddr = loader->GetSymbolAddr(StartSymbol);
  // }
  // if( StartAddr == 0x00ull ){
  //   // load "main" symbol
  //   StartAddr = loader->GetSymbolAddr("main");
  //   //StartAddr = loader->GetSymbolAddr("_start");
  //   if( StartAddr == 0x00ull ){
  //     output->fatal(CALL_INFO, -1,
  //                   "Error: failed to auto discover address for <main> for core=%d\n", id);
  //   }
  // }
  // for (int t=0;  t < _REV_HART_COUNT_; t++){
  //   RevRegFile* regFile = GetRegFile(t);
  //   regFile->RV32_PC = (uint32_t)(StartAddr);
  //   regFile->RV64_PC = StartAddr;
  // }
  HART_CTS.set();

  ECALL_buf[0] = '\0';
  ECALL_bytesRead = 0;
  ECALL_string.clear();

  return true;
}

void RevProc::SetupArgs(){
  auto Argv = opts->GetArgv();

  // ----------------------------------
  // We need to initialize the x10 register to include the value of ARGC
  // This is >= 1 (the executable name is always included)
  // We also need to initialize the ARGV pointer to the value
  // of the ARGV base pointer in memory which is currently set to the
  // program header region.  When we come out of reset, this is StackTop+60 bytes
  // ----------------------------------

  // calculate the total size of the argv's
  uint64_t TotalSize = 0x00ull;
  for( unsigned i=0; i<Argv.size(); i++ ){
    TotalSize += (Argv[i].size()+1);
  }

  for( int r=0; r < _REV_HART_COUNT_; r++ ){
    // setup argc
    RevRegFile* regFile = GetRegFile(r);
    regFile->RV32[10] = Argv.size();
    regFile->RV64[10] = Argv.size();

    regFile->RV32[11] = (uint32_t)(mem->GetStackTop()+60);
    regFile->RV64[11] = mem->GetStackTop()+60;
  }
}

bool RevProc::IsFloat(unsigned Entry){
  if( (InstTable[Entry].rdClass == RegFLOAT) ||
      (InstTable[Entry].rs1Class == RegFLOAT) ||
      (InstTable[Entry].rs2Class == RegFLOAT) ||
      (InstTable[Entry].rs3Class == RegFLOAT) ){
    return true;
  }
  return false;
}

RevInst RevProc::DecodeCRInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct4  = InstTable[Entry].funct4;

  // registers
  CompInst.rd      = DECODE_RD(Inst);
  CompInst.rs1     = DECODE_RD(Inst);
  CompInst.rs2     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm     = 0x00;

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCIInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = DECODE_RD(Inst);
  CompInst.rs1     = DECODE_RD(Inst);
  CompInst.imm     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm    |= ((Inst & 0b1000000000000)>>7);

  if((CompInst.opcode == 0b10) &&
     (CompInst.funct3 == 0b001)){
    // c.fldsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1100000) >> 2);        // [4:3]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    CompInst.imm |= ((Inst & 0b11100) << 4);          // [8:6]
  }else if( (CompInst.opcode == 0b10) &&
            (CompInst.funct3 == 0b010) ){
    // c.lwsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1110000) >> 2);        // [4:2]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    CompInst.imm |= ((Inst & 1100) << 4);             // [7:6]
  }else if( (CompInst.opcode == 0b10) &&
            (CompInst.funct3 == 0b011) ){
    CompInst.imm = 0;
    if( feature->GetXlen() == 64 ){
      // c.ldsp
      CompInst.imm =  ((Inst & 0b1100000) >> 2);        // [4:3]
      CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
      CompInst.imm |= ((Inst & 0b11100) << 4);          // [8:6]
    }else{
      // c.flwsp
      CompInst.imm =  ((Inst & 0b1110000) >> 2);        // [4:2]
      CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
      CompInst.imm |= ((Inst & 1100) << 4);             // [7:6]
    }
  }else if( (CompInst.opcode == 0b01) &&
      (CompInst.funct3 == 0b011) &&
      (CompInst.rd == 2)){
    // c.addi16sp
    // swizzle: nzimm[4|6|8:7|5] nzimm[9]
    CompInst.imm = 0;
    CompInst.imm = ((Inst & 0b1000000) >> 2); // bit 4
    CompInst.imm |= ((Inst & 0b100) << 3);    // bit 5
    CompInst.imm |= ((Inst & 0b100000) << 1); // bit 6
    CompInst.imm |= ((Inst & 0b11000) << 4);  // bit 8:7
    CompInst.imm |= ((Inst & 0b1000000000000) >> 3);  // bit 9
    if( (CompInst.imm & 0b1000000000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111111111111000000000;
    }
  }else if( (CompInst.opcode == 0b01) &&
      (CompInst.funct3 == 0b011)  &&
      (CompInst.rd != 0) && (CompInst.rd != 2) ){
    // c.lui
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111100) << 10);       // [16:12]
    CompInst.imm |= ((Inst & 0b1000000000000) << 5);  // [17]
    if( (CompInst.imm & 0b100000000000000000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111000000000000000000;
    }
    CompInst.imm >>= 12;  //immd value will be re-aligned on execution
  }else if( (CompInst.opcode == 0b01) &&
            (CompInst.funct3 == 0b010) &&
            (CompInst.rd != 0) ){
    // c.li
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111100) >> 2);        // [4:0]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    if( (CompInst.imm & 0b100000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111111111111111000000;
    }
  }else if( (CompInst.imm & 0b100000) > 0 ){
    // sign extend
    CompInst.imm |= 0b11111111111111111111111111100000;
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCSSInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs2     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm     = ((Inst & 0b1111110000000) >> 7);

  if( CompInst.funct3 == 0b101 ){
    // c.fsdsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1110000000000) >> 7);    // [5:3]
    CompInst.imm |= ((Inst & 0b1110000000) >> 1);       // [8:6]
  }else if( CompInst.funct3 == 0b110 ){
    // c.swsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111000000000) >> 7);    // [5:2]
    CompInst.imm |= ((Inst & 0b110000000) >> 1);        // [7:6]
  }else if( CompInst.funct3 == 0b111 ){
    CompInst.imm = 0;
    if( feature->GetXlen() == 64 ){
      // c.sdsp
      CompInst.imm =  ((Inst & 0b1110000000000) >> 7);    // [5:3]
      CompInst.imm |= ((Inst & 0b1110000000) >> 1);       // [8:6]
    }else{
      // c.fswsp
      CompInst.imm =  ((Inst & 0b1111000000000) >> 7);    // [5:2]
      CompInst.imm |= ((Inst & 0b110000000) >> 1);        // [7:6]
    }
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCIWInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = ((Inst & 0b11100) >> 2);
  CompInst.imm     = ((Inst & 0b1111111100000) >> 5);

  //swizzle: nzuimm[5:4|9:6|2|3]
  std::bitset<32> imm(CompInst.imm);
  std::bitset<32> tmp(0);
  tmp[0] = imm[1];
  tmp[1] = imm[0];
  tmp[2] = imm[6];
  tmp[3] = imm[7];
  tmp[4] = imm[2];
  tmp[5] = imm[3];
  tmp[6] = imm[4];
  tmp[7] = imm[5];

  CompInst.imm = tmp.to_ulong();

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCLInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = ((Inst & 0b11100) >> 2);
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);

  if( CompInst.funct3 == 0b001 ){
    // c.fld
    CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b010 ){
    // c.lw
    CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
    CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b011 ){
    if( feature->GetXlen() == 64 ){
      // c.ld
      CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }else{
      // c.flw
      CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
      CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }
  }else if( CompInst.funct3 == 0b101 ){
    // c.fsd
    CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b110 ){
    // c.sw
    CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
    CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b111 ){
    if( feature->GetXlen() == 64 ){
      // c.sd
      CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }else{
      // c.fsw
      CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
      CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }
  }


  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCSInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs2     = ((Inst & 0b011100) >> 2);
  CompInst.rs1     = ((Inst & 0b01110000000) >> 7);

  // The immd is pre-scaled in this instruction format
  if(CompInst.funct3 == 0b110){
    //c.sw
    CompInst.imm     = ((Inst & 0b0100000) << 1);         //offset[6]
    CompInst.imm    |= ((Inst & 0b01110000000000) >> 6);   //offset[5:3]
    CompInst.imm    |= ((Inst & 0b01000000) >> 4);          //offset[2]
  }else{
    if(feature->GetXlen() == 32){
      //c.fsw
      CompInst.imm     = ((Inst & 0b00100000) << 1);         //imm[6]
      CompInst.imm     = ((Inst & 0b01000000) << 4);         //imm[2]
      CompInst.imm    |= ((Inst & 0b01110000000000) >> 7); //imm[5:3]
    }else{
      //c.sd
      CompInst.imm     = ((Inst & 0b01100000) << 1);         //imm[7:6]
      CompInst.imm    |= ((Inst & 0b01110000000000) >> 7); //imm[5:3]
    }
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCAInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct2  = InstTable[Entry].funct2;
  CompInst.funct6  = InstTable[Entry].funct6;

  // registers
  CompInst.rs2     = ((Inst & 0b11100) >> 2);
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);
  CompInst.rd      = ((Inst & 0b1110000000) >> 7);

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCBInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);
  CompInst.offset  = ((Inst & 0b1111100) >> 2);
  CompInst.offset |= ((Inst & 0b1110000000000) >> 5);

  //swizzle: offset[8|4:3]  offset[7:6|2:1|5]
  std::bitset<16> tmp(0);
  // handle c.beqz/c.bnez offset
  if( (CompInst.opcode == 0b01) && (CompInst.funct3 >= 0b110) ){
    std::bitset<16> o(CompInst.offset);
    tmp[0] = o[1];
    tmp[1] = o[2];
    tmp[2] = o[5];
    tmp[3] = o[6];
    tmp[4] = o[0];
    tmp[5] = o[3];
    tmp[6] = o[4];
    tmp[7] = o[7];
  } else if( (CompInst.opcode == 0b01) && (CompInst.funct3 == 0b100)) { 
    //We have a shift or a andi 
    CompInst.rd = CompInst.rs1;
  }

  CompInst.offset = ((uint16_t)tmp.to_ulong()) << 1; // scale to corrrect position to be consistent with other compressed ops
  CompInst.imm = ((Inst & 0b01111100) >> 2);
  CompInst.imm |= ((Inst & 0b01000000000000) >> 7);


/*  // handle c.beqz/c.bnez offset
  if( (CompInst.opcode = 0b01) && (CompInst.funct3 >= 0b110) ){
    CompInst.offset = 0;  // reset it
    CompInst.offset = ((Inst & 0b11000) >> 2);          // [2:1]
    CompInst.offset |= ((Inst & 0b110000000000) >> 7);  // [4:3]
    CompInst.offset |= ((Inst & 0b100) << 3);           // [5]
    CompInst.offset |= ((Inst & 0b1100000) << 1);       // [7:6]
    CompInst.offset |= ((Inst & 0b1000000000000) >> 4); // [8]

    if( (CompInst.offset & 0b100000000) > 0 ){
      // sign extend
      CompInst.offset |= 0b11111111100000000;
    }
  }*/

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCJInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  uint16_t offset = ((Inst & 0b1111111111100) >> 2);

  //swizzle bits offset[11|4|9:8|10|6|7|3:1|5]
  std::bitset<16> offsetBits(offset);
  std::bitset<16> target;
  target.reset();
  target[0] = offsetBits[1];
  target[1] = offsetBits[2];
  target[2] = offsetBits[3];
  target[3] = offsetBits[9];
  target[4] = offsetBits[0];
  target[5] = offsetBits[5];
  target[6] = offsetBits[4];
  target[7] = offsetBits[7];
  target[8] = offsetBits[8];
  target[9] = offsetBits[6];
  target[10] = offsetBits[10];
  CompInst.jumpTarget = ((u_int16_t)target.to_ulong()) << 1;
  //CompInst.jumpTarget = ((u_int16_t)target.to_ulong());

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCompressed(uint32_t Inst){
  uint16_t TmpInst = (uint16_t)(Inst&0b1111111111111111);
  uint8_t opc     = 0;
  uint8_t funct2  = 0;
  uint8_t funct3  = 0;
  uint8_t funct4  = 0;
  uint8_t funct6  = 0;
  uint8_t l3      = 0;
  uint32_t Enc    = 0x00ul;
  uint64_t PC     = GetPC();
  RevInst TInst;

  if( !feature->HasCompressed() ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Compressed instructions not enabled!\n",
                  PC);

  }

  ResetInst(&TInst);

  // decode the opcode
  opc = (TmpInst & 0b11);
  l3  = ((TmpInst & 0b1110000000000000)>>13);
  if( opc == 0b00 ){
    // quadrant 0
    funct3 = l3;
  }else if( opc == 0b01){
    // quadrant 1
    if( l3 <= 0b011 ){
      // upper portion: misc
      funct3 = l3;
    }else if( (l3 > 0b011) && (l3 < 0b101) ){
      // middle portion: arithmetics
      uint8_t opSelect = ((TmpInst & 0b110000000000) >> 10);
      if( opSelect == 0b11 ){
        funct6 = ((TmpInst & 0b1111110000000000) >> 10);
        funct2 = ((TmpInst & 0b01100000) >> 5 );
      }else{
        funct3 = l3;
        funct2 = opSelect;
      }
    }else{
      // lower power: jumps/branches
      funct3 = l3;
    }
  }else if( opc == 0b10){
    // quadrant 2
    if( l3 == 0b000 ){
      // slli{64}
      funct3 = l3;
    }else if( l3 < 0b100 ){
      // float/double/quad load
      funct3 = l3;
    }else if( l3 == 0b100 ){
      // jump, mv, break, add
      funct4 = ((TmpInst & 0b1111000000000000) >> 12);
    }else{
      // float/double/quad store
      funct3 = l3;
    }
  }

  Enc |= (uint32_t)(opc);
  Enc |= (uint32_t)(funct2 << 2);
  Enc |= (uint32_t)(funct3 << 4);
  Enc |= (uint32_t)(funct4 << 8);
  Enc |= (uint32_t)(funct6 << 12);

  bool isCoProcInst = false;
  std::map<uint32_t,unsigned>::iterator it = CEncToEntry.find(Enc);
  if( it == CEncToEntry.end() ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){ 
        //Create NOP - ADDI x0, x0 0
        uint8_t caddi_op= 0b01;
        Inst = 0;
        Enc = 0;
        Enc |= caddi_op;
        it = CEncToEntry.find(Enc);
      }else{
        output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%d\n opc=%x; funct2=%x, funct3=%x, funct4=%x, funct6=%x\n",
                  PC,
                  Enc, opc, funct2, funct3, funct4, funct6 );
      }
  }

  unsigned Entry = it->second;
  if( Entry > (InstTable.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 "\
                  Opcode = %x Funct2 = %x Funct3 = %x Funct4 = %x Funct6 = %x Enc = %x \n", \
                  PC, opc, funct2, funct3, funct4, funct6, Enc );

  }

  RegFile->Entry = Entry;

  RegFile->trigger = false;

  switch( InstTable[Entry].format ){
  case RVCTypeCR:
    return DecodeCRInst(TmpInst,Entry);
    break;
  case RVCTypeCI:
    return DecodeCIInst(TmpInst,Entry);
    break;
  case RVCTypeCSS:
    return DecodeCSSInst(TmpInst,Entry);
    break;
  case RVCTypeCIW:
    return DecodeCIWInst(TmpInst,Entry);
    break;
  case RVCTypeCL:
    return DecodeCLInst(TmpInst,Entry);
    break;
  case RVCTypeCS:
    return DecodeCSInst(TmpInst,Entry);
    break;
  case RVCTypeCA:
    return DecodeCAInst(TmpInst,Entry);
    break;
  case RVCTypeCB:
    return DecodeCBInst(TmpInst,Entry);
    break;
  case RVCTypeCJ:
    return DecodeCJInst(TmpInst,Entry);
    break;
  default:
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction format at PC=%" PRIx64 ".", PC );
    break;
  }

  // we should never arrive here
  // we return a null instruction in order to forego a compiler warning
  return TInst;
}

RevInst RevProc::DecodeRInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = InstTable[Entry].funct7;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  if( (InstTable[Entry].imm == FImm) && (InstTable[Entry].rs2Class == RegUNKNOWN)){
    DInst.imm  = DECODE_IMM12(Inst) & 0b011111; 
  }else{
    DInst.imm     = 0x0;
  }

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode the atomic RL/AQ fields
  if( DInst.opcode == 0b0101111 ){
    DInst.rl = DECODE_RL(Inst);
    DInst.aq = DECODE_AQ(Inst);
  }

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }

  DInst.compressed = false;

  return DInst;
}

RevInst RevProc::DecodeIInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }

  // imm
  DInst.imm     = DECODE_IMM12(Inst);

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }
  DInst.compressed = false;

  return DInst;
}

RevInst RevProc::DecodeSInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  DInst.imm     = (DECODE_RD(Inst) | (DECODE_FUNCT7(Inst)<<5));

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeUInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = 0x0;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }

  // imm
  DInst.imm     = DECODE_IMM20(Inst);

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeBInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  DInst.imm =   (uint32_t)((Inst << 4)&0b100000000000)|   // [11]
                (uint32_t)((Inst & 0b111100000000)>>7)|   // [4:1]
                (uint32_t)((Inst >> 20)&0b11111100000)|   // [10:5]
                (uint32_t)((Inst >> 19)&0b1000000000000);  // [12]

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeJInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }

  // immA
  DInst.imm     = 0x00;
  DInst.imm     = ( (uint32_t)((Inst >> 20) & 0b11111111110) |            // imm[10:1]
                    (uint32_t)(Inst & 0b11111111000000000000) |           // imm[19:12]
                    (uint32_t)((Inst >> 9) & 0b100000000000) |            // imm[11]
                    (uint32_t)((Inst >> 11) & 0b100000000000000000000) ); // imm[20]

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeR4Inst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = DECODE_FUNCT2(Inst);
  DInst.funct7  = InstTable[Entry].funct7;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }
  if( InstTable[Entry].rs3Class != RegUNKNOWN ){
    DInst.rs3  = DECODE_RS3(Inst);
  }

  // imm
  DInst.imm     = 0x0;

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

bool RevProc::DebugReadReg(unsigned Idx, uint64_t *Value){
  RevRegFile* regFile = GetRegFile(HartToExec);
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    *Value = regFile->RV32[Idx];
    return true;
  }else{
    *Value = regFile->RV64[Idx];
    return true;
  }
}

bool RevProc::DebugWriteReg(unsigned Idx, uint64_t Value){
  RevRegFile* regFile = GetRegFile(HartToExec);
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    regFile->RV32[Idx] = (uint32_t)(Value&0xFFFFFFFF);
    return true;
  }else{
    regFile->RV64[Idx] = Value;
    return true;
  }
}

uint64_t RevProc::GetPC(){
  if( feature->GetXlen() == 32 ){
    return (uint64_t)(RegFile->RV32_PC);
  }else{
    return RegFile->RV64_PC;
  }
}

void RevProc::SetPC(uint64_t PC){
  if( feature->GetXlen() == 32 ){
    RegFile->RV32_PC = (uint32_t)(PC);
  }else{
    RegFile->RV64_PC = PC;
  }
}

bool RevProc::PrefetchInst(){
  uint64_t PC   = 0x00ull;
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile->RV32_PC);
  }else{
    PC = RegFile->RV64_PC;
  }

  // These are addresses that we can't decode
  // Return false back to the main program loop
  if( (PC == 0x00ull) ||
      (PC == _PAN_FWARE_JUMP_) ){
    return false;
  }

  return sfetch->IsAvail(PC);
}

RevInst RevProc::DecodeInst(){
  uint32_t Enc  = 0x00ul;
  uint32_t Inst = 0x00ul;
  uint64_t PC   = 0x00ull;
  bool Fetched  = false;
  RevInst TInst;

  ResetInst(&TInst);

  // Stage 1: Retrieve the instruction
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile->RV32_PC);
  }else{
    PC = RegFile->RV64_PC;
  }

  if( !sfetch->InstFetch(PC, Fetched, Inst) ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve prefetched instruction at PC=0x%" PRIx64 "\n",
                  PC);
  }

  if(0 != Inst){
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Thread %d; PC:InstPayload = 0x%" PRIx64 ":0x%" PRIx32 "\n",
                    id, HartToDecode, PC, Inst);
  }else{
    output->fatal(CALL_INFO, -1,
                  "Error: Core %d failed to decode instruction at PC=0x%" PRIx64 "; Inst=%d\n",
                  id,
                  PC,
                  Inst );
  }

  // Stage 1a: handle the crack fault injection
  if( CrackFault ){
    srand(time(NULL));
    uint64_t rval = rand() % (2^(fault_width));
    Inst |= rval;

    // clear the fault
    CrackFault = false;
  }

  // Stage 2: Retrieve the opcode
  const uint32_t Opcode = (uint32_t)(Inst&0b1111111);

  // If we find a compressed instruction, then take
  // the compressed decode path
  if( (Opcode&0b11) != 0b11 ){
    // this is a compressed instruction
    return DecodeCompressed(Inst);
  }

  // Stage 3: Determine if we have a funct3 field
  uint32_t Funct3 = 0x00ul;
  const uint32_t inst42 = ((Opcode&0b11100) >> 2);
  const uint32_t inst65 = ((Opcode&0b1100000) >> 5);

  if( (inst42 == 0b011) && (inst65 == 0b11) ){
    // JAL
    Funct3 = 0x00ul;
  }else if( (inst42 == 0b101) && (inst65 == 0b00) ){
    // AUIPC
    Funct3 = 0x00ul;
  }else if( (inst42 == 0b101) && (inst65 == 0b01) ){
    // LUI
    Funct3 = 0x00ul;
  }else{
    // Retrieve the field
    Funct3 = ((Inst&0b111000000000000) >> 12 );
  }

  // Stage 4: Determine if we have a funct7 field (R-Type and some specific I-Type)
  uint32_t Funct7 = 0x00ul;
  if( inst65 == 0b01 ) {
    if( (inst42 == 0b011) || (inst42 == 0b100) || (inst42 == 0b110) ){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
      //Atomics have a smaller funct7 field - trim out the aq and rl fields
      if(Opcode == 0b0101111){
          Funct7 = (Funct7 &0b01111100) >> 2;
      }
    }
  }else if((inst65== 0b10) && (inst42 == 0b100)){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
  }else if((inst65 == 0b00) && (inst42 == 0b110) && (Funct3 != 0)){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
  }else if((inst65 == 0b00) && (inst42 == 0b100) && (Funct3 == 0b101)){
      // Special I-Type encoding for SRAI - also, Funct7 is only 6 bits in this case
      Funct7 = ((Inst >> 26) & 0b1111111);
  }

  uint32_t fcvtOp = 0;
  //Special encodings for FCVT instructions
  if( (0b1010011 == Opcode) && ((0b1100000 == Funct7) || (0b1101000 == Funct7)) ){
      fcvtOp =  DECODE_RS2(Inst);
  }

  // Stage 5: Determine if we have an imm12 field
  uint32_t Imm12 = 0x00ul;
  if( (inst42 == 0b100) && (inst65 == 0b11)  && (Funct3 == 0)){
    Imm12 = ((Inst >> 19) & 0b111111111111);
  }

  // Stage 6: Compress the encoding
  Enc |= Opcode;
  Enc |= (Funct3<<8);
  Enc |= (Funct7<<11);
  Enc |= (Imm12<<18);
  Enc |= (fcvtOp<<30);

  // Stage 7: Look up the value in the table
  bool isCoProcInst = false;
  std::map<uint32_t,unsigned>::iterator it;
  it = EncToEntry.find(Enc);
   if( it == EncToEntry.end() && ((Funct3 == 7) || (Funct3==1)) && (inst65 == 0b10)){
    //This is kind of a hack, but we may not have found the instruction becasue
    //  Funct3 is overloaded with rounding mode, so if this is a RV32F or RV64F
    //  set Funct3 to zero and check again
    Enc = 0;
    Enc |= Opcode;
    Enc |= (Funct7<<11);
    Enc |= (Imm12<<18);
    Enc |= (fcvtOp<<30);
    it = EncToEntry.find(Enc);
    if( it == EncToEntry.end() ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){ 
        //Create NOP - ADDI x0, x0 0
        uint32_t addi_op= 0b0010011;
        Inst = 0;
        Enc = 0;
        Enc |= addi_op;
        it = EncToEntry.find(Enc);
      }else{
        // failed to decode the instruction
        output->fatal(CALL_INFO, -1,
                    "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%d\n",
                    PC,
                    Enc );
      }
    }

  }

  unsigned Entry = it->second;

  if( Entry > (InstTable.size()-1) ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){ 
        //Create NOP - ADDI x0, x0 0
        uint32_t addi_op= 0b0010011;
        Inst = 0;
        Enc = 0;
        Enc |= addi_op;
        it = EncToEntry.find(Enc);
        Entry = it->second;
      } else {
        output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 " \
                  Opcode = %x Funct3 = %x Funct7 = %x Imm12 = %x Enc = %x \n", \
                  PC, Opcode, Funct3, Funct7, Imm12, Enc );
      }

  }

  RegFile->Entry = Entry;

  RegFile->trigger = false;


  // Stage 8: Do a full deocode using the target format
  switch( InstTable[Entry].format ){
  case RVTypeR:
    return DecodeRInst(Inst,Entry);
    break;
  case RVTypeI:
    return DecodeIInst(Inst,Entry);
    break;
  case RVTypeS:
    return DecodeSInst(Inst,Entry);
    break;
  case RVTypeU:
    return DecodeUInst(Inst,Entry);
    break;
  case RVTypeB:
    return DecodeBInst(Inst,Entry);
    break;
  case RVTypeJ:
    return DecodeJInst(Inst,Entry);
    break;
  case RVTypeR4:
    return DecodeR4Inst(Inst,Entry);
    break;
  default:
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction format at PC=%" PRIx64 ".", PC );
    break;
  }

  // we should never arrive here
  // we return a null instruction to forego a compiler warning
  return TInst;
}

void RevProc::ResetInst(RevInst *I){
  I->opcode     = 0;
  I->funct2     = 0;
  I->funct3     = 0;
  I->funct4     = 0;
  I->funct6     = 0;
  I->funct7     = 0;
  I->rd         = ~0;  // Set registers to value that is clearly invalid
  I->rs1        = ~0;
  I->rs2        = ~0;
  I->rs3        = ~0;
  I->imm        = 0;
  I->fmt        = 0;
  I->rm         = 0;
  I->aq         = 0;
  I->rl         = 0;
  I->offset     = 0;
  I->jumpTarget = 0;
  I->instSize   = 0;
  I->compressed = false;
  I->hazard     = nullptr;
}

void RevProc::HandleRegFault(unsigned width){
  // build the permissible set of registers available to fault
  unsigned LWidth = 0;
  std::vector<std::pair<std::string,void*>> RRegs;

  RevRegFile* regFile = GetRegFile(HartToExec); 

  if( feature->GetXlen() == 32 ){
    if( width > feature->GetXlen() ){
      LWidth = feature->GetXlen();
    }else{
      LWidth = width;
    }

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->RV32[i])));
    }
  }else{
    // rv64
    LWidth = width;

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->RV64[i])));
    }
  }

  if( feature->IsModeEnabled(RV_F) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->SPF[i])));
    }
  }else if( feature->IsModeEnabled(RV_D) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->DPF[i])));
    }
  }

  // build the payload
  srand(time(NULL));
  uint64_t rval = rand() % (2^(LWidth));

  // select a register
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(0, RRegs.size()-1); // define the range
  unsigned RegIdx = distr(gen);

  std::string RegName = RRegs[RegIdx].first;
  if( feature->GetXlen() == 32 ){
    uint32_t *ptr = (uint32_t *)(RRegs[RegIdx].second);
    *ptr |= (uint32_t)(rval);
  }else{
    uint64_t *ptr = (uint64_t *)(RRegs[RegIdx].second);
    *ptr |= rval;
  }

  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:REG: Register fault of %d bits into register %s\n",
                  LWidth,RegName.c_str());
}

void RevProc::HandleCrackFault(unsigned width){
  CrackFault = true;
  fault_width = width;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:CRACK: Crack+Decode fault injected into next decode cycle\n");
}

void RevProc::HandleALUFault(unsigned width){
  ALUFault = true;
  fault_width = true;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:ALU: ALU fault injected into next retire cycle\n");
}

bool RevProc::DependencyCheck(uint16_t HartID, RevInst* I){

  bool depFound = false;
  bool isFloat = IsFloat(I->entry);

  RevRegFile* regFile = GetRegFile(HartID);

  // check the load hazard bit
  if( I->hazard != nullptr ){
    bool *Hazard = I->hazard;
    if( *Hazard ){
      return true;
    }
  }

  if(feature->IsRV32()){
    if(isFloat){
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs3] || depFound : depFound;
    }else{
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs3] || depFound : depFound;
    }
  }else {
    if(isFloat){
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs3] || depFound : depFound;
    }else{
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs3] || depFound : depFound;
    }
  }
  return depFound;
}

void RevProc::DependencySet(uint16_t HartID, RevInst* Inst){
      RevRegFile* regFile = GetRegFile(HartID);
      if(Inst->rd != 0 && Inst->rd < _REV_NUM_REGS_){
        bool isFloat = IsFloat(Inst->entry);
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[Inst->rd] = true;
          }else{
            regFile->RV32_Scoreboard[Inst->rd] = true;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[Inst->rd] = true;
          }else{
            regFile->RV64_Scoreboard[Inst->rd] = true;
          }
      }
    }
}

void RevProc::DependencySet(uint16_t HartID, uint16_t RegNum, bool isFloat){
      RevRegFile* regFile = GetRegFile(HartID);
      if(RegNum != 0 && RegNum < _REV_NUM_REGS_){
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[RegNum] = true;
          }else{
            regFile->RV32_Scoreboard[RegNum] = true;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[RegNum] = true;
          }else{
            regFile->RV64_Scoreboard[RegNum] = true;
          }
      }
    }
}

void RevProc::DependencyClear(uint16_t HartID, RevInst* Inst){
    RevRegFile* regFile = GetRegFile(HartID);
    if(Inst->rd < _REV_NUM_REGS_){
        bool isFloat = IsFloat(Inst->entry);
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[Inst->rd] = false;
          }else{
            regFile->RV32_Scoreboard[Inst->rd] = false;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[Inst->rd] = false;
          }else{
            regFile->RV64_Scoreboard[Inst->rd] = false;
          }
      }
    }
}

void RevProc::DependencyClear(uint16_t HartID, uint16_t RegNum, bool isFloat){
      RevRegFile* regFile = GetRegFile(HartID);
      if(RegNum != 0 && RegNum < _REV_NUM_REGS_){
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[RegNum] = false;
          }else{
            regFile->RV32_Scoreboard[RegNum] = false;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[RegNum] = false;
          }else{
            regFile->RV64_Scoreboard[RegNum] = false;
          }
      }
    }
}

uint16_t RevProc::GetHartID(){
  if(HART_CTS.none()) { return HartToDecode;};

  uint16_t NextHartID = HartToDecode;
  if(HART_CTS[HartToDecode]){
    NextHartID = HartToDecode;
  }else{
    for(int HartID = 0; HartID < _REV_HART_COUNT_; HartID++){
      NextHartID++;
      if(NextHartID >= _REV_HART_COUNT_){
        NextHartID = 0;
      }
      if(HART_CTS[NextHartID]){ break; };
    }
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Hart switch from %d to %d \n",
                    id, HartToDecode, NextHartID);
  }
  return NextHartID;
}

bool *RevProc::createLoadHazard(){
  bool *LH = new bool;
  *LH = false;
  LoadHazards.push_back(LH);
  return LH;
}

void RevProc::destroyLoadHazard(bool *LH){
  if( LH != nullptr ){
    LoadHazards.remove(LH);
  }
}

bool RevProc::ClockTick( SST::Cycle_t currentCycle ){
  bool rtn = false;
  Stats.totalCycles++;

  if( AssignedThreads.size() > _REV_HART_COUNT_ ){
    output->fatal(CALL_INFO, 99, "Proc has become oversubscribed\n");
  } else if( AssignedThreads.size() ){
    std::cout << "Proc " << id << " has " << std::dec << AssignedThreads.size() << " assigned threads" << std::endl;
    for(int i = 0; i < _REV_HART_COUNT_; i++){
      if( AssignedThreads.size() > i ){
        std::cout << "Hart " << i << " has ThreadID: " << AssignedThreads.at(i)->GetThreadID() << std::endl;
      }
    }
  }
  else {
    return false;
  }


#ifdef _REV_DEBUG_
  if((currentCycle % 100000000) == 0){
    std::cout << "Current Cycle: " << currentCycle <<  " PC: "
              << std::hex << ExecPC << std::dec << std::endl;
  }
#endif

  // -- MAIN PROGRAM LOOP --
  //
  // If the clock is down to zero, then fetch the next instruction
  // else if the the instruction has not yet been triggered, execute it
  // else, wait until the counter is decremented to zero to retire the instruction
  //
  //
  // if( PendingCtxSwitch ){
    /*
     * There was a ctx switch event triggered... Either:
     * - a call to fork/clone
     * - Child process finished executing
     * - TODO: ThreadManager decided to switch threads
     */
    // if( Pipeline.empty() ) {
      // if( !ChangeActiveThreadID(NextThreadID) ){
      //   output->fatal(CALL_INFO, -1,
      //                 "Core %d ; Hart %u; ThreadID %d Failed to change active ThreadID to %u\n",
      //                 id, HartToDecode, GetActiveThreadID(), NextThreadID);
      // } else {
      //   RegFile->trigger = 0;
      //   RegFile->cost = 0;
      //   ExecPC = GetPC();
      //   PendingCtxSwitch = false;
      //   NextThreadID = 0;
      // }
    // }
  // }

  if( AssignedThreads.size() ){
    for (int HartID = 0; HartID < _REV_HART_COUNT_; HartID++){
      // Only execute the hart if it is assigned a thread
      if( AssignedThreads.size() > HartID ){
        HART_CTS[HartID] = AssignedThreads.at(HartID)->GetRegFile()->cost == 0;
      }
    }
    // std::cout << *AssignedThreads.front() << std::endl;
  } else {
    
  }

  if( HART_CTS.any() && (!Halted)) {
    // fetch the next instruction
    ResetInst(&Inst);

    //Determine the active Hart
    HartToDecode = GetHartID();
    RegFile = AssignedThreads.at(HartToDecode)->GetRegFile();

    if( !PrefetchInst() ){
      Stalled = true;
      Stats.cyclesStalled++;
    }else{
      Stalled = false;
    }

    // If the next instruction is our special bounce address
    // DO NOT decode it.  It will decode to a bogus instruction.
    // We do not want to retire this instruction until we're ready
    if( (GetPC() != _PAN_FWARE_JUMP_) && (!Stalled) ){
      Inst = DecodeInst();
      Inst.entry = RegFile->Entry;
    }

    Inst.hazard = nullptr;

    // Now that we have decoded the instruction, check for pipeline hazards
    if(Stalled || DependencyCheck(HartToDecode, &Inst)) {
      RegFile->cost = 0; // We failed dependency check, so set cost to 0 - this will
      Stats.cyclesIdle_Pipeline++;        // prevent the instruction from advancing to the next stage
      HART_CTE[HartToDecode] = false;
      HartToExec = _REV_INVALID_HART_ID_;
    }else {
      Stats.cyclesBusy++;
      HART_CTE[HartToDecode] = true;
      HartToExec = HartToDecode;
    }
    Inst.cost = RegFile->cost;
    Inst.entry = RegFile->Entry;
    rtn = true;
    ExecPC = GetPC();
  }

  if( ( (HartToExec != _REV_INVALID_HART_ID_) && !RegFile->trigger) && !Halted && HART_CTE[HartToExec]){
    // trigger the next instruction
    // HartToExec = HartToDecode;
    RegFile->trigger = true;

    // pull the PC
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Thread %d; Executing PC= 0x%" PRIx64 "\n",
                    id, HartToExec, ExecPC);

    // attempt to execute the instruction as long as it is NOT
    // the firmware jump PC
    if( ExecPC != _PAN_FWARE_JUMP_ ){

      // Find the instruction extension
      std::map<unsigned,std::pair<unsigned,unsigned>>::iterator it;
      it = EntryToExt.find(RegFile->Entry);
      if( it == EntryToExt.end() ){
        // failed to find the extension
        output->fatal(CALL_INFO, -1,
                    "Error: failed to find the instruction extension at PC=%" PRIx64 ".", ExecPC );
      }

      // found the instruction extension
      std::pair<unsigned,unsigned> EToE = it->second;
      RevExt *Ext = Extensions[EToE.first];

      // Update RegFile (in case of prior context switch)
      Ext->SetRegFile(RegFile);

      // -- BEGIN new pipelining implementation
      // if( !PendingCtxSwitch ){
        bool *LH = createLoadHazard();
        Pipeline.push_back(std::make_pair(HartToExec,Inst));
        Pipeline.back().second.hazard = LH;
      // }

      if( (Ext->GetName() == "RV32F") ||
          (Ext->GetName() == "RV32D") ||
          (Ext->GetName() == "RV64F") ||
          (Ext->GetName() == "RV64D") ){
        Stats.floatsExec++;
      }

      // set the hazarding
      DependencySet(HartToExec, &(Pipeline.back().second));
      // -- END new pipelining implementation

      // execute the instruction
      if( !Ext->Execute(EToE.second, Pipeline.back().second, HartToExec) ){
        output->fatal(CALL_INFO, -1,
                    "Error: failed to execute instruction at PC=%" PRIx64 ".", ExecPC );
      }

//      #define __REV_DEEP_TRACE__
      #ifdef __REV_DEEP_TRACE__
      if(feature->IsRV32()){
        std::cout << "RDT: Executed PC = " << std::hex << ExecPC
                  << " Inst: " << std::setw(23)
                  << InstTable[Inst.entry].mnemonic
                  << " r" << std::dec << (uint32_t)Inst.rd  << "= "
                  << std::hex << RegFile->RV32[Inst.rd]
                  << " r" << std::dec << (uint32_t)Inst.rs1 << "= "
                  << std::hex << RegFile->RV32[Inst.rs1]
                  << " r" << std::dec << (uint32_t)Inst.rs2 << "= "
                  << std::hex << RegFile->RV32[Inst.rs2]
                  << " imm = " << std::hex << Inst.imm
                  << std::endl;

      }else{
        std::cout << "RDT: Executed PC = " << std::hex << ExecPC \
                  << " Inst: " << std::setw(23)
                  << InstTable[Inst.entry].mnemonic
                  << " r" << std::dec << (uint32_t)Inst.rd  << "= "
                  << std::hex << RegFile->RV64[Inst.rd]
                  << " r" << std::dec << (uint32_t)Inst.rs1 << "= "
                  << std::hex << RegFile->RV64[Inst.rs1]
                  << " r" << std::dec << (uint32_t)Inst.rs2 << "= "
                  << std::hex << RegFile->RV64[Inst.rs2]
                  << " imm = " << std::hex << Inst.imm
                  << std::endl;
        std::cout << "RDT: Address of RD = 0x" << std::hex
                  << (uint64_t *)(&RegFile->RV64[Inst.rd])
                  << std::dec << std::endl;
      }
      #endif

      /*
       * Exception Handling
       * - Currently this is only for ecall
      */
      if( (RegFile->RV64_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ||
          (RegFile->RV32_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ){
        // Ecall found
        output->verbose(CALL_INFO, 6, 0,
                  "Core %d; HartID %d; ThreadID %d - Exception Raised: ECALL with code = %lu\n", 
                  id, HartToExec, GetActiveThreadID(), RegFile->RV64[17]);
        #ifdef _REV_DEBUG_
        std::cout << "Hart "<< HartToExec << " found ecall with code: "
                  << code << std::endl;
        #endif

        /* Execute system call on this RevProc */
        ExecEcall(Pipeline.back().second); //ExecEcall will also set the exception cause registers

        #ifdef _REV_DEBUG_
        std::cout << "Hart "<< HartToExec << " returned from ecall with code: "
                  << rc << std::endl;
        #endif

        // } else {
        //   ExecEcall();
        #ifdef _REV_DEBUG_
        std::cout << "Hart "<< HartToExec << " found ecall with code: "
                  << code << std::endl;
        #endif

        #ifdef _REV_DEBUG_
        std::cout << "Hart "<< HartToExec << " returned from ecall with code: "
                  << rc << std::endl;
        #endif
        // }
      }

      // inject the ALU fault
      if( ALUFault ){
        // inject ALU fault
        RevExt *Ext = Extensions[EToE.first];
        if( (Ext->GetName() == "RV32F") ||
            (Ext->GetName() == "RV32D") ){
          // write an rv32 float rd
          uint32_t rval = rand() % (2^(fault_width));
          uint32_t tmp = (uint32_t)(RegFile->SPF[Inst.rd]);
          tmp |= rval;
          RegFile->SPF[Inst.rd] = (float)(tmp);
        }else if( (Ext->GetName() == "RV64F") ||
                  (Ext->GetName() == "RV64D") ){
          // write an rv64 float rd
          uint64_t rval = rand() % (2^(fault_width));
          uint64_t tmp = (uint64_t)(RegFile->DPF[Inst.rd]);
          tmp |= rval;
          RegFile->DPF[Inst.rd] = (double)(tmp);
        }else if( feature->GetXlen() == 32 ){
          // write an rv32 gpr rd
          uint32_t rval = rand() % (2^(fault_width));
          RegFile->RV32[Inst.rd] |= rval;
        }else{
          // write an rv64 gpr rd
          uint64_t rval = rand() % (2^(fault_width));
          RegFile->RV64[Inst.rd] |= rval;
        }

        // clear the fault
        ALUFault = false;
      }
    }

    // if this is a singlestep, clear the singlestep and halt
    if( SingleStep ){
      SingleStep = false;
      Halted = true;
    }

    rtn = true;
  }else{
    // wait until the counter has been decremented
    // note that this will continue to occur until the counter is drained
    // and the HART is halted
    output->verbose(CALL_INFO, 9, 0,
                    "Core %d ; No available thread to exec PC= 0x%" PRIx64 "\n",
                    id, ExecPC);
    rtn = true;
    Stats.cyclesIdle_Total++;
    if(HART_CTE.any()){
      Stats.cyclesIdle_MemoryFetch++;
    }
  }

  // walk the pipeline hazards.  if a load hazard is set, increase the cost to > 0
  for( auto i : Pipeline ){
    if( *(i.second.hazard) ){
      i.second.cost++;
    }
  }

  // Check for pipeline hazards
  if(!Pipeline.empty() &&
     (Pipeline.front().second.cost > 0)){
    Pipeline.front().second.cost--;
    if((Pipeline.front().second.cost == 0) &&
       (!*(Pipeline.front().second.hazard))){
      // Ready to retire this instruction
      uint16_t tID = Pipeline.front().first;
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; ThreadID %d; Retiring PC= 0x%" PRIx64 "\n",
                      id, tID, ExecPC);
      Retired++;
      DependencyClear(tID, &(Pipeline.front().second));
      destroyLoadHazard(Pipeline.front().second.hazard);
      Pipeline.erase(Pipeline.begin());
      GetRegFile(tID)->cost = 0;
    }else{
      // could not retire the instruction, bump the cost
      Pipeline.front().second.cost++;
    }
  }

  // Check for completion states and new tasks
  if( (GetPC() == _PAN_FWARE_JUMP_) || (GetPC() == 0x00ull) ){
    // look for more work on the execution queue
    // if no work is found, don't update the PC
    // just wait and spin
    bool done = true;
    if( GetPC() == _PAN_FWARE_JUMP_ ){
      if( PExec != nullptr){
        uint64_t Addr = 0x00ull;
        unsigned Idx = 0;
        PanExec::PanStatus Status = PExec->GetNextEntry(&Addr,&Idx);
        switch( Status ){
        case PanExec::QExec:
          output->verbose(CALL_INFO, 5, 0,
                      "Core %d ; PAN Exec Jumping to PC= 0x%" PRIx64 "\n",
                      id, Addr);
          SetPC(Addr);
          done = false;
          break;
        case PanExec::QNull:
          // no work to do; spin on the firmware jump PC
          output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; No PAN work to do; Jumping to PC= 0x%" PRIx64 "\n",
                      id, ExecPC);
          done = false;
          SetPC(_PAN_FWARE_JUMP_);
          break;
        case PanExec::QValid:
        case PanExec::QError:
          done = true;
        default:
          break;
        }
      }
    }else if( GetPC() == 0x00ull ) {
      // PAN execution contexts not enabled, this is our last PC
      done = true;
    }

    // determine if we have any outstanding memory requests
    if( mem->outstandingRqsts() ){
      done = false;
    }

    // if( HartToExec != _REV_INVALID_HART_ID_ ){
    //   AssignedThreads.at(HartToExec)->SetState(ThreadState::Done);
      // if( ActiveThreadIDs.size() > HartToExec ) {
        // uint32_t CurrThreadID = ActiveThreadIDs.at(HartToExec);
        // uint32_t ParentThreadID = ThreadTable.at(ActiveThreadIDs.at(HartToExec))->GetParentThreadID();
        // output->verbose(CALL_INFO, 2, 0,
        //               "Thread %u completed execution.\n", CurrThreadID);
        // TODO: Change to ThreadManager logic
        // if(ParentThreadID != 0 ){
        //   done = false;
        //   output->verbose(CALL_INFO, 2, 0,
        //                   "Switching from thread with ThreadID = %u to its parent ThreadID = %u\n",
        //                   ActiveThreadIDs.at(HartToExec), ParentThreadID);
        //   CtxSwitchAlert(ParentThreadID);
        //   SwapToParent = true;
        //   ThreadTable.at(ActiveThreadIDs.at(HartToExec))->SetState(ThreadState::Dead);
        // } else {
          // done = true;
        // }
      // }
    // }
    if( done && AssignedThreads.size() ){
      AssignedThreads.at(HartToDecode)->SetState(ThreadState::Done);
    }
    else if( done && !AssignedThreads.size() ){


      // we are really done, return
      output->verbose(CALL_INFO,2,0,"Program execution complete\n");
      Stats.percentEff = float(Stats.cyclesBusy)/Stats.totalCycles;
      output->verbose(CALL_INFO,2,0,
                      "Program Stats: Total Cycles: %" PRIu64 " Busy Cycles: %" PRIu64 " Idle Cycles: %" PRIu64 " Eff: %f\n",
                      Stats.totalCycles, Stats.cyclesBusy,
                      Stats.cyclesIdle_Total, Stats.percentEff);
      output->verbose(CALL_INFO,3,0,"\t Bytes Read: %d Bytes Written: %d Floats Read: %d Doubles Read %d Floats Exec: %" PRIu64 " TLB Hits: %" PRIu64 " TLB Misses: %" PRIu64 " Inst Retired: %" PRIu64 "\n",
                                      mem->memStats.bytesRead,
                                      mem->memStats.bytesWritten,
                                      mem->memStats.floatsRead,
                                      mem->memStats.doublesRead,
                                      Stats.floatsExec,
                                      mem->memStats.TLBHits,
                                      mem->memStats.TLBMisses,
                                      Retired);
      return false;
    }
  }
  if( AssignedThreads.size() == 0 ){
    return false;
  }

  return rtn;
}


uint32_t RevProc::HartToExecThreadID(){
  if( AssignedThreads.size() <= HartToExec )
    return AssignedThreads.at(HartToExec)->GetThreadID();
  else{
    return 0;
  }
}

// TODO: Replace with ThreadManager logic
std::shared_ptr<RevThread> RevProc::HartToExecCtx(){
  if( HartToExec <= AssignedThreads.size() )
    return AssignedThreads.at(HartToExec);
  else{
    return 0;
  }
}


// bool RevProc::UpdateRegFile(){
  // uint16_t HartID = GetHartID();
  // auto it = ThreadTable.find(ActiveThreadIDs.at(HartID));
  // if( it != ThreadTable.end() ){
  //   std::shared_ptr<RevThread> Ctx = it->second;
  //   RegFile = Ctx->GetRegFile();
  //   return true;
  // }
  // else {
  //   output->fatal(CALL_INFO, -1,
  //                 "Failed to find RegFile for ThreadID = %d on Hart = %d \n", ActiveThreadIDs.at(HartID), HartID);
  // }
  // return false;
// }


RevRegFile* RevProc::GetRegFile(uint16_t HartID){
  if( AssignedThreads.size() < HartID ){
    output->fatal(CALL_INFO, 1,
                  "Tried to get RegFile for HartID = %d but there is no AssignedThread for that Hart\n", HartID);
  }
  return AssignedThreads.at(HartID)->GetRegFile();
}

// bool RevProc::InitThreadTable(){
//   /*
//    * We need to create the first Ctx for each HART which will have the following attributes: 
//    * - ThreadID = 1024 + However many already initialized Ctx objects there are
//    * - ParentThreadID = 0 : (Only the first thread on every RevProc has ParentThreadID = 0)
//    * - MemStartAddr : Top of stack (NOTE: No functionality yet)
//    * - MemStartSize : _DEFAULT_THREAD_MEM_SIZE_ (NOTE: No functionality yet)
//   */

//   for( unsigned HartID=0; HartID<_REV_HART_COUNT_; HartID++){
//     uint32_t ParentThreadID = 0;
//     uint32_t FirstActiveThreadID = mem->GetNewThreadThreadID();

//     std::shared_ptr<RevThread> DefaultCtx = std::make_shared<RevThread>(
//         FirstActiveThreadID,
//         ParentThreadID);

//     /* Set the first RegFile as ActiveRegFile */
//     RegFile = DefaultCtx->GetRegFile();

//     /* Add first ThreadID to ActiveThreadIDs */
//     ActiveThreadIDs.emplace_back(FirstActiveThreadID);

//     /* Add to ThreadTable */
//     ThreadTable.emplace(FirstActiveThreadID, DefaultCtx);
//   }
//   return true;
// }


/* =====================================================
 * ChangeActiveThreadID(NewThreadID)
 * =====================================================
 * This function changes the active pid of HartToExec
 *
 * Returns:
 * - True if successfully changed
 * - False if not (ie. ThreadID doesn't exist)
 *
 * NOTES:
 * - This function automatically sets the new Ctx state to Running
 * - This function automatically sets old Ctx state to Waiting
 */
// TODO: Add ThreadManager logic
bool RevProc::ChangeActiveThreadID(uint32_t NewThreadID){
  // auto it = ThreadTable.find(NewThreadID);
  // if( it != ThreadTable.end() ){
  //   // std::shared_ptr<RevThread> NewCtx = it->second;
  //   /* If switching to parent, output the child is being removed from the ThreadTable */
  //   if( SwapToParent ){
  //     output->verbose(CALL_INFO, 2, 0, "Removing ThreadCtx w/ ThreadID = %d from the ThreadTable\n",
  //                     ActiveThreadIDs.at(HartToExec));
  //     ThreadTable.erase(ActiveThreadIDs.at(HartToExec));
  //   }
  //   ActiveThreadIDs.at(HartToExec) = NewThreadID;
  //   ActiveThreadIDs.at(HartToDecode) = NewThreadID;
  //   UpdateRegFile();
  //   return true;
  // }else{
  //   /* TODO: Maybe don't output fatal? */
  //   output->fatal(CALL_INFO, -1,
  //                 "Failed to load ctx w/ ThreadID=%d into Hart=%d because ThreadID does not exist in ThreadTable\n",
  //                 NewThreadID, HartToExec);
  //   return false;
  // }
}

/* NOTE: This is currently not used but will once more complex scheduling is supported */
/* ChangeActiveThreadID(ThreadID, HartID)
 * This function changes the active pid of HartID
 *
 * Returns:
 * - True if successfully changed
 * - False if not (ie. ThreadID doesn't exist)
 *
 * NOTES:
 * - This function automatically sets the new Ctx state to Running
 * - This function automatically sets old Ctx state to Waiting
 */
// bool RevProc::ChangeActiveThreadID(uint32_t ThreadID, uint16_t HartID){
//   auto NewActiveCtx = ThreadTable.find(ThreadID);
//   if( NewActiveCtx != ThreadTable.end() ){
//     if( ActiveThreadIDs.size() >= HartID ){
//       ActiveThreadIDs.at(HartToExec) = ThreadID;
//       return true;
//     } else {
//     /* TODO: Maybe don't output fatal? */
//       output->fatal(CALL_INFO, -1, "Failed to load ctx w/ ThreadID=%d into Hart=%d because Hart does not exist",
//                     ThreadID, HartToExec);
//       return false;
//     }
//   }else{
//     /* TODO: Maybe don't output fatal? */
//     output->fatal(CALL_INFO, -1,
//                   "Failed to load ctx w/ ThreadID=%d into Hart=%d because ThreadID does not exist in ThreadTable", 
//                   ThreadID, HartToExec);
//     return false;
//   }
// }


// This function is only responsible for submitting the necessary memory requests
// to duplicate the TLS Initialization Template and potentially the Parent's Stack
//
// Once the memory is set up properly it then signals back to RevCPU that a new 
// RevThread object needs to be created
uint32_t RevProc::SpawnThread(uint64_t fn){
  std::cout << "========> Inside of SpawnThread <========" << std::endl;
  std::cout << "FUNCTION POINTER: 0x" << std::hex << fn << std::endl;
  

  // Need to do: 
  // 1) Get a new ThreadID
  // 2) Copy TLS
  // 3a) Potentially Copy Parent's stack
  
  NewThreadInfo.emplace(fn, mem->AddThreadMem());
  // 3b) Potentially Copy Parent's RegFile
  // 3c) Potentially Copy Parent's Parent ThreadID
  // 3d) Potentially set a priority? 
  // 4) Assign the correct stack ptr based on above
  // 5) Don't think we should do (below)... In RevCPU we should check a Proc's AssignedThreads.end() to see if any have a "START" status and then pop them off the back and into Threads
  //      Check if we have a Hart available (ie. AssignedThreads.size < _NUM_HARTS_ [prior to adding this one])
  // 6) Return threadId to the appropriate location (probably a register file -- Probably in the syscall clone tho)

}

/* Returns vector of all ThreadIDs in the ThreadTable */
// std::vector<uint32_t> RevProc::GetThreadIDs(){
//   std::vector<uint32_t> ThreadIDs;
//   for( const auto& Thread : ThreadTable ){
//     ThreadIDs.push_back(Thread.first);
//   }
//   return ThreadIDs;
// }

/* 
 * There are a few assumptions made by this function
 * - The Active Thread is the one creating the child 
 * - The child duplicates the parents RegFile
 * - Automatically adds ChildCtx to the current Procs ThreadTable  
 * - The new Child will start with ThreadState::Ready
*/
// uint32_t RevProc::CreateChildCtx() {
//   /* We get the currently executing ThreadID's context as this is assumed to be the parent */
//   std::shared_ptr<RevThread> ParentCtx = ThreadTable.at(ActiveThreadIDs.at(HartToExec));

//   /* Get new ThreadID from global counter in RevMem */
//   uint32_t ChildThreadID = mem->GetNewThreadThreadID();

//   /* Create ChildCtx as a copy of ParentCtx */
//   auto ChildCtx = std::make_shared<RevThread>(ChildThreadID,
//                                        ActiveThreadIDs.at(HartToExec));

//   /* Child's Regfile is the same as the parent's with the exception of return value */
//   ChildCtx->DuplicateRegFile(*RegFile);

//   /* Add child to Proc's ThreadTable */
//   ThreadTable.emplace(ChildThreadID, ChildCtx);

//   /* Get Child's regfile so we can make the below modifications */
//   RevRegFile* ChildRegFile = ChildCtx->GetRegFile();

//   /* Zero the childs cause registers as they have no exceptions raised */
//   ChildRegFile->RV64_SCAUSE = 0;
//   ChildRegFile->RV32_SCAUSE = 0;

//   /* The child's return value from fork/clone is 0 */
//   ChildRegFile->RV64[10] = 0;

//   /* Add ChildThreadID to list of Parent's Children */
//   ParentCtx->AddChildThreadID(ChildThreadID); /* NOTE: This has no functionality at this point */

//   return ChildThreadID;
// }


/* ========================================= */
/* System Call (ecall) Implementations Below */
/* ========================================= */
void RevProc::InitEcallTable(){
  Ecalls = {
    { 0,   &RevProc::ECALL_io_setup},               //  rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
    { 1,   &RevProc::ECALL_io_destroy},             //  rev_io_destroy(aio_context_t ctx)
    { 2,   &RevProc::ECALL_io_submit},              //  rev_io_submit(aio_context_t, long, struct iocb  *  *)
    { 3,   &RevProc::ECALL_io_cancel},              //  rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
    { 4,   &RevProc::ECALL_io_getevents},           //  rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
    { 5,   &RevProc::ECALL_setxattr},               //  rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
    { 6,   &RevProc::ECALL_lsetxattr},              //  rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
    { 7,   &RevProc::ECALL_fsetxattr},              //  rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
    { 8,   &RevProc::ECALL_getxattr},               //  rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
    { 9,   &RevProc::ECALL_lgetxattr},              //  rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
    { 10,  &RevProc::ECALL_fgetxattr},              //  rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
    { 11,  &RevProc::ECALL_listxattr},              //  rev_listxattr(const char  *path, char  *list, size_t size)
    { 12,  &RevProc::ECALL_llistxattr},             //  rev_llistxattr(const char  *path, char  *list, size_t size)
    { 13,  &RevProc::ECALL_flistxattr},             //  rev_flistxattr(int fd, char  *list, size_t size)
    { 14,  &RevProc::ECALL_removexattr},            //  rev_removexattr(const char  *path, const char  *name)
    { 15,  &RevProc::ECALL_lremovexattr},           //  rev_lremovexattr(const char  *path, const char  *name)
    { 16,  &RevProc::ECALL_fremovexattr},           //  rev_fremovexattr(int fd, const char  *name)
    { 17,  &RevProc::ECALL_getcwd},                 //  rev_getcwd(char  *buf, unsigned long size)
    { 18,  &RevProc::ECALL_lookup_dcookie},         //  rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
    { 19,  &RevProc::ECALL_eventfd2},               //  rev_eventfd2(unsigned int count, int flags)
    { 20,  &RevProc::ECALL_epoll_create1},          //  rev_epoll_create1(int flags)
    { 21,  &RevProc::ECALL_epoll_ctl},              //  rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
    { 22,  &RevProc::ECALL_epoll_pwait},            //  rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
    { 23,  &RevProc::ECALL_dup},                    //  rev_dup(unsigned int fildes)
    { 24,  &RevProc::ECALL_dup3},                   //  rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
    { 25,  &RevProc::ECALL_fcntl64},                //  rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
    { 26,  &RevProc::ECALL_inotify_init1},          //  rev_inotify_init1(int flags)
    { 27,  &RevProc::ECALL_inotify_add_watch},      //  rev_inotify_add_watch(int fd, const char  *path, u32 mask)
    { 28,  &RevProc::ECALL_inotify_rm_watch},       //  rev_inotify_rm_watch(int fd, __s32 wd)
    { 29,  &RevProc::ECALL_ioctl},                  //  rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
    { 30,  &RevProc::ECALL_ioprio_set},             //  rev_ioprio_set(int which, int who, int ioprio)
    { 31,  &RevProc::ECALL_ioprio_get},             //  rev_ioprio_get(int which, int who)
    { 32,  &RevProc::ECALL_flock},                  //  rev_flock(unsigned int fd, unsigned int cmd)
    { 33,  &RevProc::ECALL_mknodat},                //  rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
    { 34,  &RevProc::ECALL_mkdirat},                //  rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
    { 35,  &RevProc::ECALL_unlinkat},               //  rev_unlinkat(int dfd, const char  * pathname, int flag)
    { 36,  &RevProc::ECALL_symlinkat},              //  rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
    { 37,  &RevProc::ECALL_linkat},                 //  rev_unlinkat(int dfd, const char  * pathname, int flag)
    { 38,  &RevProc::ECALL_renameat},               //  rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
    { 39,  &RevProc::ECALL_umount},                 //  rev_umount(char  *name, int flags)
    { 40,  &RevProc::ECALL_mount},                  //  rev_umount(char  *name, int flags)
    { 41,  &RevProc::ECALL_pivot_root},             //  rev_pivot_root(const char  *new_root, const char  *put_old)
    { 42,  &RevProc::ECALL_ni_syscall},             //  rev_ni_syscall(void)
    { 43,  &RevProc::ECALL_statfs64},               //  rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
    { 44,  &RevProc::ECALL_fstatfs64},              //  rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
    { 45,  &RevProc::ECALL_truncate64},             //  rev_truncate64(const char  *path, loff_t length)
    { 46,  &RevProc::ECALL_ftruncate64},            //  rev_ftruncate64(unsigned int fd, loff_t length)
    { 47,  &RevProc::ECALL_fallocate},              //  rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
    { 48,  &RevProc::ECALL_faccessat},              //  rev_faccessat(int dfd, const char  *filename, int mode)
    { 49,  &RevProc::ECALL_chdir},                  //  rev_chdir(const char  *filename)
    { 50,  &RevProc::ECALL_fchdir},                 //  rev_fchdir(unsigned int fd)
    { 51,  &RevProc::ECALL_chroot},                 //  rev_chroot(const char  *filename)
    { 52,  &RevProc::ECALL_fchmod},                 //  rev_fchmod(unsigned int fd, umode_t mode)
    { 53,  &RevProc::ECALL_fchmodat},               //  rev_fchmodat(int dfd, const char  * filename, umode_t mode)
    { 54,  &RevProc::ECALL_fchownat},               //  rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
    { 55,  &RevProc::ECALL_fchown},                 //  rev_fchown(unsigned int fd, uid_t user, gid_t group)
    { 56,  &RevProc::ECALL_openat},                 //  rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
    { 57,  &RevProc::ECALL_close},                  //  rev_close(unsigned int fd)
    { 58,  &RevProc::ECALL_vhangup},                //  rev_vhangup(void)
    { 59,  &RevProc::ECALL_pipe2},                  //  rev_pipe2(int  *fildes, int flags)
    { 60,  &RevProc::ECALL_quotactl},               //  rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
    { 61,  &RevProc::ECALL_getdents64},             //  rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
    { 62,  &RevProc::ECALL_lseek},                  //  rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
    { 63,  &RevProc::ECALL_read},                   //  rev_read(unsigned int fd, char  *buf, size_t count)
    { 64,  &RevProc::ECALL_write},                  //  rev_write(unsigned int fd, const char  *buf, size_t count)
    { 65,  &RevProc::ECALL_readv},                  //  rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
    { 66,  &RevProc::ECALL_writev},                 //  rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
    { 67,  &RevProc::ECALL_pread64},                //  rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
    { 68,  &RevProc::ECALL_pwrite64},               //  rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
    { 69,  &RevProc::ECALL_preadv},                 //  rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
    { 70,  &RevProc::ECALL_pwritev},                //  rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
    { 71,  &RevProc::ECALL_sendfile64},             //  rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
    { 72,  &RevProc::ECALL_pselect6_time32},        //  rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
    { 73,  &RevProc::ECALL_ppoll_time32},           //  rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
    { 74,  &RevProc::ECALL_signalfd4},              //  rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
    { 75,  &RevProc::ECALL_vmsplice},               //  rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
    { 76,  &RevProc::ECALL_splice},                 //  rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
    { 77,  &RevProc::ECALL_tee},                    //  rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
    { 78,  &RevProc::ECALL_readlinkat},             //  rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
    { 79,  &RevProc::ECALL_newfstatat},             //  rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
    { 80,  &RevProc::ECALL_newfstat},               //  rev_newfstat(unsigned int fd, struct stat  *statbuf)
    { 81,  &RevProc::ECALL_sync},                   //  rev_sync(void)
    { 82,  &RevProc::ECALL_fsync},                  //  rev_fsync(unsigned int fd)
    { 83,  &RevProc::ECALL_fdatasync},              //  rev_fdatasync(unsigned int fd)
    { 84,  &RevProc::ECALL_sync_file_range2},       //  rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
    { 84,  &RevProc::ECALL_sync_file_range},        //  rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
    { 85,  &RevProc::ECALL_timerfd_create},         //  rev_timerfd_create(int clockid, int flags)
    { 86,  &RevProc::ECALL_timerfd_settime},        //  rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
    { 87,  &RevProc::ECALL_timerfd_gettime},        //  rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
    { 88,  &RevProc::ECALL_utimensat},              //  rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
    { 89,  &RevProc::ECALL_acct},                   //  rev_acct(const char  *name)
    { 90,  &RevProc::ECALL_capget},                 //  rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
    { 91,  &RevProc::ECALL_capset},                 //  rev_capset(cap_user_header_t header, const cap_user_data_t data)
    { 92,  &RevProc::ECALL_personality},            //  rev_personality(unsigned int personality)
    { 93,  &RevProc::ECALL_exit},                   //  rev_exit(int error_code)
    { 94,  &RevProc::ECALL_exit_group},             //  rev_exit_group(int error_code)
    { 95,  &RevProc::ECALL_waitid},                 //  rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
    { 96,  &RevProc::ECALL_set_tid_address},        //  rev_set_tid_address(int  *tidptr)
    { 97,  &RevProc::ECALL_unshare},                //  rev_unshare(unsigned long unshare_flags)
    { 98,  &RevProc::ECALL_futex},                  //  rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
    { 99,  &RevProc::ECALL_set_robust_list},        //  rev_set_robust_list(struct robust_list_head  *head, size_t len)
    { 100, &RevProc::ECALL_get_robust_list},        //  rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
    { 101, &RevProc::ECALL_nanosleep},              //  rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
    { 102, &RevProc::ECALL_getitimer},              //  rev_getitimer(int which, struct __kernel_old_itimerval  *value)
    { 103, &RevProc::ECALL_setitimer},              //  rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
    { 104, &RevProc::ECALL_kexec_load},             //  rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
    { 105, &RevProc::ECALL_init_module},            //  rev_init_module(void  *umod, unsigned long len, const char  *uargs)
    { 106, &RevProc::ECALL_delete_module},          //  rev_delete_module(const char  *name_user, unsigned int flags)
    { 107, &RevProc::ECALL_timer_create},           //  rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
    { 108, &RevProc::ECALL_timer_gettime},          //  rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
    { 109, &RevProc::ECALL_timer_getoverrun},       //  rev_timer_getoverrun(timer_t timer_id)
    { 110, &RevProc::ECALL_timer_settime},          //  rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
    { 111, &RevProc::ECALL_timer_delete},           //  rev_timer_delete(timer_t timer_id)
    { 112, &RevProc::ECALL_clock_settime},          //  rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
    { 113, &RevProc::ECALL_clock_gettime},          //  rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
    { 114, &RevProc::ECALL_clock_getres},           //  rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
    { 115, &RevProc::ECALL_clock_nanosleep},        //  rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
    { 116, &RevProc::ECALL_syslog},                 //  rev_syslog(int type, char  *buf, int len)
    { 117, &RevProc::ECALL_ptrace},                 //  rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
    { 118, &RevProc::ECALL_sched_setparam},         //  rev_sched_setparam(pid_t pid, struct sched_param  *param)
    { 119, &RevProc::ECALL_sched_setscheduler},     //  rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
    { 120, &RevProc::ECALL_sched_getscheduler},     //  rev_sched_getscheduler(pid_t pid)
    { 121, &RevProc::ECALL_sched_getparam},         //  rev_sched_getparam(pid_t pid, struct sched_param  *param)
    { 122, &RevProc::ECALL_sched_setaffinity},      //  rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
    { 123, &RevProc::ECALL_sched_getaffinity},      //  rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
    { 124, &RevProc::ECALL_sched_yield},            //  rev_sched_yield(void)
    { 125, &RevProc::ECALL_sched_get_priority_max}, //  rev_sched_get_priority_max(int policy)
    { 126, &RevProc::ECALL_sched_get_priority_min}, //  rev_sched_get_priority_min(int policy)
    { 127, &RevProc::ECALL_sched_rr_get_interval},  //  rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
    { 128, &RevProc::ECALL_restart_syscall},        //  rev_restart_syscall(void)
    { 129, &RevProc::ECALL_kill},                   //  rev_kill(pid_t pid, int sig)
    { 130, &RevProc::ECALL_tkill},                  //  rev_tkill(pid_t pid, int sig)
    { 131, &RevProc::ECALL_tgkill},                 //  rev_tgkill(pid_t tgid, pid_t pid, int sig)
    { 132, &RevProc::ECALL_sigaltstack},            //  rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
    { 133, &RevProc::ECALL_rt_sigsuspend},          //  rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
    { 134, &RevProc::ECALL_rt_sigaction},           //  rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
    { 135, &RevProc::ECALL_rt_sigprocmask},         //  rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
    { 136, &RevProc::ECALL_rt_sigpending},          //  rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
    { 137, &RevProc::ECALL_rt_sigtimedwait_time32}, //  rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
    { 138, &RevProc::ECALL_rt_sigqueueinfo},        //  rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
    { 140, &RevProc::ECALL_setpriority},            //  rev_setpriority(int which, int who, int niceval)
    { 141, &RevProc::ECALL_getpriority},            //  rev_getpriority(int which, int who)
    { 142, &RevProc::ECALL_reboot},                 //  rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
    { 143, &RevProc::ECALL_setregid},               //  rev_setregid(gid_t rgid, gid_t egid)
    { 144, &RevProc::ECALL_setgid},                 //  rev_setgid(gid_t gid)
    { 145, &RevProc::ECALL_setreuid},               //  rev_setreuid(uid_t ruid, uid_t euid)
    { 146, &RevProc::ECALL_setuid},                 //  rev_setuid(uid_t uid)
    { 147, &RevProc::ECALL_setresuid},              //  rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
    { 148, &RevProc::ECALL_getresuid},              //  rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
    { 149, &RevProc::ECALL_setresgid},              //  rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
    { 150, &RevProc::ECALL_getresgid},              //  rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
    { 151, &RevProc::ECALL_setfsuid},               //  rev_setfsuid(uid_t uid)
    { 152, &RevProc::ECALL_setfsgid},               //  rev_setfsgid(gid_t gid)
    { 153, &RevProc::ECALL_times},                  //  rev_times(struct tms  *tbuf)
    { 154, &RevProc::ECALL_setpgid},                //  rev_setpgid(pid_t pid, pid_t pgid)
    { 155, &RevProc::ECALL_getpgid},                //  rev_getpgid(pid_t pid)
    { 156, &RevProc::ECALL_getsid},                 //  rev_getsid(pid_t pid)
    { 157, &RevProc::ECALL_setsid},                 //  rev_setsid(void)
    { 158, &RevProc::ECALL_getgroups},              //  rev_getgroups(int gidsetsize, gid_t  *grouplist)
    { 159, &RevProc::ECALL_setgroups},              //  rev_setgroups(int gidsetsize, gid_t  *grouplist)
    { 160, &RevProc::ECALL_newuname},               //  rev_newuname(struct new_utsname  *name)
    { 161, &RevProc::ECALL_sethostname},            //  rev_sethostname(char  *name, int len)
    { 162, &RevProc::ECALL_setdomainname},          //  rev_setdomainname(char  *name, int len)
    { 163, &RevProc::ECALL_getrlimit},              //  rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
    { 164, &RevProc::ECALL_setrlimit},              //  rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
    { 165, &RevProc::ECALL_getrusage},              //  rev_getrusage(int who, struct rusage  *ru)
    { 166, &RevProc::ECALL_umask},                  //  rev_umask(int mask)
    { 167, &RevProc::ECALL_prctl},                  //  rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
    { 168, &RevProc::ECALL_getcpu},                 //  rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
    { 169, &RevProc::ECALL_gettimeofday},           //  rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
    { 170, &RevProc::ECALL_settimeofday},           //  rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
    { 171, &RevProc::ECALL_adjtimex},               //  rev_adjtimex(struct __kernel_timex  *txc_p)
    { 172, &RevProc::ECALL_getpid},                 //  rev_getpid(void)
    { 173, &RevProc::ECALL_getppid},                //  rev_getppid(void)
    { 174, &RevProc::ECALL_getuid},                 //  rev_getuid(void)
    { 175, &RevProc::ECALL_geteuid},                //  rev_geteuid(void)
    { 176, &RevProc::ECALL_getgid},                 //  rev_getgid(void)
    { 177, &RevProc::ECALL_getegid},                //  rev_getegid(void)
    { 178, &RevProc::ECALL_gettid},                 //  rev_gettid(void)
    { 179, &RevProc::ECALL_sysinfo},                //  rev_sysinfo(struct sysinfo  *info)
    { 180, &RevProc::ECALL_mq_open},                //  rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
    { 181, &RevProc::ECALL_mq_unlink},              //  rev_mq_unlink(const char  *name)
    { 182, &RevProc::ECALL_mq_timedsend},           //  rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
    { 183, &RevProc::ECALL_mq_timedreceive},        //  rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
    { 184, &RevProc::ECALL_mq_notify},              //  rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
    { 185, &RevProc::ECALL_mq_getsetattr},          //  rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
    { 186, &RevProc::ECALL_msgget},                 //  rev_msgget(key_t key, int msgflg)
    { 187, &RevProc::ECALL_msgctl},                 //  rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
    { 188, &RevProc::ECALL_msgrcv},                 //  rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
    { 189, &RevProc::ECALL_msgsnd},                 //  rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
    { 190, &RevProc::ECALL_semget},                 //  rev_semget(key_t key, int nsems, int semflg)
    { 191, &RevProc::ECALL_semctl},                 //  rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
    { 192, &RevProc::ECALL_semtimedop},             //  rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
    { 193, &RevProc::ECALL_semop},                  //  rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
    { 194, &RevProc::ECALL_shmget},                 //  rev_shmget(key_t key, size_t size, int flag)
    { 195, &RevProc::ECALL_shmctl},                 //  rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
    { 196, &RevProc::ECALL_shmat},                  //  rev_shmat(int shmid, char  *shmaddr, int shmflg)
    { 197, &RevProc::ECALL_shmdt},                  //  rev_shmdt(char  *shmaddr)
    { 198, &RevProc::ECALL_socket},                 //  rev_socket(int, int, int)
    { 199, &RevProc::ECALL_socketpair},             //  rev_socketpair(int, int, int, int  *)
    { 200, &RevProc::ECALL_bind},                   //  rev_bind(int, struct sockaddr  *, int)
    { 201, &RevProc::ECALL_listen},                 //  rev_listen(int, int)
    { 202, &RevProc::ECALL_accept},                 //  rev_accept(int, struct sockaddr  *, int  *)
    { 203, &RevProc::ECALL_connect},                //  rev_connect(int, struct sockaddr  *, int)
    { 204, &RevProc::ECALL_getsockname},            //  rev_getsockname(int, struct sockaddr  *, int  *)
    { 205, &RevProc::ECALL_getpeername},            //  rev_getpeername(int, struct sockaddr  *, int  *)
    { 206, &RevProc::ECALL_sendto},                 //  rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
    { 207, &RevProc::ECALL_recvfrom},               //  rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
    { 208, &RevProc::ECALL_setsockopt},             //  rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
    { 209, &RevProc::ECALL_getsockopt},             //  rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
    { 210, &RevProc::ECALL_shutdown},               //  rev_shutdown(int, int)
    { 211, &RevProc::ECALL_sendmsg},                //  rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
    { 212, &RevProc::ECALL_recvmsg},                //  rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
    { 213, &RevProc::ECALL_readahead},              //  rev_readahead(int fd, loff_t offset, size_t count)
    { 214, &RevProc::ECALL_brk},                    //  rev_brk(unsigned long brk)
    { 215, &RevProc::ECALL_munmap},                 //  rev_munmap(unsigned long addr, size_t len)
    { 216, &RevProc::ECALL_mremap},                 //  rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
    { 217, &RevProc::ECALL_add_key},                //  rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
    { 218, &RevProc::ECALL_request_key},            //  rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
    { 219, &RevProc::ECALL_keyctl},                 //  rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
    { 220, &RevProc::ECALL_clone},                  //  rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
    { 221, &RevProc::ECALL_execve},                 //  rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
    { 222, &RevProc::ECALL_mmap},                   //  rev_old_mmap(struct mmap_arg_struct  *arg)
    { 223, &RevProc::ECALL_fadvise64_64},           //  rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
    { 224, &RevProc::ECALL_swapon},                 //  rev_swapon(const char  *specialfile, int swap_flags)
    { 225, &RevProc::ECALL_swapoff},                //  rev_swapoff(const char  *specialfile)
    { 226, &RevProc::ECALL_mprotect},               //  rev_mprotect(unsigned long start, size_t len, unsigned long prot)
    { 227, &RevProc::ECALL_msync},                  //  rev_msync(unsigned long start, size_t len, int flags)
    { 228, &RevProc::ECALL_mlock},                  //  rev_mlock(unsigned long start, size_t len)
    { 229, &RevProc::ECALL_munlock},                //  rev_munlock(unsigned long start, size_t len)
    { 230, &RevProc::ECALL_mlockall},               //  rev_mlockall(int flags)
    { 231, &RevProc::ECALL_munlockall},             //  rev_munlockall(void)
    { 232, &RevProc::ECALL_mincore},                //  rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
    { 233, &RevProc::ECALL_madvise},                //  rev_madvise(unsigned long start, size_t len, int behavior)
    { 234, &RevProc::ECALL_remap_file_pages},       //  rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
    { 235, &RevProc::ECALL_mbind},                  //  rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
    { 236, &RevProc::ECALL_get_mempolicy},          //  rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
    { 237, &RevProc::ECALL_set_mempolicy},          //  rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
    { 238, &RevProc::ECALL_migrate_pages},          //  rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
    { 239, &RevProc::ECALL_move_pages},             //  rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
    { 240, &RevProc::ECALL_rt_tgsigqueueinfo},      //  rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
    { 241, &RevProc::ECALL_perf_event_open},        //  rev_perf_event_open(")
    { 242, &RevProc::ECALL_accept4},                //  rev_accept4(int, struct sockaddr  *, int  *, int)
    { 243, &RevProc::ECALL_recvmmsg_time32},        //  rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
    { 260, &RevProc::ECALL_wait4},                  //  rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
    { 261, &RevProc::ECALL_prlimit64},              //  rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
    { 262, &RevProc::ECALL_fanotify_init},          //  rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
    { 263, &RevProc::ECALL_fanotify_mark},          //  rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
    { 264, &RevProc::ECALL_name_to_handle_at},      //  rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
    { 265, &RevProc::ECALL_open_by_handle_at},      //  rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
    { 266, &RevProc::ECALL_clock_adjtime},          //  rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
    { 267, &RevProc::ECALL_syncfs},                 //  rev_syncfs(int fd)
    { 268, &RevProc::ECALL_setns},                  //  rev_setns(int fd, int nstype)
    { 269, &RevProc::ECALL_sendmmsg},               //  rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
    { 270, &RevProc::ECALL_process_vm_readv},       //  rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
    { 271, &RevProc::ECALL_process_vm_writev},      //  rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
    { 272, &RevProc::ECALL_kcmp},                   //  rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
    { 273, &RevProc::ECALL_finit_module},           //  rev_finit_module(int fd, const char  *uargs, int flags)
    { 274, &RevProc::ECALL_sched_setattr},          //  rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
    { 275, &RevProc::ECALL_sched_getattr},          //  rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
    { 276, &RevProc::ECALL_renameat2},              //  rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
    { 277, &RevProc::ECALL_seccomp},                //  rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
    { 278, &RevProc::ECALL_getrandom},              //  rev_getrandom(char  *buf, size_t count, unsigned int flags)
    { 279, &RevProc::ECALL_memfd_create},           //  rev_memfd_create(const char  *uname_ptr, unsigned int flags)
    { 280, &RevProc::ECALL_bpf},                    //  rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
    { 281, &RevProc::ECALL_execveat},               //  rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
    { 282, &RevProc::ECALL_userfaultfd},            //  rev_userfaultfd(int flags)
    { 283, &RevProc::ECALL_membarrier},             //  rev_membarrier(int cmd, unsigned int flags, int cpu_id)
    { 284, &RevProc::ECALL_mlock2},                 //  rev_mlock2(unsigned long start, size_t len, int flags)
    { 285, &RevProc::ECALL_copy_file_range},        //  rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
    { 286, &RevProc::ECALL_preadv2},                //  rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
    { 287, &RevProc::ECALL_pwritev2},               //  rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
    { 288, &RevProc::ECALL_pkey_mprotect},          //  rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
    { 289, &RevProc::ECALL_pkey_alloc},             //  rev_pkey_alloc(unsigned long flags, unsigned long init_val)
    { 290, &RevProc::ECALL_pkey_free},              //  rev_pkey_free(int pkey)
    { 291, &RevProc::ECALL_statx},                  //  rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
    { 292, &RevProc::ECALL_io_pgetevents},          //  rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
    { 293, &RevProc::ECALL_rseq},                   //  rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
    { 294, &RevProc::ECALL_kexec_file_load},        //  rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
    { 403, &RevProc::ECALL_clock_gettime},          //  rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
    { 404, &RevProc::ECALL_clock_settime},          //  rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
    { 405, &RevProc::ECALL_clock_adjtime},          //  rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
    { 406, &RevProc::ECALL_clock_getres},           //  rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
    { 407, &RevProc::ECALL_clock_nanosleep},        //  rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
    { 408, &RevProc::ECALL_timer_gettime},          //  rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
    { 409, &RevProc::ECALL_timer_settime},          //  rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
    { 410, &RevProc::ECALL_timerfd_gettime},        //  rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
    { 411, &RevProc::ECALL_timerfd_settime},        //  rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
    { 412, &RevProc::ECALL_utimensat},              //  rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
    { 416, &RevProc::ECALL_io_pgetevents},          //  rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
    { 418, &RevProc::ECALL_mq_timedsend},           //  rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
    { 419, &RevProc::ECALL_mq_timedreceive},        //  rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
    { 420, &RevProc::ECALL_semtimedop},             //  rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
    { 422, &RevProc::ECALL_futex},                  //  rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
    { 423, &RevProc::ECALL_sched_rr_get_interval},  //  rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
    { 424, &RevProc::ECALL_pidfd_send_signal},      //  rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
    { 425, &RevProc::ECALL_io_uring_setup},         //  rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
    { 426, &RevProc::ECALL_io_uring_enter},         //  rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
    { 427, &RevProc::ECALL_io_uring_register},      //  rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
    { 428, &RevProc::ECALL_open_tree},              //  rev_open_tree(int dfd, const char  *path, unsigned flags)
    { 429, &RevProc::ECALL_move_mount},             //  rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
    { 430, &RevProc::ECALL_fsopen},                 //  rev_fsopen(const char  *fs_name, unsigned int flags)
    { 431, &RevProc::ECALL_fsconfig},               //  rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
    { 432, &RevProc::ECALL_fsmount},                //  rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
    { 433, &RevProc::ECALL_fspick},                 //  rev_fspick(int dfd, const char  *path, unsigned int flags)
    { 434, &RevProc::ECALL_pidfd_open},             //  rev_pidfd_open(pid_t pid, unsigned int flags)
    { 435, &RevProc::ECALL_clone3},                 //  rev_clone3(struct clone_args  *uargs, size_t size)
    { 436, &RevProc::ECALL_close_range},            //  rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
    { 437, &RevProc::ECALL_openat2},                //  rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
    { 438, &RevProc::ECALL_pidfd_getfd},            //  rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
    { 439, &RevProc::ECALL_faccessat2},             //  rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
    { 440, &RevProc::ECALL_process_madvise},        //  rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)
    { 1000, &RevProc::ECALL_pthread_create},        //  
    };
}

/* ======================================================= */
/* rev_setxattr(const char *path, const char *name,        */
/*              const void *value, size_t size, int flags) */
/*======================================================== */
RevProc::ECALL_status_t RevProc::ECALL_setxattr(RevInst& inst){
  const char *path = (char*)RegFile->RV64[10];
  const char *name = (char*)RegFile->RV64[11];
  const void *value = (void*)RegFile->RV64[12];
  size_t size = RegFile->RV64[13];
  uint64_t flags = RegFile->RV64[14];

#ifdef __APPLE__
  uint32_t position = 0;
  uint64_t rc = setxattr(path, name, value, size, position, flags);
#else
  uint64_t rc = setxattr(path, name, value, size, flags);
#endif
  RegFile->RV64[10] = rc;
  return ECALL_status_t::SUCCESS;
}

/* Increments program break by n bytes  */
RevProc::ECALL_status_t RevProc::ECALL_brk(RevInst& inst){
  uint64_t Addr = RegFile->RV64[10];

  const uint64_t heapend = mem->GetHeapEnd();
  if( Addr > 0 && Addr > heapend ){
    uint64_t Size = Addr - heapend;
    mem->ExpandHeap(Size);
  } else {
    output->fatal(CALL_INFO, 11,
                  "Out of memory / Unable to expand system break (brk) to Addr = 0x%lx", Addr);
  }
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ======================================================= */
/* rev_clone3(struct clone_args*, size_t args_size)        */
/* ======================================================= */
RevProc::ECALL_status_t RevProc::ECALL_clone(RevInst& inst){
  uint64_t CloneArgsAddr = RegFile->RV64[10];
  RevProc::ECALL_status_t rtval = RevProc::ECALL_status_t::SUCCESS;
  // size_t SizeOfCloneArgs = RegFile()->RV64[11];

 if(0 == ECALL_bytesRead){
    // First time through the function... 
    /* Fetch the clone_args */
    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
                                // using the 1st uint64, so that's all we're going to fetch
    uint64_t* args = reinterpret_cast<uint64_t*>(ECALL_buf);
    mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
    ECALL_bytesRead = 8;
    rtval = RevProc::ECALL_status_t::CONTINUE;
 }else{
    /*
    * Parse clone flags
    * NOTE: if no flags are set, we get fork() like behavior
    */
   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL_buf);
    for( uint64_t bit=1; bit != 0; bit <<= 1 ){
      switch (*args & bit) {
        case CLONE_VM:
          // std::cout << "CLONE_VM is true" << std::endl;
          break;
        case CLONE_FS: /* Set if fs info shared between processes */
          // std::cout << "CLONE_FS is true" << std::endl;
          break;
        case CLONE_FILES: /* Set if open files shared between processes */
          // std::cout << "CLONE_FILES is true" << std::endl;
          break;
        case CLONE_SIGHAND: /* Set if signal handlers shared */
          // std::cout << "CLONE_SIGHAND is true" << std::endl;
          break;
        case CLONE_PIDFD: /* Set if a pidfd should be placed in the parent */
          // std::cout << "CLONE_PIDFD is true" << std::endl;
          break;
        case CLONE_PTRACE: /* Set if tracing continues on the child */
          // std::cout << "CLONE_PTRACE is true" << std::endl;
          break;
        case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
          // std::cout << "CLONE_VFORK is true" << std::endl;
          break;
        case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
          // std::cout << "CLONE_PARENT is true" << std::endl;
          break;
        case CLONE_THREAD: /* Set to add to same thread group */
          // std::cout << "CLONE_THREAD is true" << std::endl;
          break;
        case CLONE_NEWNS: /* Set to create new namespace */
          // std::cout << "CLONE_NEWNS is true" << std::endl;
          break;
        case CLONE_SYSVSEM: /* Set to shared SVID SEM_UNDO semantics */
          // std::cout << "CLONE_SYSVSEM is true" << std::endl;
          break;
        case CLONE_SETTLS: /* Set TLS info */
          // std::cout << "CLONE_SETTLS is true" << std::endl;
          break;
        case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
          // std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
          break;
        case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
          // std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
          break;
        case CLONE_DETACHED: /* Create clone detached */
          // std::cout << "CLONE_DETACHED is true" << std::endl;
          break;
        case CLONE_UNTRACED: /* Set if the tracing process can't force CLONE_PTRACE on this clone */
          // std::cout << "CLONE_UNTRACED is true" << std::endl;
          break;
        case CLONE_CHILD_SETTID: /* New cgroup namespace */
          // std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
          break;
        case CLONE_NEWCGROUP: /* New cgroup namespace */
          // std::cout << "CLONE_NEWCGROUP is true" << std::endl;
          break;
        case CLONE_NEWUTS: /* New utsname group */
          // std::cout << "CLONE_NEWUTS is true" << std::endl;
          break;
        case CLONE_NEWIPC: /* New ipcs */
          // std::cout << "CLONE_NEWIPC is true" << std::endl;
          break;
        case CLONE_NEWUSER: /* New user namespace */
          // std::cout << "CLONE_NEWUSER is true" << std::endl;
          break;
        case CLONE_NEWPID: /* New pid namespace */
          // std::cout << "CLONE_NEWPID is true" << std::endl;
          break;
        case CLONE_NEWNET: /* New network namespace */
          // std::cout << "CLONE_NEWNET is true" << std::endl;
          break;
        case CLONE_IO: /* Clone I/O Context */
          // std::cout << "CLONE_IO is true" << std::endl;
          break;
        default:
          break;
      } // switch
    } // for

    /* Get the parent ctx (Current active, executing PID) */
    std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

    /* Create the child ctx */
    uint32_t ChildPID = CreateChildCtx();
    std::shared_ptr<RevThreadCtx> ChildCtx = ThreadTable.at(ChildPID);

    /* TODO: Create a copy of Parents Memory Space (need Demand Paging first) */

    /*
    * ===========================================================================================
    * Register File
    * ===========================================================================================
    * We need to duplicate the parent's RegFile to to the Childs
    * - NOTE: when we return from this function, the return value will
    *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
    *         register. In a traditional fork code this looks like:
    *
    *         pid_t pid = fork()
    *         if pid < 0: // Error
    *         else if pid = 0: // New Child Process
    *         else: // Parent Process
    *
    *         In this case, the value of pid is the value thats returned to a0
    *         It follows that
    *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
    *         - The Parent's a0 register MUST have its PID in it
    * ===========================================================================================
    */

    /*
    Alert the Proc there needs to be a Ctx switch
    Pass the PID that will be switched to once the 
    current pipeline is executed until completion
    */
    CtxSwitchAlert(ChildPID);

    /* Parent's return value is the child's PID */
    RegFile->RV64[10] = ChildPID;

    /* Child's return value is 0 */
    ChildCtx->GetRegFile()->RV64[10] = 0;

    /*clean up ecall state*/
    rtval = RevProc::ECALL_status_t::SUCCESS;
    ECALL_bytesRead = 0;
    ECALL_buf[0] = '\0';

  } //else
  return rtval;
}


/* =============================== */
/* rev_chdir(const char *filename) */
/* =============================== */
RevProc::ECALL_status_t RevProc::ECALL_chdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_chdir called\n");
  RevProc::ECALL_status_t rtval = ECALL_status_t::SUCCESS;

  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  if('\0' != ECALL_buf[0]){
    //We are in the middle of the string
    ECALL_string = ECALL_string + ECALL_buf[0];
    mem->ReadVal<char>(HartToExec, RegFile->RV64[10] + sizeof(char)*ECALL_string.length(), &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    rtval = RevProc::ECALL_status_t::CONTINUE;
  }else if(('\0' == ECALL_buf[0]) && (ECALL_string.length() > 0)) {
    //found the null terminator - we're done
    ECALL_string = ECALL_string + ECALL_buf[0];
    const int rc = chdir(ECALL_string.data());
    RegFile->RV64[10] = rc;
    ECALL_string.clear();   //reset the ECALL buffers
    ECALL_buf[0] = '\0';
    DependencyClear(HartToExec, 10, false);
    rtval = RevProc::ECALL_status_t::SUCCESS;
  }else{
    //first time through the ECALL
    mem->ReadVal<char>(HartToExec, RegFile->RV64[10], &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    rtval = RevProc::ECALL_status_t::CONTINUE;
    DependencySet(HartToExec, 10, false);
  }

  return rtval;
}

/* ============================================================ */
/* rev_mkdirat(int dfd, const char * path, unsigned short mode) */
/* ============================================================ */
// RevProc::ECALL_status_t RevProc::ECALL_mkdir(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL_mkdir called\n");
//   std::string path = "";
//   unsigned i=0;

//   // we don't know how long the path string is so read a byte (char)
//   // at a time and search for the string terminator character '\0'
//   do {
//     char dirchar;
//     mem->ReadMem(RegFile->RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
//     path = path + dirchar;
//     i++;
//   } while( path.back() != '\0');

//   const int rc = chdir(path.data());
//   RegFile->RV64[10] = rc;
// }



/* ======================== */
/* rev_exit(int error_code) */
/* ======================== */
RevProc::ECALL_status_t RevProc::ECALL_exit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_exit called\n");
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();
  const uint64_t status = RegFile->RV64[10];

  /* If the current ctx has ParentPID = 0,
     it has no parent and we should terminate the sim */
  if( CurrCtx->GetParentPID() == 0 ){
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
    exit(status);
  } else {
    /* Parent exists & Child is exiting... switch back to parent */
    CtxSwitchAlert(CurrCtx->GetParentPID());
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
    return RevProc::ECALL_status_t::SUCCESS;
  }
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================= */
/* rev_getcwd(char *buf, unsigned long size) */
/* ========================================= */
RevProc::ECALL_status_t RevProc::ECALL_getcwd(RevInst& inst){
  uint64_t BufAddr = RegFile->RV64[10];
  uint64_t size = RegFile->RV64[11];
  std::string CWD = std::filesystem::current_path().c_str();
  mem->WriteMem(feature->GetHart(), BufAddr, size, &CWD);

  /* Returns null-terminated string in buf */
  RegFile->RV64[10] = BufAddr;

  return RevProc::ECALL_status_t::SUCCESS;
}

/* ================ */
/* rev_getpid(void) */
/* ================ */
RevProc::ECALL_status_t RevProc::ECALL_getpid(RevInst& inst){
  /* TODO: Implement error handling */
  output->verbose(CALL_INFO, 2, 0, "ECALL_getpid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  RegFile->RV64[10] = ActivePIDs.at(HartToExec);
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ================= */
/* rev_getppid(void) */
/* ================= */
RevProc::ECALL_status_t RevProc::ECALL_getppid(RevInst& inst){
/* TODO: Implement error handling */
  output->verbose(CALL_INFO, 2, 0, "ECALL_getppid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  uint32_t ParentPID = CurrentCtx->GetParentPID();
  RegFile->RV64[10] = ParentPID;
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================================== */
/* rev_write(unsigned int fd, const char *buf, size_t nbytes) */
/* ========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_write(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_write called\n");
  int fildes = RegFile->RV64[10];
  std::size_t nbytes = RegFile->RV64[12];

  RevProc::ECALL_status_t rtv = RevProc::ECALL_status_t::SUCCESS;

  if(ECALL_bytesRead == nbytes){
    //Read is complete - send to host
    ECALL_string = ECALL_string.append(ECALL_buf);
    /* Perform the write on the host system */
    const int rc = write(fildes, ECALL_string.data(), nbytes);

    /* write returns the number of bytes written */
    RegFile->RV64[10] = rc;

    /*Reset our tracking state*/
    ECALL_bytesRead = 0;
    ECALL_buf[0] = '\0';
    ECALL_string.clear();
    rtv = RevProc::ECALL_status_t::SUCCESS;
    DependencyClear(HartToExec, 10, false);

  }else {
    if(ECALL_bytesRead > 0){
      //Not our first time through... so capture previous read data
      ECALL_string = ECALL_string.append(ECALL_buf);
    }
    if(1 == (nbytes - ECALL_bytesRead)){
      mem->ReadVal<char>(HartToExec, RegFile->RV64[11] + ECALL_bytesRead, ECALL_buf, inst.hazard, REVMEM_FLAGS(0x0));
      ECALL_buf[1] = '\0';  //Pre-null terminate to allow for succesful string cat later
      ECALL_bytesRead += sizeof(char);
    }
    else if(3 >= (nbytes - ECALL_bytesRead )){
      mem->ReadVal<int16_t>(HartToExec, RegFile->RV64[11] + ECALL_bytesRead, reinterpret_cast<int16_t*>(ECALL_buf), inst.hazard, REVMEM_FLAGS(0x0));
      ECALL_buf[2] = '\0'; //Pre-null terminate to allow for succesful string cat later
      ECALL_bytesRead += sizeof(int16_t);
    }
    else if(7 >= (nbytes - ECALL_bytesRead )){
      mem->ReadVal<int32_t>(HartToExec, RegFile->RV64[11] + ECALL_bytesRead, reinterpret_cast<int32_t*>(ECALL_buf), inst.hazard, REVMEM_FLAGS(0x0));
      ECALL_buf[4] = '\0'; //Pre-null terminate to allow for succesful string cat later
      ECALL_bytesRead += sizeof(int32_t);
    }
    else {
      mem->ReadVal<int64_t>(HartToExec, RegFile->RV64[11] + ECALL_bytesRead, reinterpret_cast<int64_t*>(ECALL_buf), inst.hazard, REVMEM_FLAGS(0x0));
      ECALL_buf[8] = '\0'; //Pre-null terminate to allow for succesful string cat later
     ECALL_bytesRead += sizeof(int64_t);
    }
    rtv = RevProc::ECALL_status_t::CONTINUE;
    DependencySet(HartToExec, 10, false);
  }
  return rtv;
}


/* ========================================================================== */
/* rev_timer_settime(timer_t timer_id, int flags, */
/*   const struct __kernel_itimerspec  *new_setting, */
/*   struct __kernel_itimerspec  *old_setting) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ======================================================================== */
/* rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *setting) */
/* ======================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_clock_settime(clockid_t which_clock, */
/* const struct __kernel_timespec *tp) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_clock_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============================================================ */
/* rev_clock_gettime(clockid_t which_clock, struct timeval *tp) */
/* ============================================================ */
RevProc::ECALL_status_t RevProc::ECALL_clock_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ====================================== */
/* rev_mmap(struct mmap_arg_struct *args) */
/* ====================================== */
// void *mmap(void *addr, size_t length, int prot, int flags,
//          int fd, off_t offset);
RevProc::ECALL_status_t RevProc::ECALL_mmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mmap called\n"); 

  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];
  // uint64_t Prot = RegFile->RV64[12];
  // uint64_t Flags = RegFile->RV64[13];
  // uint64_t fd = RegFile->RV64[14];
  // uint64_t offset = RegFile->RV64[15];

  if( !Addr ){
    // If address is NULL... We add it to MemSegs.end()->getTopAddr()+1
    Addr = mem->AllocMem(Size+1);
    // Addr = mem->AddMemSeg(Size); 
  } else {
    // We were passed an address... try to put a segment there.
    // Currently there is no handling of getting it 'close' to the 
    // suggested address... instead if it can't allocate a new segment 
    // there it fails.
    if( !mem->AllocMemAt(Addr, Size) ){
      output->fatal(CALL_INFO, 11, "Failed to add mem segment\n");
    }
  }
  RegFile->RV64[10] = Addr;
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ================================== */
/* munmap(void *addr, size_t length); */
/* ================================== */
RevProc::ECALL_status_t RevProc::ECALL_munmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munmap called\n"); 
  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];

  int rc =  mem->DeallocMem(Addr, Size) == -1;
  if(rc == -1){
    output->fatal(CALL_INFO, 11, 
                  "Failed to perform munmap(Addr = 0x%lx, Size = 0x%lx)"
                  "likely because the memory was not allocated to begin with" , 
                  Addr, Size);
  }

  RegFile->RV64[10] = rc;
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ================ */
/* rev_gettid(void) */
/* ================ */
RevProc::ECALL_status_t RevProc::ECALL_gettid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettid called\n");
  RevRegFile* regFile = RegFile;

  /* rc = Currently Executing Hart */
  regFile->RV64[10] = HartToExec;
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================================= */
/* rev_settimeofday(struct timeval *tv, struct timezone *tz) */
/* ========================================================= */
RevProc::ECALL_status_t RevProc::ECALL_settimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: settimeofday called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============================================================= */
/* int rev_gettimeofday(struct timeval *tv, struct timezone *tz) */
/* ============================================================= */
RevProc::ECALL_status_t RevProc::ECALL_gettimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettimeofday called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ========================================================================== */
/* rev_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset, */
/* size_t sigsetsize) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_rt_sigprocmask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigprocmask called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ================================== */
/* rev_timer_delete(timer_t timer_id) */
/* ================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_delete(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_delete called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ===========================================================================*/
/* rev_timer_create(clockid_t which_clock, struct sigevent *timer_event_spec, */
/*   timer_t *created_timer_id) */
/* ===========================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_timer_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_create called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ========================================================================== */
/* rev_nanosleep(struct __kernel_timespec *rqtp, */
/* struct __kernel_timespec *rmtp) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: nanosleep called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_get_robust_list(int pid, struct robust_list_head *head_ptr, */
/*   size_t *len_ptr) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_get_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_robust_list called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============================================================== */
/* rev_set_robust_list(struct robust_list_head *head, size_t len) */
/* ============================================================== */
RevProc::ECALL_status_t RevProc::ECALL_set_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_robust_list called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, */
/*   struct rusage  *ru) */
/* ==========================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_waitid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: waitid called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============================== */
/* rev_exit_group(int error_code) */
/* ============================== */
RevProc::ECALL_status_t RevProc::ECALL_exit_group(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit_group called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============================== */
/* rev_fdatasync(unsigned int fd) */
/* ============================== */
RevProc::ECALL_status_t RevProc::ECALL_fdatasync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fdatasync called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ========================== */
/* rev_fsync(unsigned int fd) */
/* ========================== */
RevProc::ECALL_status_t RevProc::ECALL_fsync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsync called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ============== */
/* rev_sync(void) */
/* ============== */
RevProc::ECALL_status_t RevProc::ECALL_sync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync called\n");
  return RevProc::ECALL_status_t::SUCCESS;
}

/* =================================================================== */
/*  ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags) */
/* =================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_tee(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tee called\n");
#if 0
  // commented out to remove warnings
  int fd_in      = RegFile->RV64[10];
  int fd_out     = RegFile->RV64[11];
  size_t len     = RegFile->RV64[12];
  uint64_t flags = RegFile->RV64[13];
#endif
  return RevProc::ECALL_status_t::SUCCESS;
}

/* =================================================================== */
/* int openat(int dirfd, const char *pathname, int flags, mode_t mode) */
/* =================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_openat(RevInst& inst){
  int dfd = RegFile->RV64[10];
  int filenameAddr = RegFile->RV64[11];
  int flags = RegFile->RV64[12]; /* NOTE: Unused for now */
  uint64_t mode = RegFile->RV64[13];

  RevProc::ECALL_status_t rtval = ECALL_status_t::SUCCESS;
  /*
   * NOTE: this is currently only opening files in the current directory
   *       because of some oddities in parsing the arguments & flags
   *       but this will be fixed in the near future
  */


  /* Read the filename from memory one character at a time until we find '\0' */

  if('\0' != ECALL_buf[0]){
    //We are in the middle of the string
    ECALL_string = ECALL_string + ECALL_buf[0];
    mem->ReadVal<char>(HartToExec, filenameAddr + sizeof(char)*ECALL_string.length(), &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    rtval = RevProc::ECALL_status_t::CONTINUE;
    DependencySet(HartToExec, 10, false);
  }else if(('\0' == ECALL_buf[0]) && (ECALL_string.length() > 0)) {
    //found the null terminator - we're done
    ECALL_string = ECALL_string + ECALL_buf[0];

    /* Do the openat on the host */
    dfd = open(std::filesystem::current_path().c_str(), O_RDONLY);
    int fd = openat(dfd, ECALL_string.c_str(), O_RDWR);

    HartToExecCtx()->AddFD(fd);

    /* openat returns the file descriptor of the opened file */
    RegFile->RV64[10] = fd;

    ECALL_string.clear();   //reset the ECALL buffers
    ECALL_buf[0] = '\0';
    rtval = RevProc::ECALL_status_t::SUCCESS;
    DependencyClear(HartToExec, 10, false);
  }else{
    //first time through the ECALL
    mem->ReadVal<char>(HartToExec, filenameAddr, &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    DependencySet(HartToExec, 10, false);
    rtval = RevProc::ECALL_status_t::CONTINUE;
  }

  return rtval;
}

/* =================================================== */
/* rev_read(unsigned int fd, char *buf, size_t nbytes) */
/* =================================================== */
RevProc::ECALL_status_t RevProc::ECALL_read(RevInst& inst){
  int fd = RegFile->RV64[10];
  uint64_t BufAddr = RegFile->RV64[11];
  size_t BufSize = RegFile->RV64[12];

  /* Check if Current Ctx has access to the fd */
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();

  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %d; Hart %d; PID %" PRIu32 " tried to read from file descriptor: %" PRIu64 " but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
    return RevProc::ECALL_status_t::SUCCESS;
  }
  /*
   * This buffer is an intermediate buffer for storing the data read from host 
   * for later use in writing to RevMem
  */
  char TmpBuf[BufSize];

  /*
   * Read nbytes of fd from host
   *
   * NOTE: Because the fd is in the Ctx's fildes vector, we can reasonably
   *       assume the file is already open on the host system because we 
   *       try to maintain parity between those
   */

  /* Do the read on the host */
  uint64_t rc = read(fd, &TmpBuf, BufSize);

  /* Write that data to the buffer inside of Rev */
  mem->WriteMem(feature->GetHart(), BufAddr, BufSize, &TmpBuf);

  RegFile->RV64[10] = rc;
  return RevProc::ECALL_status_t::SUCCESS;
}


/* ========================== */
/* rev_close(unsigned int fd) */
/* ========================== */
RevProc::ECALL_status_t RevProc::ECALL_close(RevInst& inst){
  int fd = RegFile->RV64[10];
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();

  /* Check if CurrCtx has fd in fildes vector */
  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %d; Hart %d; PID %" PRIu32 " tried to close file descriptor %d but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
    return RevProc::ECALL_status_t::SUCCESS;
  }
  /* Close file on host */
  uint64_t rc = close(fd);

  /* Remove from Ctx's fildes */
  CurrCtx->RemoveFD(fd);

  /* rc is propogated to rev from host */
  RegFile->RV64[10] = rc;

  return RevProc::ECALL_status_t::SUCCESS;
}

/* ====================================================================== */
/* rev_fchown(unsigned int fd, unsigned short user, unsigned short group) */         
/* ====================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_fchown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchown called"); 
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ==================================================================================== */
/* rev_fchownat(int dfd, const char *filename, unsigned user, unsigned group, int flag) */
/* ==================================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_fchownat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchownat called"); 
  return RevProc::ECALL_status_t::SUCCESS;
}

/* ======================================================================*/
/* rev_mkdirat(int dfd, const char * path, unsigned short mode)          */
/* ======================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_mkdirat(RevInst& inst){

  output->verbose(CALL_INFO, 2, 0, "ECALL_mkdirat called"); 

  unsigned fd = RegFile->RV64[10];
  unsigned Mode = RegFile->RV64[12];
  RevProc::ECALL_status_t rtval = ECALL_status_t::SUCCESS;

  if('\0' != ECALL_buf[0]){
    //We are in the middle of the string
    ECALL_string = ECALL_string + ECALL_buf[0];
    mem->ReadVal<char>(HartToExec, RegFile->RV64[11] + sizeof(char)*ECALL_string.length(), &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    rtval = RevProc::ECALL_status_t::CONTINUE;
    DependencySet(HartToExec, 10, false);
  }else if(('\0' == ECALL_buf[0]) && (ECALL_string.length() > 0)) {
    //found the null terminator - we're done
    ECALL_string = ECALL_string + ECALL_buf[0];

    /* Do the mkdirat on the host */
    const int rc = mkdirat(fd, ECALL_string.data(), Mode);
    RegFile->RV64[10] = rc;

    ECALL_string.clear();   //reset the ECALL buffers
    ECALL_buf[0] = '\0';
    rtval = RevProc::ECALL_status_t::SUCCESS;
    DependencyClear(HartToExec, 10, false);
  }else{
    //first time through the ECALL
    mem->ReadVal<char>(HartToExec, RegFile->RV64[11], &ECALL_buf[0], inst.hazard, REVMEM_FLAGS(0x00));
    DependencySet(HartToExec, 10, false);
    rtval = RevProc::ECALL_status_t::CONTINUE;
  }
/*
  std::string path = "";
  unsigned i=0;
  
  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    mem->ReadMem(RegFile->RV64[11] + sizeof(char)*i, sizeof(char), &dirchar);
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = mkdirat(fd, path.data(), Mode);
  RegFile->RV64[10] = rc;*/


  return rtval;
}

/* =========================================================== */
/* rev_dup3(unsigned int oldfd, unsigned int newfd, int flags) */
/* =========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_dup3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup3 called"); 
  return RevProc::ECALL_status_t::SUCCESS;
}


/* =========================================================== */
/* rev_dup(unsigned int fildes)                                */
/* =========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_dup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup called");
  return RevProc::ECALL_status_t::SUCCESS;
}


/*
 * This is the function that is called when an ECALL exception is detected inside ClockTick
 * - Currently the only way to set this exception is by Ext->Execute(....) an ECALL instruction
 *
 * Eventually this will be integrated into a TrapHandler however since ECALLs are the only
 * supported exceptions at this point there is no need just yet.
 */
void RevProc::ExecEcall(RevInst& inst){
  // a7 register = ecall code
  uint64_t EcallCode;
  if( feature->IsRV32() )
    EcallCode = (uint64_t)RegFile->RV32[17];
  else if( feature->IsRV64() )
    EcallCode = RegFile->RV64[17];
  else {
    return;
  }

  auto it = Ecalls.find(EcallCode);
  if( it != Ecalls.end() ){
    /* call the function */
    ECALL_status_t status = (it->second)(this, inst);
    /* Trap handled... 0 cause registers */
    RegFile->RV64_SCAUSE = status;
    RegFile->RV32_SCAUSE = status;
    //For now, rewind the PC and keep executing the ECALL until we
    // have completed 
    if(RevProc::ECALL_status_t::SUCCESS != status){
      if( feature->IsRV32() ){
        RegFile->RV32_PC -= inst.instSize;
      }else{
        RegFile->RV64_PC -= inst.instSize;
      }

    }
  } else {
    output->fatal(CALL_INFO, -1, "Ecall Code = %lu not found", EcallCode);
  }
}

uint32_t RevProc::GetActiveThreadID(){
  uint32_t ActiveThreadID = 0;
  if( HartToDecode != _REV_INVALID_HART_ID_ ){
    if( AssignedThreads.size() >= HartToDecode ){
      ActiveThreadID = AssignedThreads.at(HartToDecode)->GetThreadID();
    } 
    else {
      output->fatal(CALL_INFO, 1, "HartToDecode = %d but there are only %zu threads assigned to this Proc. This might be a bug\n", HartToDecode, AssignedThreads.size());
    }
  } 
  else {
    output->fatal(CALL_INFO, 1, "HartToDecode = %d is invalid. This is a bug\n", HartToDecode);
  }
  return ActiveThreadID;
}

// EOF
// EOF
