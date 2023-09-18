//
// _PanExec_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/PanExec.h"

using namespace SST;
using namespace RevCPU;

PanExec::PanExec() : CurEntry(0){
}

PanExec::~PanExec(){
}

bool PanExec::AddEntry(uint64_t Addr, unsigned *Idx){
  if( ExecQueue.size() == _PANEXEC_MAX_ENTRY_ )
    return false;

  unsigned Entry = GetNewEntry();

  ExecQueue.push_back(std::tuple<unsigned,
                                 PanExec::PanStatus,
                                 uint64_t>(Entry, PanExec::QValid, Addr));

  *Idx = Entry;
  return true;
}

bool PanExec::RemoveEntry(unsigned Idx){
  std::vector<std::tuple<unsigned, PanExec::PanStatus, uint64_t>>::iterator it;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( Idx == std::get<0>(*it) ){
      ExecQueue.erase(it);
      return true;
    }
  }

  return false;
}

PanExec::PanStatus PanExec::StatusEntry(unsigned Idx){
  std::vector<std::tuple<unsigned, PanExec::PanStatus, uint64_t>>::iterator it;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( Idx == std::get<0>(*it) )
      return std::get<1>(*it);
  }

  return PanExec::QError;
}

PanExec::PanStatus PanExec::GetNextEntry(uint64_t *Addr, unsigned *Idx){
  std::vector<std::tuple<unsigned, PanExec::PanStatus, uint64_t>>::iterator it;

  // if no work to do, return null
  if( ExecQueue.size() == 0 )
    return PanExec::QNull;

  for( it=ExecQueue.begin(); it != ExecQueue.end(); ++it ){
    if( std::get<1>(*it) == PanExec::QValid ){
      // use this entry
      *Idx  = std::get<0>(*it);
      *Addr = std::get<2>(*it);
      std::get<1>(*it) = PanExec::QExec;
      return PanExec::QExec;
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

// EOF
