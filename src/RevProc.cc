//
// _RevProc_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevProc.h"

RevProc::RevProc( unsigned Id,
                  RevOpts *Opts,
                  RevMem *Mem,
                  RevLoader *Loader,
                  SST::Output *Output )
  : Halted(false), SingleStep(false),
    CrackFault(false), ALUFault(false), fault_width(0),
    id(Id), opts(Opts), mem(Mem), loader(Loader), output(Output), threadToDecode(0),
    threadToExec(0) {

  // initialize the machine model for the target core
  std::string Machine;
  if( !Opts->GetMachineModel(id,Machine) )
    output->fatal(CALL_INFO, -1, "Error: failed to retrieve the machine model for core=%d\n", id);

  unsigned MinCost = 0;
  unsigned MaxCost = 0;

  Opts->GetMemCost(Id,MinCost,MaxCost);

  feature = new RevFeature(Machine,output,MinCost,MaxCost,Id);
  if( !feature )
    output->fatal(CALL_INFO, -1, "Error: failed to create the RevFeature object for core=%d\n", id);

  // load the instruction tables
  if( !LoadInstructionTable() )
    output->fatal(CALL_INFO, -1, "Error : failed to load instruction table for core=%d\n", id );

  //RegFile = new RevRegFile[_REV_THREAD_COUNT_];

  // reset the core
  if( !Reset() )
    output->fatal(CALL_INFO, -1, "Error: failed to reset the core resources for core=%d\n", id );

  /*CyclesWithIssue 	  = registerStatistic<uint64_t>( "cycles_with_issue" );
	CyclesWithoutIssue 	  =registerStatistic<uint64_t>( "cycles_no_issue" );
  Cycles                =registerStatistic<uint64_t>( "cycles" );*/
  totalCycles = 0;
  cyclesBusy = 0;
  cyclesIdle = 0;
  percentEff = 0.0;
  floatsExec = 0;
}

RevProc::~RevProc(){
  for( unsigned i=0; i<Extensions.size(); i++ )
    delete Extensions[i];
  delete feature;
  //delete [] RegFile;
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

bool RevProc::EnableExt(RevExt *Ext){
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
  for( unsigned i=0; i<IT.size(); i++ ){
    InstTable.push_back(IT[i]);
    std::pair<unsigned,unsigned> ExtObj =
      std::pair<unsigned,unsigned>(Extensions.size()-1,i);
    EntryToExt.insert(
      std::pair<unsigned,
        std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
  }

  return true;
}

bool RevProc::SeedInstTable(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Seeding instruction table for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  // I-Extension
  if( feature->IsModeEnabled(RV_I) ){
    EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64I(feature,RegFile,mem,output)));
    }
  }

  // M-Extension
  if( feature->IsModeEnabled(RV_M) ){
    EnableExt(static_cast<RevExt *>(new RV32M(feature,RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64M(feature,RegFile,mem,output)));
    }
  }

  // A-Extension
  if( feature->IsModeEnabled(RV_A) ){
    EnableExt(static_cast<RevExt *>(new RV32A(feature,RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64A(feature,RegFile,mem,output)));
    }
  }

  // F-Extension
  if( feature->IsModeEnabled(RV_F) ){
    EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)));
    }
  }

  // D-Extension
  if( feature->IsModeEnabled(RV_D) ){
    EnableExt(static_cast<RevExt *>(new RV32D(feature,RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)));
    }
  }

  // PAN Extension
  if( feature->IsModeEnabled(RV_P) ){
    //if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64P(feature,RegFile,mem,output)));
    //}else{
      // FIXME
      //output->fatal(CALL_INFO, -1, "Error: PAN can only be enabled on RV64");
    //}
  }

  // C-Extension
  if( feature->IsModeEnabled(RV_C) ){
    output->fatal(CALL_INFO, -1, "Error: compressed encodings enabled for core=%d not currently supported\n", id);
  }

  return true;
}

uint32_t RevProc::CompressEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= (uint32_t)(Entry.opcode);
  Value |= (uint32_t)((uint32_t)(Entry.funct3)<<8);
  Value |= (uint32_t)((uint32_t)(Entry.funct7)<<11);
  Value |= (uint32_t)((uint32_t)(Entry.imm12)<<18);

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
    EncToEntry.insert(
      std::pair<uint32_t,unsigned>(CompressEncoding(InstTable[i]),i) );
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Table Entry %d = %s\n",
                    id,
                    CompressEncoding(InstTable[i]),
                    ExtractMnemonic(InstTable[i]).c_str() );
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
  // reset the register file
  for (int t=0;  t < _REV_THREAD_COUNT_; t++){
    RegFile[t].RV32_PC = 0x00l;
    RegFile[t].RV64_PC = 0x00ull;
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      RegFile[t].RV32[i] = 0x00l;
      RegFile[t].RV64[i] = 0x00ull;
      RegFile[t].SPF[i]  = 0.f;
      RegFile[t].DPF[i]  = 0.f;
    }
  


    // initialize all the relevant program registers
    // -- x2 : stack pointer
    RegFile[t].RV32[2] = (uint32_t)(mem->GetStackTop());
    RegFile[t].RV64[2] = mem->GetStackTop();

    // -- x3 : global pointer
    RegFile[t].RV32[3] = (uint32_t)(loader->GetSymbolAddr("__global_pointer$"));
    RegFile[t].RV64[3] = loader->GetSymbolAddr("__global_pointer$");

    // -- x8 : frame pointer
    RegFile[t].RV32[8] = RegFile[t].RV32[3];
    RegFile[t].RV64[8] = RegFile[t].RV64[3];

    RegFile[t].cost = 0;

  }
  // set the pc
  uint64_t StartAddr = 0x00ull;
  if( !opts->GetStartAddr( id, StartAddr ) )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to init the start address for core=%d\n", id);
  std::string StartSymbol = "main";
  if( StartAddr == 0x00ull ){
    if( !opts->GetStartSymbol( id, StartSymbol ) )
      output->fatal(CALL_INFO, -1,
                    "Error: failed to init the start symbol address for core=%d\n", id);

    StartAddr = loader->GetSymbolAddr(StartSymbol);
  }
  if( StartAddr == 0x00ull ){
    // load "main" symbol
    StartAddr = loader->GetSymbolAddr("main");
    if( StartAddr == 0x00ull ){
      output->fatal(CALL_INFO, -1,
                    "Error: failed to auto discover address for <main> for core=%d\n", id);
    }
  }
  for (int t=0;  t < _REV_THREAD_COUNT_; t++){
    RegFile[t].RV32_PC = (uint32_t)(StartAddr);
    RegFile[t].RV64_PC = StartAddr;
  }
  THREAD_CTS.set();

  return true;
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

RevInst RevProc::DecodeRInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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
  DInst.imm     = 0x0;

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

  return DInst;
}

RevInst RevProc::DecodeIInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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

  return DInst;
}

RevInst RevProc::DecodeSInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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

  return DInst;
}

RevInst RevProc::DecodeUInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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

  return DInst;
}

RevInst RevProc::DecodeBInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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
  //DInst.imm     = twos_compl((DECODE_RD(Inst) | (DECODE_FUNCT7(Inst)<<5)),12);
  DInst.imm =   (uint32_t)((Inst << 4)&0b100000000000)|   // [11]
                (uint32_t)((Inst & 0b111100000000)>>7)|   // [4:1]
                (uint32_t)((Inst >> 20)&0b11111100000)|   // [10:5]
                (uint32_t)((Inst >> 20)&0b1000000000000);  // [12]

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  return DInst;
}

RevInst RevProc::DecodeJInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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

  return DInst;
}

RevInst RevProc::DecodeR4Inst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile[threadToDecode].cost  = InstTable[Entry].cost;

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

  return DInst;
}

bool RevProc::DebugReadReg(unsigned Idx, uint64_t *Value){
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    *Value = RegFile[threadToExec].RV32[Idx];
    return true;
  }else{
    *Value = RegFile[threadToExec].RV64[Idx];
    return true;
  }
}

bool RevProc::DebugWriteReg(unsigned Idx, uint64_t Value){
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    RegFile[threadToExec].RV32[Idx] = (uint32_t)(Value&0xFFFFFFFF);
    return true;
  }else{
    RegFile[threadToExec].RV64[Idx] = Value;
    return true;
  }
}

uint64_t RevProc::GetPC(){
  if( feature->GetXlen() == 32 ){
    return (uint64_t)(RegFile[threadToDecode].RV32_PC);
  }else{
    return RegFile[threadToDecode].RV64_PC;
  }
}

void RevProc::SetPC(uint64_t PC){
  if( feature->GetXlen() == 32 ){
    RegFile[threadToDecode].RV32_PC = (uint32_t)(PC);
  }else{
    RegFile[threadToDecode].RV64_PC = PC;
  }
}

RevInst RevProc::DecodeInst(){
  uint32_t Enc  = 0x00ul;
  uint32_t Inst = 0x00ul;
  uint64_t PC   = 0x00ull;

  // Stage 1: Retrieve the instruction
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile[threadToDecode].RV32_PC);
  }else{
    PC = RegFile[threadToDecode].RV64_PC;
  }

  if( !mem->ReadMem( PC, 4, (void *)(&Inst)) ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve instruction at PC=0x%" PRIx64 ".", PC );
  }

  output->verbose(CALL_INFO, 6, 0,
                  "Core %d ; Thread %d; PC:InstPayload = 0x%" PRIx64 ":0x%" PRIx32 "\n",
                  id, threadToDecode, PC, Inst);

  // Stage 1a: handle the crack fault injection
  if( CrackFault ){
    srand(time(NULL));
    uint64_t rval = rand() % (2^(fault_width));
    Inst |= rval;

    // clear the fault
    CrackFault = false;
  }

  // Stage 2: Retrieve the opcode
  uint32_t Opcode = (uint32_t)(Inst&0b1111111);

  // Stage 3: Determine if we have a funct3 field
  uint32_t Funct3 = 0x00ul;
  uint32_t inst42 = ((Opcode&0b11100) >> 2);
  uint32_t inst65 = ((Opcode&0b1100000) >> 5);

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

  // Stage 4: Determine if we have a funct7 field (R-Type)
  uint32_t Funct7 = 0x00ul;
  if( inst65 == 0b01 ) {
    if( (inst42 == 0b011) || (inst42 == 0b100) ){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
    }
  }else if((inst65== 0b10) && (inst42 == 0b100)){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
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

  // Stage 7: Look up the value in the table
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
    it = EncToEntry.find(Enc);
    if( it == EncToEntry.end() ){
      // failed to decode the instruction
      output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%d\n",
                  PC,
                  Enc );
    }

  }

  unsigned Entry = it->second;

  if( Entry > (InstTable.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 " \
                  Opcode = %x Funct3 = %x Funct7 = %x Imm12 = %x Enc = %x \n", \
                  PC, Opcode, Funct3, Funct7, Imm12, Enc );

  }

  RegFile[threadToDecode].Entry = Entry;

  RegFile[threadToDecode].trigger = false;

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
}

void RevProc::ResetInst(RevInst *I){
  I->opcode = 0;
  I->funct3 = 0;
  I->funct2 = 0;
  I->funct7 = 0;
  I->rd     = 0;
  I->rs1    = 0;
  I->rs2    = 0;
  I->rs3    = 0;
  I->imm    = 0;
  I->fmt    = 0;
  I->rm     = 0;
  I->aq     = 0;
  I->rl     = 0;
  I->instSize = 0;
}

void RevProc::HandleRegFault(unsigned width){
  // build the permissible set of registers available to fault
  unsigned LWidth = 0;
  std::vector<std::pair<std::string,void*>> RRegs;

  if( feature->GetXlen() == 32 ){
    if( width > feature->GetXlen() ){
      LWidth = feature->GetXlen();
    }else{
      LWidth = width;
    }

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile[threadToExec].RV32[i])));
    }
  }else{
    // rv64
    LWidth = width;

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile[threadToExec].RV64[i])));
    }
  }

  if( feature->IsModeEnabled(RV_F) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile[threadToExec].SPF[i])));
    }
  }else if( feature->IsModeEnabled(RV_D) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile[threadToExec].DPF[i])));
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

uint8_t RevProc::GetThreadID(){
  if(THREAD_CTS.none()) { return threadToDecode;};

  uint8_t nextID = threadToDecode;
  if(THREAD_CTS[threadToDecode]){
    nextID = threadToDecode;
  }else{
    for(int tID = 0; tID < _REV_THREAD_COUNT_; tID++){
      nextID++;
      if(nextID >= _REV_THREAD_COUNT_){
        nextID = 0;
      }
      if(THREAD_CTS[nextID]){ break; };
    }
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Thread switch from %d to %d \n",
                    id, threadToDecode, nextID);
  }
  return nextID;
}

bool RevProc::ClockTick( SST::Cycle_t currentCycle ){
  bool rtn = false;
  totalCycles++;

  // -- MAIN PROGRAM LOOP --
  //
  // If the clock is down to zero, then fetch the next instruction
  // else if the the instruction has not yet been triggered, execute it
  // else, wait until the counter is decremented to zero to retire the instruction
  //
  for (int tID = 0; tID < _REV_THREAD_COUNT_; tID++){
    THREAD_CTS[tID] = (RegFile[tID].cost == 0);
  }
  if( THREAD_CTS.any() && (!Halted)) { // && (RegFile[threadID].cost == 0)){
    // fetch the next instruction
    ResetInst(&Inst);

    //Determine the active thread
    threadToDecode = GetThreadID();

    // If the next instruction is our special bounce address
    // DO NOT decode it.  It will decode to a bogus instruction.
    // We do not want to retire this instruction until we're ready
    if( GetPC() != _PAN_FWARE_JUMP_ ){
      Inst = DecodeInst();
    }
    rtn = true;
    ExecPC = GetPC();
    cyclesBusy++;
  }
  if( (!RegFile[threadToDecode].trigger) && (!Halted) ){
    // trigger the next instruction
    threadToExec = threadToDecode;
    RegFile[threadToExec].trigger = true;

    // pull the PC
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Thread %d; Executing PC= 0x%" PRIx64 "\n",
                    id, threadToExec, ExecPC);

    // attempt to execute the instruction as long as it is NOT
    // the firmware jump PC
    if( ExecPC != _PAN_FWARE_JUMP_ ){

      // Find the instruction extension
      std::map<unsigned,std::pair<unsigned,unsigned>>::iterator it;
      it = EntryToExt.find(RegFile[threadToExec].Entry);
      if( it == EntryToExt.end() ){
        // failed to find the extension
        output->fatal(CALL_INFO, -1,
                    "Error: failed to find the instruction extension at PC=%" PRIx64 ".", ExecPC );
      }

      // found the instruction extension
      std::pair<unsigned,unsigned> EToE = it->second;
      RevExt *Ext = Extensions[EToE.first];

      // execute the instruction
      if( !Ext->Execute(EToE.second,Inst, threadToExec) ){
        output->fatal(CALL_INFO, -1,
                    "Error: failed to execute instruction at PC=%" PRIx64 ".", ExecPC );
      }
        if( (Ext->GetName() == "RV32F") ||
            (Ext->GetName() == "RV32D") ||
            (Ext->GetName() == "RV64F") ||
            (Ext->GetName() == "RV64D") ){
              floatsExec++;
            }

      // inject the ALU fault
      if( ALUFault ){
        // inject ALU fault
        RevExt *Ext = Extensions[EToE.first];
        if( (Ext->GetName() == "RV32F") ||
            (Ext->GetName() == "RV32D") ){
          // write an rv32 float rd
          uint32_t rval = rand() % (2^(fault_width));
          uint32_t tmp = (uint32_t)(RegFile[threadToExec].SPF[Inst.rd]);
          tmp |= rval;
          RegFile[threadToExec].SPF[Inst.rd] = (float)(tmp);
        }else if( (Ext->GetName() == "RV64F") ||
                  (Ext->GetName() == "RV64D") ){
          // write an rv64 float rd
          uint64_t rval = rand() % (2^(fault_width));
          uint64_t tmp = (uint64_t)(RegFile[threadToExec].DPF[Inst.rd]);
          tmp |= rval;
          RegFile[threadToExec].DPF[Inst.rd] = (double)(tmp);
        }else if( feature->GetXlen() == 32 ){
          // write an rv32 gpr rd
          uint32_t rval = rand() % (2^(fault_width));
          RegFile[threadToExec].RV32[Inst.rd] |= rval;
        }else{
          // write an rv64 gpr rd
          uint64_t rval = rand() % (2^(fault_width));
          RegFile[threadToExec].RV64[Inst.rd] |= rval;
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
     output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; No available thread to exec PC= 0x%" PRIx64 "\n",
                      id, ExecPC);
    rtn = true;
    cyclesIdle++;
  }

  for(int tID = 0; tID < _REV_THREAD_COUNT_; tID ++){
      if(RegFile[tID].cost > 0){
        RegFile[tID].cost = RegFile[tID].cost - 1;
        if( RegFile[tID].cost == 0 ){
            output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; ThreadID %d; Retiring PC= 0x%" PRIx64 "\n",
                      id, tID, ExecPC);
            RegFile[tID].trigger = false;
        }
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

    if( done ){
      // we are really done, return
      output->verbose(CALL_INFO,2,0,"Program execution complete\n");
      percentEff = float(cyclesBusy)/totalCycles;
      output->verbose(CALL_INFO,2,0,"Program Stats: Total Cycles: %d Busy Cycles: %d Idle Cycles: %d Eff: %f\n", totalCycles, cyclesBusy, cyclesIdle, percentEff);
      output->verbose(CALL_INFO,2,0,"\t Bytes Read: %d Bytes Written: %d Floats Read: %d Doubles Read %d  Floats Exec: %d\n", \
                                      mem->memStats.bytesRead, \
                                      mem->memStats.bytesWritten, \
                                      mem->memStats.floatsRead, \
                                      mem->memStats.doublesRead, \
                                      floatsExec);
      return false;
    }
  }

  return rtn;
}

// EOF
