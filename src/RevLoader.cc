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

// Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym, from_le
bool RevLoader::LoadElf32(char *membuf, size_t sz){
  std::vector<uint8_t> zeros;
  Elf32_Ehdr *eh = (Elf32_Ehdr *)(membuf);
  Elf32_Phdr *ph = (Elf32_Phdr *)(membuf + eh->e_phoff);
  RV32Entry = eh->e_entry;
  if( sz < eh->e_phoff + eh->e_phnum * sizeof(*ph) )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  // write the program header
  elfinfo.phnum = eh->e_phnum;
  elfinfo.phent = sizeof(Elf32_Phdr);
  elfinfo.phdr  = eh->e_phoff;
  elfinfo.phdr_size = eh->e_phnum * sizeof(Elf32_Phdr);
  uint64_t sp = mem->GetStackTop() - (uint64_t)(elfinfo.phdr_size);
  mem->WriteMem(sp,elfinfo.phdr_size,(void *)(ph),REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE));
  mem->SetStackTop(sp);

  for( unsigned i=0; i<eh->e_phnum; i++ ){
    if( ph[i].p_type == PT_LOAD && ph[i].p_memsz ){
      if( ph[i].p_filesz ){
        if( sz < ph[i].p_offset + ph[i].p_filesz )
          output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
        mem->WriteMem(ph[i].p_paddr,
                      ph[i].p_filesz,
                      (uint8_t*)(membuf+ph[i].p_offset),
                      REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE));
      }
      zeros.resize(ph[i].p_memsz - ph[i].p_filesz);
      mem->WriteMem(ph[i].p_paddr + ph[i].p_filesz,
                    ph[i].p_memsz - ph[i].p_filesz,
                    &zeros[0],
                    REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE) );
    }
  }

  Elf32_Shdr* sh = (Elf32_Shdr*)(membuf + eh->e_shoff);
  if( sz < eh->e_shoff + eh->e_shnum * sizeof(*sh) )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  if( eh->e_shstrndx >= eh->e_shnum )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  if( sz < sh[eh->e_shstrndx].sh_offset + sh[eh->e_shstrndx].sh_size )
    output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );

  char *shstrtab = membuf + sh[eh->e_shstrndx].sh_offset;
  unsigned strtabidx = 0;
  unsigned symtabidx = 0;

  for( unsigned i=0; i<eh->e_shnum; i++ ){
    unsigned maxlen = sh[eh->e_shstrndx].sh_size - sh[i].sh_name;
    if( sh[i].sh_name >= sh[eh->e_shstrndx].sh_size )
      output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
    if( strnlen(shstrtab + sh[i].sh_name, maxlen) >= maxlen )
      output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
    if( sh[i].sh_type & SHT_NOBITS )
      continue;
    if( sz < sh[i].sh_offset + sh[i].sh_size )
      output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
    if( strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0 )
      strtabidx = i;
    if( strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0 )
      symtabidx = i;
  }

  if( strtabidx && symtabidx ){
    char *strtab = membuf + sh[strtabidx].sh_offset;
    Elf32_Sym* sym = (Elf32_Sym*)(membuf + sh[symtabidx].sh_offset);
    for( unsigned i=0; i<sh[symtabidx].sh_size/sizeof(Elf32_Sym); i++ ){
      unsigned maxlen = sh[strtabidx].sh_size - sym[i].st_name;
      if( sym[i].st_name >= sh[strtabidx].sh_size )
        output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
      if( strnlen(strtab + sym[i].st_name, maxlen) >= maxlen )
        output->fatal(CALL_INFO, -1, "Error: RV32 Elf is unrecognizable\n" );
      symtable[strtab+sym[i].st_name] = sym[i].st_value;
    }
  }

  return true;
}

bool RevLoader::LoadElf64(char *membuf, size_t sz){
  std::vector<uint8_t> zeros;
  Elf64_Ehdr *eh = (Elf64_Ehdr *)(membuf);
  Elf64_Phdr *ph = (Elf64_Phdr *)(membuf + eh->e_phoff);
  RV64Entry = eh->e_entry;
  if( sz < eh->e_phoff + eh->e_phnum * sizeof(*ph) )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );


  // write the program header
  elfinfo.phnum = eh->e_phnum;
  elfinfo.phent = sizeof(Elf64_Phdr);
  elfinfo.phdr  = eh->e_phoff;
  elfinfo.phdr_size = eh->e_phnum * sizeof(Elf64_Phdr);
  uint64_t sp = mem->GetStackTop() - (uint64_t)(elfinfo.phdr_size);
  mem->WriteMem(sp,elfinfo.phdr_size,(void *)(ph),REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE));
  mem->SetStackTop(sp);


  for( unsigned i=0; i<eh->e_phnum; i++ ){
    if( ph[i].p_type == PT_LOAD && ph[i].p_memsz ){
      if( ph[i].p_filesz ){
        if( sz < ph[i].p_offset + ph[i].p_filesz )
          output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
        mem->WriteMem(ph[i].p_paddr,
                      ph[i].p_filesz,
                      (uint8_t*)(membuf+ph[i].p_offset),
                      REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE));
      }
      zeros.resize(ph[i].p_memsz - ph[i].p_filesz);
      mem->WriteMem(ph[i].p_paddr + ph[i].p_filesz,
                    ph[i].p_memsz - ph[i].p_filesz,
                    &zeros[0],
                    REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE) );
    }
  }

  Elf64_Shdr* sh = (Elf64_Shdr*)(membuf + eh->e_shoff);
  if( sz < eh->e_shoff + eh->e_shnum * sizeof(*sh) )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  if( eh->e_shstrndx >= eh->e_shnum )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  if( sz < sh[eh->e_shstrndx].sh_offset + sh[eh->e_shstrndx].sh_size )
    output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );

  char *shstrtab = membuf + sh[eh->e_shstrndx].sh_offset;
  unsigned strtabidx = 0;
  unsigned symtabidx = 0;

  for( unsigned i=0; i<eh->e_shnum; i++ ){
    unsigned maxlen = sh[eh->e_shstrndx].sh_size - sh[i].sh_name;
    if( sh[i].sh_type & SHT_NOBITS )
      continue;
    if( sz < sh[i].sh_offset + sh[i].sh_size )
      output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
    if( strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0 )
      strtabidx = i;
    if( strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0 )
      symtabidx = i;
  }

  if( strtabidx && symtabidx ){
    char *strtab = membuf + sh[strtabidx].sh_offset;
    Elf64_Sym* sym = (Elf64_Sym*)(membuf + sh[symtabidx].sh_offset);
    for( unsigned i=0; i<sh[symtabidx].sh_size/sizeof(Elf64_Sym); i++ ){
      unsigned maxlen = sh[strtabidx].sh_size - sym[i].st_name;
      if( sym[i].st_name >= sh[strtabidx].sh_size )
        output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
      if( strnlen(strtab + sym[i].st_name, maxlen) >= maxlen )
        output->fatal(CALL_INFO, -1, "Error: RV64 Elf is unrecognizable\n" );
      symtable[strtab+sym[i].st_name] = sym[i].st_value;
    }
  }

  return true;
}

std::string RevLoader::GetArgv(unsigned entry){
  if( entry > (argv.size()-1) )
    return "";

  return argv[entry];
}

bool RevLoader::LoadProgramArgs(){
  // argv[0] = program name
  argv.push_back(exe);

  // split the rest of the arguments into tokens
  splitStr(args,' ',argv);

  if( argv.size() == 0 ){
    output->fatal(CALL_INFO, -1, "Error: failed to initialize the program arguments\n");
    return false;
  }

  // load the program args into memory
  uint64_t sp = 0x00ull;
  for( unsigned i=0; i<argv.size(); i++ ){
    output->verbose(CALL_INFO,6,0,
                    "Loading program argv[%d] = %s\n", i, argv[i].c_str() );
    sp = mem->GetStackTop();
    char tmpc[argv[i].size() + 1];
    argv[i].copy(tmpc,argv[i].size()+1);
    tmpc[argv[i].size()] = '\0';
    size_t len = argv[i].size() + 1;
    mem->SetStackTop(sp-(uint64_t)(len));
    mem->WriteMem(mem->GetStackTop(),len,(void *)(&tmpc),REVMEM_FLAGS(RevCPU::RevFlag::F_NONCACHEABLE));
  }

  return true;
}

void RevLoader::splitStr(const std::string& s,
                         char c,
                         std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);

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
  if( fstat(fd,&FileStats) < 0 )
    output->fatal(CALL_INFO, -1, "Error: failed to stat executable file: %s\n", exe.c_str() );

  size_t FileSize = FileStats.st_size;

  // map the executable into memory
  char *membuf = (char *)(mmap(NULL,FileSize, PROT_READ, MAP_PRIVATE, fd, 0));
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
    if( !LoadElf32(membuf,FileSize) )
      output->fatal(CALL_INFO, -1, "Error: could not load Elf32 binary\n" );
  }else{
    if( !LoadElf64(membuf,FileSize) )
      output->fatal(CALL_INFO, -1, "Error: could not load Elf64 binary\n" );
  }

  // unmap the file
  munmap( membuf, FileSize );

  // print the symbol table entries
  std::map<std::string,uint64_t>::iterator it = symtable.begin();
  while( it != symtable.end() ){
    output->verbose(CALL_INFO,6,0,
                    "Symbol Table Entry [%s:0x%" PRIx64 "]\n",
                    it->first.c_str(), it->second );
    it++;
  }

  /// load the program arguments
  if( !LoadProgramArgs() )
    return false;

  // Initiate a memory fence in order to ensure that the entire ELF
  // infrastructure is loaded
  mem->FenceMem();

  return true;
}

uint64_t RevLoader::GetSymbolAddr(std::string Symbol){
  uint64_t tmp = 0x00ull;
  if( symtable.find(Symbol) != symtable.end() ){
    tmp = symtable[Symbol];
  }
  return tmp;
}

// EOF
