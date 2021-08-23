//
// _PanExec_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "PanExec.h"

using namespace SST;
using namespace RevCPU;

PanExec::PanExec() : CurEntry(0){
}

PanExec::~PanExec(){
}

bool PanExec::AddEntry(uint64_t Addr, unsigned *Idx, unsigned Core){
  if( ExecQueue.size() == _PANEXEC_MAX_ENTRY_ )
    return false;

  unsigned Entry = GetNewEntry();

  ExecQueue.push_back(std::tuple<unsigned,
                                 PanExec::PanStatus,
                                 uint64_t,
                                 unsigned>(Entry,PanExec::QValid,Addr,Core));

  *Idx = Entry;

  // Allocate entry in execution cache
  std::vector<std::tuple<uint64_t,PanExec::PanStatus,unsigned>>::iterator it;
  bool AllocateEntry = true;
  for( it=PinCache.begin(); it != PinCache.end(); ++it ){
    if( std::get<1>(*it) == PanExec::QValid ){
      if( std::get<0>(*it) == Addr ){
        AllocateEntry = false;
        break;
      }
    }
  }

  if( AllocateEntry ){
    unsigned CacheEntry = GetNewCacheEntry();
    PinCache.push_back(std::tuple<uint64_t,
                                  PanExec::PanStatus,
                                  unsigned>(Addr,PanExec::QValid,Core));
  }
  return true;
}

bool PanExec::RemoveEntry(unsigned Idx){
  std::vector<std::tuple<unsigned,PanExec::PanStatus,uint64_t,unsigned>>::iterator it;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( Idx == std::get<0>(*it) ){
      ExecQueue.erase(it);
      return true;
    }
  }

  return false;
}

PanExec::PanStatus PanExec::StatusEntry(unsigned Idx){
  std::vector<std::tuple<unsigned,PanExec::PanStatus,uint64_t,unsigned>>::iterator it;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( Idx == std::get<0>(*it) )
      return std::get<1>(*it);
  }

  return PanExec::QError;
}

PanExec::PanStatus PanExec::GetNextEntry(uint64_t *Addr, unsigned *Idx, unsigned Core){
  std::vector<std::tuple<unsigned,PanExec::PanStatus,uint64_t,unsigned>>::iterator it;

  // if no work to do, return null
  if( ExecQueue.size() == 0 )
    return PanExec::QNull;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( std::get<1>(*it) == PanExec::QValid ){
      // if wildcard core identifier
      if(std::get<3>(*it) == 0xFFFF){
	// Look in cache to see if it matches this core or hasn't ever been allocated
        if(Core == CheckPinCache(std::get<2>(*it)) || CheckPinCache(std::get<2>(*it)) == 0xFFFF){
          // use this entry
          *Idx  = std::get<0>(*it);
          *Addr = std::get<2>(*it);
          std::get<1>(*it) = PanExec::QExec;
          SetPinCache(*Addr, Core);
          return PanExec::QExec;
        }else{
          return PanExec::QNull;
	}
      }else if(std::get<3>(*it) == Core){
        // use this entry
        *Idx  = std::get<0>(*it);
        *Addr = std::get<2>(*it);
        std::get<1>(*it) = PanExec::QExec;
        return PanExec::QExec;
      }else{
        // entry Core does not match calling process
        return PanExec::QNull;
      }
    }
  }

  return PanExec::QError;
}

unsigned PanExec::GetNewEntry(){
  CurEntry = CurEntry+1;
  if( CurEntry >= _PANEXEC_MAX_ENTRY_ )
    CurEntry = 0;

  return CurEntry;
}

unsigned PanExec::GetNewCacheEntry(){
  CacheCurEntry = CacheCurEntry+1;
  if( CacheCurEntry >= _PANEXEC_MAX_CACHE_ENTRY_ )
    CacheCurEntry = 0;

  return CacheCurEntry;
}

unsigned PanExec::CheckPinCache(uint64_t Addr){
  std::vector<std::tuple<uint64_t,PanExec::PanStatus,unsigned>>::iterator it;
  for( it=PinCache.begin(); it != PinCache.end(); ++it ){
    if( std::get<1>(*it) == PanExec::QValid ){
      if( std::get<0>(*it) == Addr ){
        return std::get<1>(*it);
      }
    }
  }
  return 0xFFFF;
}

void PanExec::SetPinCache(uint64_t Addr, unsigned Core){
  std::vector<std::tuple<uint64_t,PanExec::PanStatus,unsigned>>::iterator it;
  for( it=PinCache.begin(); it != PinCache.end(); ++it ){
    if( std::get<1>(*it) == PanExec::QValid ){
      if( std::get<0>(*it) == Addr ){
        std::get<2>(*it) = Core;
      }
    }
  }
}

// EOF
