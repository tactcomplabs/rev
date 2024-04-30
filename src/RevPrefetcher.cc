//
// _RevPrefetcher_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevPrefetcher.h"

namespace SST::RevCPU {

/*RevPrefetcher::~RevPrefetcher(){
// delete all the existing streams
for(auto* s : iStack)
delete[] s;
}*/

bool RevPrefetcher::IsAvail( uint64_t Addr ) {

  // note: this logic now considers compressed instructions
  uint64_t lastAddr = 0x00ull;
  for( unsigned i = 0; i < baseAddr.size(); i++ ) {
    lastAddr = baseAddr[i] + ( depth * 4 );
    if( ( Addr >= baseAddr[i] ) && ( Addr < lastAddr ) ) {
      // found it, fetch the address
      // first, calculate the vector offset
      uint32_t Off = static_cast<uint32_t>( ( Addr - baseAddr[i] ) / 4 );
      if( Off > ( depth - 1 ) ) {
        // some sort of error occurred
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ) {
        // the instruction hasn't been filled yet, stall
        return false;
      }

      // we may be short of instruction width in our current stream
      // determine if an adjacent stream has the payload
      if( ( lastAddr - Addr < 4 ) || ( ( Addr & 0x03 ) != 0 ) ) {

        uint32_t TmpInst;
        bool     Fetched = false;
        if( !FetchUpper( Addr + 2, Fetched, TmpInst ) ) {
          return false;
        }
        if( !Fetched ) {
          // initiated a Fill
          return false;
        }
      }

      if( !OutstandingFetchQ.empty() ) {
        auto it =
          LSQueue->equal_range( OutstandingFetchQ.back().LSQHash() );  // Find all outstanding dependencies for this register
        if( it.first != LSQueue->end() ) {
          for( auto i = it.first; i != it.second; ++i ) {  // Iterate over all outstanding loads for this reg (if any)
            if( i->second.Addr == OutstandingFetchQ.back().Addr ) {
              return false;
            }
          }
        }
      }
      // the instruction is available in the stream cache
      return true;
    }
  }

  // if we reach this point, then the instruction hasn't even triggered
  // a stream prefetch.  Lets go ahead and initiate one via a 'Fill' operation
  Fill( Addr );
  return false;
}

void RevPrefetcher::MarkInstructionLoadComplete( const MemReq& req ) {
  auto it = OutstandingFetchQ.begin();
  while( ( it != OutstandingFetchQ.end() ) ) {
    if( it->Addr == req.Addr ) {
      OutstandingFetchQ.erase( it++ );
      break;
    }
    it++;
  }
}

bool RevPrefetcher::FetchUpper( uint64_t Addr, bool& Fetched, uint32_t& UInst ) {
  uint64_t lastAddr = 0x00ull;
  for( unsigned i = 0; i < baseAddr.size(); i++ ) {
    lastAddr = baseAddr[i] + ( depth * 4 );
    if( ( Addr >= baseAddr[i] ) && ( Addr < lastAddr ) ) {
      uint32_t Off = static_cast<uint32_t>( ( Addr - baseAddr[i] ) / 4 );
      if( Off > ( depth - 1 ) ) {
        // some sort of error occurred
        Fetched = false;
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ) {
        // the instruction hasn't been filled yet, stall
        Fetched = false;
        return true;
      }

      // fetch the instruction
      if( Addr == ( baseAddr[i] + ( Off * 4 ) ) ) {
        UInst   = ( iStack[i][Off] << 16 );
        Fetched = true;
        return true;
      }
    }
  }

  Fill( Addr );
  Fetched = false;

  return true;
}

bool RevPrefetcher::InstFetch( uint64_t Addr, bool& Fetched, uint32_t& Inst ) {
  // scan the baseAddr vector to see if the address is cached
  uint64_t lastAddr = 0x00ull;
  for( unsigned i = 0; i < baseAddr.size(); i++ ) {
    lastAddr = baseAddr[i] + ( depth * 4 );
    if( ( Addr >= baseAddr[i] ) && ( Addr < lastAddr ) ) {
      // found it, fetch the address
      // first, calculate the vector offset
      uint32_t Off = static_cast<uint32_t>( ( Addr - baseAddr[i] ) / 4 );
      if( Off > ( depth - 1 ) ) {
        // some sort of error occurred
        Fetched = false;
        return false;
      }

      if( iStack[i][Off] == REVPREF_INIT_ADDR ) {
        // the instruction hasn't been filled yet, stall
        Fetched = false;
        return true;
      }

      // fetch the instruction
      if( Addr == ( baseAddr[i] + ( Off * 4 ) ) ) {
        Inst = iStack[i][Off];
      } else {
        // compressed instruction, adjust the offset
        Inst = ( iStack[i][Off] >> 16 );
        uint32_t TmpInst;
        if( !FetchUpper( Addr + 2, Fetched, TmpInst ) )
          return false;
        if( !Fetched ) {
          // we initiated a fill
          return true;
        }
        Inst |= TmpInst;
      }

      Fetched = true;

      // if this is the last instruction in the stream buffer, we need to deallocate the stream
      if( Off == ( depth - 1 ) ) {
        DeleteStream( i );
        Fill( Addr + 2 );  // go ahead and fill the next instruction
      }

      return true;
    }
  }

  // we missed in the stream cache, lets perform a fill
  Fill( Addr );
  Fetched = false;

  return true;
}

void RevPrefetcher::Fill( uint64_t Addr ) {

  // If address is not 32bit aligned... then make it aligned
  Addr &= 0xFFFFFFFFFFFFFFFC;

  // allocate a new stream buffer
  baseAddr.push_back( Addr );
  iStack.push_back( std::vector<uint32_t>( depth ) );

  // initialize it
  size_t x = baseAddr.size() - 1;
  for( size_t y = 0; y < depth; y++ ) {
    iStack[x][y] = REVPREF_INIT_ADDR;
  }

  // now fill it
  for( size_t y = 0; y < depth; y++ ) {
    MemReq req(
      Addr + ( y * 4 ), RevReg::zero, RevRegClass::RegGPR, feature->GetHartToExecID(), MemOp::MemOpREAD, true, MarkLoadAsComplete
    );
    LSQueue->insert( req.LSQHashPair() );
    OutstandingFetchQ.emplace_back( req );
    mem->ReadVal<uint32_t>( feature->GetHartToExecID(), Addr + ( y * 4 ), &iStack[x][y], req, RevFlag::F_NONE );
    //Track outstanding requests
  }
}

void RevPrefetcher::DeleteStream( size_t i ) {
  // delete the target stream as we no longer need it
  if( i < baseAddr.size() ) {
    iStack.erase( iStack.begin() + i );
    baseAddr.erase( baseAddr.begin() + i );
  }
}

}  // namespace SST::RevCPU

// EOF
