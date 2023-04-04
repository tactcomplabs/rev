//
// _RevPrefetcher_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevPrefetcher.h"

bool RevPrefetcher::IsAvail(uint64_t Addr){

  // note: this logic does not consider compressed instructions
  //       however, it should work for what we need.  we only need
  //       to know whether an instruction as been filled

  uint64_t lastAddr = 0x00ull;
  for( unsigned i=0; i<baseAddr.size(); i++ ){
    lastAddr = baseAddr[i] + (depth*4);
    if( (Addr >= baseAddr[i]) && (Addr < lastAddr) ){
      // found it, fetch the address
      // first, calculate the vector offset
      uint32_t Off = 0;
      if( Addr == baseAddr[i] ){
        Off = 0;  // lets avoid division by zero
      }else{
        Off = (uint32_t)((Addr-baseAddr[i])/4);
      }
      if( Off > (depth-1) ){
        // some sort of error occurred
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ){
        // the instruction hasn't been filled yet, stall
        return false;
      }

      // the instruction is available in the stream cache
      return true;
    }
  }

  // if we reach this point, then the instruction hasn't even triggered
  // a stream prefetch.  Lets go ahead and initiate one via a 'Fill' operation
  Fill(Addr);
  return false;
}

bool RevPrefetcher::FetchUpper(uint64_t Addr, bool &Fetched, uint32_t &UInst){
  uint64_t lastAddr = 0x00ull;
  for( unsigned i=0; i<baseAddr.size(); i++ ){
    lastAddr = baseAddr[i] + (depth*4);
    if( (Addr >= baseAddr[i]) && (Addr < lastAddr) ){
      uint32_t Off = 0;
      if( Addr == baseAddr[i] ){
        Off = 0;  // lets avoid division by zero
      }else{
        Off = (uint32_t)((Addr-baseAddr[i])/4);
      }
      if( Off > (depth-1) ){
        // some sort of error occurred
        Fetched = false;
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ){
        // the instruction hasn't been filled yet, stall
        Fetched = false;
        return true;
      }

      // fetch the instruction
      if( Addr == (baseAddr[i]+(Off*4)) ){
        UInst = (iStack[i][Off]<<16);
        Fetched = true;
        return true;
      }
    }
  }

  Fill(Addr);
  Fetched = false;

  return true;
}

bool RevPrefetcher::InstFetch(uint64_t Addr, bool &Fetched, uint32_t &Inst){

  // scan the baseAddr vector to see if the address is cached
  uint64_t lastAddr = 0x00ull;
  for( unsigned i=0; i<baseAddr.size(); i++ ){
    lastAddr = baseAddr[i] + (depth*4);
    if( (Addr >= baseAddr[i]) && (Addr < lastAddr) ){
      // found it, fetch the address
      // first, calculate the vector offset
      uint32_t Off = 0;
      if( Addr == baseAddr[i] ){
        Off = 0;  // lets avoid division by zero
      }else{
        Off = (uint32_t)((Addr-baseAddr[i])/4);
      }
      if( Off > (depth-1) ){
        // some sort of error occurred
        Fetched = false;
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ){
        // the instruction hasn't been filled yet, stall
        Fetched = false;
        return true;
      }

      // fetch the instruction
      if( Addr == (baseAddr[i]+(Off*4)) ){
        Inst = iStack[i][Off];
      }else{
        // compressed instruction, adjust the offset
        Inst = (iStack[i][Off] >> 16);
        //Inst |= (iStack[i][Off+1] << 16);
        uint32_t TmpInst;
        if( !FetchUpper(Addr+2, Fetched, TmpInst) )
          return false;
        if( !Fetched ){
          // we initiated a fill
          return true;
        }
        Inst |= TmpInst;
      }

      Fetched = true;

      // if this is the last instruction in the stream buffer, we need to deallocate the stream
      if( Off == (depth-1) ){
        DeleteStream(i);
        Fill(Addr+4);   // go ahead and fill the next instruction
      }

      return true;
    }
  }

  // we missed in the stream cache, lets perform a fill
  Fill(Addr);
  Fetched = false;

  return true;
}

void RevPrefetcher::Fill(uint64_t Addr){

  // determine if the address is 32bit aligned
  if((Addr%4)!=0){
    // not 32bit aligned, adjust the base address by 2 bytes
    Fill(Addr&0xFFFFFFFFFFFFFFFC);
    return;
  }

  // allocate a new stream buffer
  baseAddr.push_back(Addr);
  iStack.push_back( new uint32_t[depth] );

  // initialize it
  unsigned x = baseAddr.size()-1;
  for( unsigned y = 0; y<depth; y++ ){
    iStack[x][y] = REVPREF_INIT_ADDR;
  }

  // now fill it
  for( unsigned y=0; y<depth; y++ ){
    mem->ReadVal( Addr+(y*4),
                  (uint32_t *)(&iStack[x][y]),
                  REVMEM_FLAGS(0x00) );
  }
}

void RevPrefetcher::DeleteStream(unsigned i){
  // delete the target stream as we no longer need it
  if( i > (baseAddr.size()-1) ){
    return ;
  }

  // delete it
  delete [] iStack[i];
  iStack.erase(iStack.begin() + i);
  baseAddr.erase(baseAddr.begin() + i);
}

// EOF
