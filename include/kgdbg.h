//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _KGDBG_H
#define _KGDBG_H

#include <iostream>
#include <unistd.h>

namespace kgdbg {

static inline void spin( const char* id = "" ) {
  std::cout << id << " spinning" << std::endl;
  unsigned long spinner = 1;
  while( spinner > 0 ) {
    spinner++;
    usleep( 100000 );
    if( spinner % 10 == 0 )  // breakpoint here
      std::cout << "." << std::flush;
  }
  std::cout << std::endl;
}

static inline void spinner( const char* id, bool cond = true ) {
  if( !std::getenv( id ) )
    return;
  if( !cond )
    return;
  spin( id );
}

}  //namespace kgdbg

#endif  //_KGDBG_H
