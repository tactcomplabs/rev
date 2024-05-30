//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "rev-macros.h"
#include "revalloc.h"
#include <map>
#include <string>

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

typedef std::basic_string<char, std::char_traits<char>, Allocator<char>> revString;

int main() {

  //When constructing the map the correct method is to use the default constructor and then
  //  insert each element as shown (using std::pair)

  std::map<revString, int, std::less<revString>, Allocator<std::pair<const revString, int>>> m;

  m.insert( std::pair{ "CPU", 10 } );
  m.insert( std::pair{ "GPU", 11 } );
  m.insert( std::pair{ "RAM", 12 } );
  assert( m.size() == 3 );

  m["CPU"] = 15;
  m["GPU"] = 17;
  m["RAM"] = 20;

  assert( m["CPU"] == 15 );
  assert( m["GPU"] == 17 );
  assert( m["RAM"] == 20 );

  assert( m.count( "RAM" ) == 1 );
  assert( m.count( "Nope" ) == 0 );

  m.extract( "RAM" );  //Use extract instead of erase
  assert( m.size() == 2 );
  assert( m["CPU"] == 15 );
  assert( m.count( "RAM" ) == 0 );

  m.extract( "GPU" );
  assert( m.size() == 1 );

  return 0;
}
