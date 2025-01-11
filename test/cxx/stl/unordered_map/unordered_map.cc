//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "rev-macros.h"
#include "revalloc.h"
#include <string>
#include <unordered_map>

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

typedef std::basic_string<char, std::char_traits<char>, Allocator<char>> revString;

using TestMap =
  std::unordered_map<uint64_t, long, std::hash<uint64_t>, std::equal_to<uint64_t>, Allocator<std::pair<const uint64_t, long>>>;

// using TestMap = std::unordered_map<uint64_t, long, std::less<uint64_t>, Allocator<std::pair<const uint64_t, long>> >;

void testInsertion() {
  TestMap m;

  m.insert( { 0, 1 } );
  m.insert( { 1, 2 } );
  m.insert( { 2, 3 } );
  assert( m.size() == 3 );

  m.insert( { 0, 2 } );
  assert( m[0] == 1 );  // the orignal value of key 0 = 1 should be returned
}

void testElementAccess() {
  TestMap m;
  m.insert( { 0, 1 } );
  m.insert( { 1, 2 } );
  m.insert( { 2, 3 } );

  assert( m.find( 0 ) != m.end() && m[0] == 1 );

  // key 4 should not exist in the map
  assert( m.find( 4 ) == m.end() );
}

void testDeletion() {
  TestMap m;
  m.insert( { 0, 1 } );
  m.insert( { 1, 2 } );
  m.insert( { 2, 3 } );

  assert( m.size() == 3 );

  auto check = m.extract( 0 );

  // removed element 0, check size, removed key and make sure key=0 does not exist in map
  assert( m.find( 0 ) == m.end() && m.size() == 2 && check.key() == 0 );
}

void testIteration() {
  TestMap m;
  m.insert( { 0, 1 } );
  m.insert( { 1, 2 } );
  m.insert( { 2, 3 } );

  // assert( m.begin()->first == 0 );

  for( auto it = m.begin(); it != m.end(); it++ ) {
    assert( it->first >= 0 && it->first <= 2 && it->second >= 1 && it->second <= 3 );
  }
}

int main() {

  testInsertion();
  testElementAccess();
  testDeletion();
  testIteration();

  return 0;
}
