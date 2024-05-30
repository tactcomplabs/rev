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

using TestMultiMap = std::multimap<
  uint64_t,                                                           // key
  std::pair<uint64_t, float>,                                         // value
  std::less<uint64_t>,                                                // sorting method
  Allocator<std::pair<const uint64_t, std::pair<uint64_t, float>>>>;  // revalloc Allocator

void testInsertion() {
  TestMultiMap m;

  m.insert( {
    1, { 2, 3.0f }
  } );
  m.insert( {
    2, { 3, 4.0f }
  } );
  assert( m.size() == 2 );

  m.insert( {
    1, { 4, 5.0f }
  } );
  m.insert( {
    1, { 6, 7.0f }
  } );
  assert( m.size() == 4 );
}

void testDuplicateKeyInsertion() {
  TestMultiMap m;
  m.insert( {
    1, { 2, 3.0f }
  } );
  m.insert( {
    1, { 4, 5.0f }
  } );
  m.insert( {
    1, { 6, 7.0f }
  } );

  assert( m.size() == 3 );

  auto range = m.equal_range( 1 );  // all values for key 1
  int  count = 0;
  for( auto it = range.first; it != range.second; it++ ) {
    count++;
  }
  assert( count == 3 );  // should be able to count 3 values for key 1
}

void testOrderPreservation() {
  TestMultiMap m;
  m.insert( {
    1, { 2, 3.0f }
  } );
  m.insert( {
    1, { 4, 5.0f }
  } );
  m.insert( {
    1, { 6, 7.0f }
  } );

  auto it = m.begin();
  assert( it->second.first == 2 );
  it++;
  assert( it->second.first == 4 );
}

void testDeletion() {
  TestMultiMap m;
  m.insert( {
    1, { 2, 3.0f }
  } );
  m.insert( {
    1, { 4, 5.0f }
  } );
  m.insert( {
    1, { 6, 7.0f }
  } );
  m.insert( {
    2, { 7, 8.0f }
  } );

  assert( m.size() == 4 );

  // delete all entries with key 2
  m.extract( 2 );

  assert( m.size() == 3 );
}

int main() {
  testInsertion();
  testDuplicateKeyInsertion();
  testOrderPreservation();
  testDeletion();
  return 0;
}
