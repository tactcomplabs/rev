//
// _RevLoader_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevLoader.h"
#include "RevMem.h"

using namespace SST::RevCPU;
using MemSegment = RevMem::MemSegment;

RevLoader::RevLoader( std::string Exe, std::string Args,
                      RevMem *Mem, SST::Output *Output )
  : exe(Exe), args(Args), mem(Mem), output(Output),
    RV32Entry(0x00l), RV64Entry(0x00ull) {
  if( !LoadElf() )
    output->fatal(CALL_INFO, -1, "Error: failed to load executable into memory\n");
}

RevLoader::~RevLoader(){
}

bool RevLoader::IsElf( const Elf64_Ehdr eh64 ){
  if( (eh64).e_ident[0] == 0x7f &&
      (eh64).e_ident[1] == 'E'  &&
      (eh64).e_ident[2] == 'L'  &&
      (eh64).e_ident[3] == 'F' )
    return true;

  return false;
}

bool RevLoader::IsRVElf32( const Elf64_Ehdr eh64 ){
  if( IsElf(eh64) && (eh64).e_ident[4] == 1 )
    return true;
  return false;
}

bool RevLoader::IsRVElf64( const Elf64_Ehdr eh64 ){
  if( IsElf(eh64) && (eh64).e_ident[4] == 2 )
    return true;
  return false;
}

bool RevLoader::IsRVLittle( const Elf64_Ehdr eh64 ){
  if( IsElf(eh64) && (eh64).e_ident[5] == 1 )
    return true;
  return false;
}

bool RevLoader::IsRVBig( const Elf64_Ehdr eh64 ){
  if( IsElf(eh64) && (eh64).e_ident[5] == 2 )
    return true;
  return false;
}

// breaks the write into cache line chunks
bool RevLoader::WriteCacheLine(uint64_t Addr, size_t Len, void *Data){
  if( Len == 0 ){
    // nothing to do here, move along
    return true;
  }

  // calculate the cache line size
  unsigned lineSize = mem->getLineSize();
  if( lineSize == 0 ){
    // default to 64byte cache lines
    lineSize = 64;
  }

  // begin writing the data, if we have a small write
  // then dispatch the write as normal.  Otherwise,
  // block the writes as cache lines
  // #131 added case for when Len==lineSize
  if( Len <= lineSize ){
    // one cache line to write, dispatch it
    return mem->WriteMem(0, Addr, Len, Data);
  }

  // calculate the base address of the first cache line
  size_t Total = 0;
  bool done = false;
  uint64_t BaseCacheAddr = Addr;
  while( !done ){
    if( BaseCacheAddr % lineSize == 0 ){
      done = true;
    }else{
      BaseCacheAddr--;
    }
  }

  // write the first cache line
  size_t TmpSize = BaseCacheAddr + lineSize - Addr;
  uint64_t TmpData = uint64_t(Data);
  uint64_t TmpAddr = Addr;
  if( !mem->WriteMem(0, TmpAddr, TmpSize, reinterpret_cast<void*>(TmpData)) ){
    output->fatal(CALL_INFO, -1, "Error: Failed to perform cache line write\n" );
  }

  TmpAddr += TmpSize;
  TmpData += TmpSize;
  Total += TmpSize;

  // now perform the remainder of the writes
  do{
    if( (Len-Total) > lineSize ){
      // setup another full cache line write
      TmpSize = lineSize;
    }else{
      // this is probably the final write operation
      TmpSize = (Len-Total);
    }

    if( !mem->WriteMem(0, TmpAddr, TmpSize, reinterpret_cast<void*>(TmpData)) ){
      output->fatal(CALL_INFO, -1, "Error: Failed to perform cache line write\n" );
    }

    // incrememnt the temp counters
    TmpAddr += TmpSize;
    TmpData += TmpSize;
    Total += TmpSize;

  }while( Total < Len );

  return true;
}

// Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym, from_le
bool RevLoader::LoadElf32(char *membuf, size_t sz){
  // Parse the ELF header
  Elf32_Ehdr *eh = (Elf32_Ehdr *)(membuf);

  // Parse the program headers
  Elf32_Phdr *ph = (Elf32_Phdr *)(membuf + eh->e_phoff);

  // Parse the section headers
  Elf32_Shdr* sh = (Elf32_Shdr*)(membuf + eh->e_shoff);
  char *shstrtab = membuf + sh[eh->e_shstrndx].sh_offset;

  // Store the entry point of the program
  RV32Entry = eh->e_entry;

  // Add memory segments for each program header
  for (unsigned i = 0; i < eh->e_phnum; i++) {
    if( sz < ph[i].p_offset + ph[i].p_filesz ){
      output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
    }
    // Check if the program header is PT_TLS
    // - If so, save the addr & size of the TLS segment
    if( ph[i].p_type == PT_TLS ){
      TLSBaseAddr = ph[i].p_paddr;
      TLSSize = ph[i].p_memsz;
      mem->SetTLSInfo(ph[i].p_paddr, ph[i].p_memsz);
    }

    // Add a memory segment for the program header
    if( ph[i].p_memsz ){
      mem->AddRoundedMemSeg(ph[i].p_paddr, ph[i].p_memsz, __PAGE_SIZE__);
    }
  }

  mem->AddThreadMem();

  // Add memory segments for each program header
  for (unsigned i = 0; i < eh->e_phnum; i++) {
    if( sz < ph[i].p_offset + ph[i].p_filesz ){
      output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
    }
    // Add a memory segment for the program header
    if( ph[i].p_memsz ){
      mem->AddRoundedMemSeg(ph[i].p_paddr, ph[i].p_memsz, __PAGE_SIZE__);
    }
  }

  uint64_t StaticDataEnd = 0;
  uint64_t BSSEnd = 0;
  uint64_t DataEnd = 0;
  uint64_t TextEnd = 0;
  for (unsigned i = 0; i < eh->e_shnum; i++) {
    // check if the section header name is bss
    if( strcmp(shstrtab + sh[i].sh_name, ".bss") == 0 ){
      BSSEnd = sh[i].sh_addr + sh[i].sh_size;
    }
    if( strcmp(shstrtab + sh[i].sh_name, ".text") == 0 ){
      TextEnd = sh[i].sh_addr + sh[i].sh_size;
    }
    if( strcmp(shstrtab + sh[i].sh_name, ".data") == 0 ){
      DataEnd = sh[i].sh_addr + sh[i].sh_size;
    }
  }
  // If BSS exists, static data ends after it
  if( BSSEnd > 0 ){
    StaticDataEnd = BSSEnd;
  } else if ( DataEnd > 0 ){
    // BSS Doesn't exist, but data does
    StaticDataEnd = DataEnd;
  } else if ( TextEnd > 0 ){
    // Text is last resort
    StaticDataEnd = TextEnd;
  } else {
    // Can't find any (Text, BSS, or Data) sections
    output->fatal(CALL_INFO, -1, "Error: No text, data, or bss sections --- RV64 Elf is unrecognizable\n" );
  }

  // Check that the ELF file is valid
  if( sz < eh->e_phoff + eh->e_phnum * sizeof(*ph) )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  // Write the program headers to memory
  elfinfo.phnum = eh->e_phnum;
  elfinfo.phent = sizeof(Elf32_Phdr);
  elfinfo.phdr  = eh->e_phoff;
  elfinfo.phdr_size = eh->e_phnum * sizeof(Elf32_Phdr);

  // set the first stack pointer
  uint32_t sp = mem->GetStackTop() - (uint32_t)(elfinfo.phdr_size);
  WriteCacheLine(sp, elfinfo.phdr_size, ph);
  mem->SetStackTop(sp);

  // iterate over the program headers
  for( unsigned i=0; i<eh->e_phnum; i++ ){
    // Look for the loadable program headers
    if( ph[i].p_type == PT_LOAD && ph[i].p_memsz ){
      if( ph[i].p_filesz ){
        if( sz < ph[i].p_offset + ph[i].p_filesz ){
          output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
        }
        WriteCacheLine(ph[i].p_paddr,
                       ph[i].p_filesz,
                       (uint8_t*)(membuf+ph[i].p_offset));
      }
      std::vector<uint8_t> zeros(ph[i].p_memsz - ph[i].p_filesz);
      WriteCacheLine(ph[i].p_paddr + ph[i].p_filesz,
                     ph[i].p_memsz - ph[i].p_filesz,
                     &zeros[0]);
    }
  }

  // Check that the ELF file is valid
  if( sz < eh->e_shoff + eh->e_shnum * sizeof(*sh) )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  if( eh->e_shstrndx >= eh->e_shnum )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  if( sz < sh[eh->e_shstrndx].sh_offset + sh[eh->e_shstrndx].sh_size )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  unsigned strtabidx = 0;
  unsigned symtabidx = 0;

  // Iterate over every section header
  for( unsigned i=0; i<eh->e_shnum; i++ ){
    // If the section header is empty, skip it
    if( sh[i].sh_type & SHT_NOBITS )
      continue;
    if( sz < sh[i].sh_offset + sh[i].sh_size )
      output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
    // Find the string table index
    if( strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0 )
      strtabidx = i;
    // Find the symbol table index
    if( strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0 )
      symtabidx = i;
  }

  // If the string table index and symbol table index are valid (NonZero)
  if( strtabidx && symtabidx ){
    // If there is a string table and symbol table, add them as valid memory
    // Parse the string table
    char *strtab = membuf + sh[strtabidx].sh_offset;
    Elf32_Sym* sym = (Elf32_Sym*)(membuf + sh[symtabidx].sh_offset);
    // Iterate over every symbol in the symbol table
    for( unsigned i=0; i<sh[symtabidx].sh_size/sizeof(Elf32_Sym); i++ ){
      // Calculate the maximum length of the symbol
      unsigned maxlen = sh[strtabidx].sh_size - sym[i].st_name;
      if( sym[i].st_name >= sh[strtabidx].sh_size )
        output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
      if( strnlen(strtab + sym[i].st_name, maxlen) >= maxlen )
        output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
      // Add the symbol to the symbol table
      symtable[strtab+sym[i].st_name] = sym[i].st_value;
    }
  }

  // Initialize the heap
  mem->InitHeap(StaticDataEnd);

  return true;
}

bool RevLoader::LoadElf64(char *membuf, size_t sz){
  // Parse the ELF header
  Elf64_Ehdr *eh = (Elf64_Ehdr *)(membuf);

  // Parse the program headers
  Elf64_Phdr *ph = (Elf64_Phdr *)(membuf + eh->e_phoff);

  // Parse the section headers
  Elf64_Shdr* sh = (Elf64_Shdr*)(membuf + eh->e_shoff);
  char *shstrtab = membuf + sh[eh->e_shstrndx].sh_offset;

  // Store the entry point of the program
  RV64Entry = eh->e_entry;

  // Add memory segments for each program header
  for (unsigned i = 0; i < eh->e_phnum; i++) {
    if( sz < ph[i].p_offset + ph[i].p_filesz ){
      output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
    }
    // Check if the program header is PT_TLS
    // - If so, save the addr & size of the TLS segment
    if( ph[i].p_type == PT_TLS ){
      TLSBaseAddr = ph[i].p_paddr;
      TLSSize = ph[i].p_memsz;
      mem->SetTLSInfo(ph[i].p_paddr, ph[i].p_memsz);
    }

    // Add a memory segment for the program header
    if( ph[i].p_memsz ){
      mem->AddRoundedMemSeg(ph[i].p_paddr, ph[i].p_memsz, __PAGE_SIZE__);
    }
  }

  // Add the first thread's memory
  mem->AddThreadMem();

  uint64_t StaticDataEnd = 0;
  uint64_t BSSEnd = 0;
  uint64_t DataEnd = 0;
  uint64_t TextEnd = 0;
  for (unsigned i = 0; i < eh->e_shnum; i++) {
    // check if the section header name is bss
    if( strcmp(shstrtab + sh[i].sh_name, ".bss") == 0 ){
      BSSEnd = sh[i].sh_addr + sh[i].sh_size;
    }
    if( strcmp(shstrtab + sh[i].sh_name, ".text") == 0 ){
      TextEnd = sh[i].sh_addr + sh[i].sh_size;
    }
    if( strcmp(shstrtab + sh[i].sh_name, ".data") == 0 ){
      DataEnd = sh[i].sh_addr + sh[i].sh_size;
    }
  }
  // If BSS exists, static data ends after it
  if( BSSEnd > 0 ){
    StaticDataEnd = BSSEnd;
  } else if ( DataEnd > 0 ){
    // BSS Doesn't exist, but data does
    StaticDataEnd = DataEnd;
  } else if ( TextEnd > 0 ){
    // Text is last resort
    StaticDataEnd = TextEnd;
  } else {
    // Can't find any (Text, BSS, or Data) sections
    output->fatal(CALL_INFO, -1, "Error: No text, data, or bss sections --- RV64 Elf is unrecognizable\n" );
  }

  // Check that the ELF file is valid
  if( sz < eh->e_phoff + eh->e_phnum * sizeof(*ph) )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  // Write the program headers to memory
  elfinfo.phnum = eh->e_phnum;
  elfinfo.phent = sizeof(Elf64_Phdr);
  elfinfo.phdr  = eh->e_phoff;
  elfinfo.phdr_size = eh->e_phnum * sizeof(Elf64_Phdr);

  // set the first stack pointer
  uint64_t sp = mem->GetStackTop() - elfinfo.phdr_size;
  WriteCacheLine(sp, elfinfo.phdr_size, ph);
  mem->SetStackTop(sp);

  // iterate over the program headers
  for( unsigned i=0; i<eh->e_phnum; i++ ){
    // Look for the loadable headers
    if( ph[i].p_type == PT_LOAD && ph[i].p_memsz ){
      if( ph[i].p_filesz ){
        if( sz < ph[i].p_offset + ph[i].p_filesz ){
          output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
        }
        WriteCacheLine(ph[i].p_paddr,
                       ph[i].p_filesz,
                       (uint8_t*)(membuf+ph[i].p_offset));
      }
      std::vector<uint8_t> zeros(ph[i].p_memsz - ph[i].p_filesz);
      WriteCacheLine(ph[i].p_paddr + ph[i].p_filesz,
                     ph[i].p_memsz - ph[i].p_filesz,
                     &zeros[0]);
    }
  }

  // Check that the ELF file is valid
  if( sz < eh->e_shoff + eh->e_shnum * sizeof(*sh) )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  if( eh->e_shstrndx >= eh->e_shnum )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  if( sz < sh[eh->e_shstrndx].sh_offset + sh[eh->e_shstrndx].sh_size )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  unsigned strtabidx = 0;
  unsigned symtabidx = 0;

  // Iterate over every section header
  for( unsigned i=0; i<eh->e_shnum; i++ ){
    // If the section header is empty, skip it
    if( sh[i].sh_type & SHT_NOBITS ){
      continue;
    }
    if( sz < sh[i].sh_offset + sh[i].sh_size ){
      output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
    }
    // Find the string table index
    if( strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0 )
      strtabidx = i;
    // Find the symbol table index
    if( strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0 )
      symtabidx = i;
  }

  // If the string table index and symbol table index are valid (NonZero)
  if( strtabidx && symtabidx ){
    // Parse the string table
    char *strtab = membuf + sh[strtabidx].sh_offset;
    Elf64_Sym* sym = (Elf64_Sym*)(membuf + sh[symtabidx].sh_offset);
    // Iterate over every symbol in the symbol table
    for( unsigned i=0; i<sh[symtabidx].sh_size/sizeof(Elf64_Sym); i++ ){
      // Calculate the maximum length of the symbol
      unsigned maxlen = sh[strtabidx].sh_size - sym[i].st_name;
      if( sym[i].st_name >= sh[strtabidx].sh_size )
        output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
      if( strnlen(strtab + sym[i].st_name, maxlen) >= maxlen )
        output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
      // Add the symbol to the symbol table
      symtable[strtab+sym[i].st_name] = sym[i].st_value;
    }
  }

  // Initialize the heap
  mem->InitHeap(StaticDataEnd);

  return true;
}

std::string RevLoader::GetArgv(unsigned entry){
  if( entry > (argv.size()-1) )
    return "";

  return argv[entry];
}

bool RevLoader::LoadProgramArgs(){
  // -------------- BEGIN MEMORY LAYOUT NOTES
  // At this point in the code, the loader has initialized .text, .bss, .sbss, etc
  // We seek to utilize the argument array information that is passed in
  // from the user (see parameter="args") in order to initialize the arguments to 'main'
  // For example, people often implement:
  //        int main( int argc, char **argv)
  //
  // This function builds the necessary information for the input arguments
  // and writes this information to memory
  //
  // There are two things we know:
  // 1) StackTop is the current top of the stack initialized by the loader (not RevMem)
  // 2) MemTop is the theoretical "top" of memory (eg, the highest possible virtual address) set by RevMem
  //
  // These data elements are written to the highest 1024 bytes of memory, or:
  // [MemTop] --> [Memtop-1024]
  //
  // This is addressable memory, but exists BEYOND the top of the standard stack
  // The data is written in two parts.  First, we write the ARGV strings (null terminated)
  // in reverse order into the most significant addresses as follows:
  //
  // [MemTop]
  //    ARGV[ARGC-1]
  //    ARGV[ARGC-2]
  //    ....
  //    ARGV[0] (also known as the executable name)
  //
  // Note that the addresses are written contiguously
  //
  // Next, we setup the prologue header just above the base stack pointer
  // that contains the layout of this array.  This includes our ARGC value
  // We start this block of memory at SP+48 bytes as follows:
  //
  // [SP+48] : ARGC
  // [SP+52] : POINTER TO A MEMORY LOCATION THAT CONTAINS THE BASE ADDRESS OF *ARGV[0]
  // [SP+60] : POINTER TO ARGV[0]
  // [SP+68] : POINTER TO ARGV[1]
  // [SP+72] : ...
  //
  // Finally, when the processor comes out of Reset() (see RevProc.cc), we initialize
  // the x10 register to the value of ARGC and the x11 register to the base pointer to ARGV
  // -------------- END MEMORY LAYOUT NOTES


  // argv[0] = program name
  argv.push_back(exe);

  // split the rest of the arguments into tokens
  splitStr(args, ' ', argv);

  if( argv.size() == 0 ){
    output->fatal(CALL_INFO, -1, "Error: failed to initialize the program arguments\n");
    return false;
  }

  uint64_t OldStackTop = mem->GetMemTop();
  uint64_t ArgArray = mem->GetStackTop()+48;

  // setup the argc argument values
  uint32_t Argc = (uint32_t)(argv.size());
  WriteCacheLine(ArgArray, 4, &Argc);
  ArgArray += 4;

  // write the argument values
  for( int i=(argv.size()-1); i >= 0; i-- ){
    output->verbose(CALL_INFO, 6, 0,
                    "Loading program argv[%d] = %s\n", i, argv[i].c_str() );

    // retrieve the current stack pointer
    std::vector<char> tmpc(argv[i].size() + 1);
    argv[i].copy(&tmpc[0], argv[i].size()+1);
    tmpc[argv[i].size()] = '\0';
    size_t len = argv[i].size() + 1;

    OldStackTop -= len;

    WriteCacheLine(OldStackTop, len, &tmpc[0]);
  }

  // now reverse engineer the address alignments
  // -- this is the address of the argv pointers (address + 8) in the stack
  // -- Note: this is NOT the actual addresses of the argv[n]'s
  uint64_t ArgBase = ArgArray+8;
  WriteCacheLine(ArgArray, 8, &ArgBase);
  ArgArray += 8;

  // -- these are the addresses of each argv entry
  for( size_t i=0; i<argv.size(); i++ ){
    WriteCacheLine(ArgArray, 8, &OldStackTop);
    OldStackTop += (argv[i].size()+1);
    ArgArray += 8;
  }

  mem->SetNextThreadMemAddr(OldStackTop - _STACK_SIZE_ - mem->GetTLSSize() - __PAGE_SIZE__);
  return true;
}

void RevLoader::splitStr(const std::string& s,
                         char c,
                         std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);

  if( (j==std::string::npos) && (s.length() > 0) ){
    v.push_back(s);
    return ;
  }

  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }

}

bool RevLoader::LoadElf(){
  // open the target file
  int fd = open(exe.c_str(), O_RDONLY);
  struct stat FileStats;
  if( fstat(fd, &FileStats) < 0 )
    output->fatal(CALL_INFO, -1, "Error: failed to stat executable file: %s\n", exe.c_str() );

  size_t FileSize = FileStats.st_size;

  // map the executable into memory
  char *membuf = (char *)(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, fd, 0));
  if( membuf == MAP_FAILED )
    output->fatal(CALL_INFO, -1, "Error: failed to map executable file: %s\n", exe.c_str() );

  // close the target file
  close(fd);

  // check the size of the elf header
  if( FileSize < sizeof(Elf64_Ehdr) )
    output->fatal(CALL_INFO, -1, "Error: Elf header is unrecognizable\n" );

  const Elf64_Ehdr* eh64 = (const Elf64_Ehdr*)(membuf);
  if( !IsRVElf32(*eh64) && !IsRVElf64(*eh64) )
    output->fatal(CALL_INFO, -1, "Error: Cannot determine Elf32 or Elf64 from header\n" );

  if( !IsRVLittle(*eh64) )
    output->fatal(CALL_INFO, -1, "Error: Not in little endian format\n" );

  if( IsRVElf32(*eh64) ){
    if( !LoadElf32(membuf, FileSize) )
      output->fatal(CALL_INFO, -1, "Error: could not load Elf32 binary\n" );
  }else{
    if( !LoadElf64(membuf, FileSize) )
      output->fatal(CALL_INFO, -1, "Error: could not load Elf64 binary\n" );
  }

  // unmap the file
  munmap( membuf, FileSize );

  // print the symbol table entries
  std::map<std::string, uint64_t>::iterator it = symtable.begin();
  while( it != symtable.end() ){
    // create inverse map to allow tracer to lookup symbols
    tracer_symbols.emplace(it->second, it->first);
    output->verbose(CALL_INFO, 6, 0,
                    "Symbol Table Entry [%s:0x%" PRIx64 "]\n",
                    it->first.c_str(), it->second );
    it++;
  }

  /// load the program arguments
  if( !LoadProgramArgs() )
    return false;

  // Initiate a memory fence in order to ensure that the entire ELF
  // infrastructure is loaded
  mem->FenceMem(0);

  return true;
}

uint64_t RevLoader::GetSymbolAddr(std::string Symbol){
  uint64_t tmp = 0x00ull;
  if( symtable.find(Symbol) != symtable.end() ){
    tmp = symtable[Symbol];
  }
  return tmp;
}

std::map<uint64_t, std::string> *SST::RevCPU::RevLoader::GetTraceSymbols()
{
    return &tracer_symbols;
}

// EOF
