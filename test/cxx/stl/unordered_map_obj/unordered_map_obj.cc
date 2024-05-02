//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <array>
#include <string>
#include <unordered_map>

#include "rev-macros.h"
#include "revalloc.h"

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }
#define DIM 450

typedef std::basic_string<char, std::char_traits<char>, Allocator<char>> revString;

class ContigSet {
public:
  uint64_t id;
  uint8_t  val1;
  uint64_t val2;
  int      val3;

  ContigSet() {}

  ContigSet( uint64_t id ) : id( id ) {}

  ContigSet( uint64_t id, uint8_t v1, uint64_t v2, uint64_t v3 ) : id( id ), val1( v1 ), val2( v2 ), val3( v3 ) {}
};

using TestMap = std::
  unordered_map<uint64_t, ContigSet, std::hash<uint64_t>, std::equal_to<uint64_t>, Allocator<std::pair<const uint64_t, ContigSet>>>;

void testInsertion() {
  TestMap m;

  m.insert( { 0, ContigSet( 0 ) } );
  m.insert( { 1, ContigSet( 1 ) } );
  m.insert( { 0, ContigSet( 2 ) } );

  assert( m.size() == 2 );  // make sure that only two entries exist here
  assert( m[0].id == 0 );   // the original value (0) should be returned

  // check for value update
  m[0].id = 42;
  assert( m[0].id == 42 );
}

void testFind() {
  TestMap m;

  m.insert( { 0, ContigSet( 0 ) } );
  m.insert( { 1, ContigSet( 1 ) } );
  m.insert( { 0, ContigSet( 2 ) } );
  m.insert( { 2, ContigSet( 3 ) } );

  assert( m.size() == 3 );

  auto it1 = m.find( 1 );  // find key1
  assert( it1 != m.end() && it1->second.id == 1 );

  auto it2 = m.find( 4 );  // find key that does not exist
  assert( it2 == m.end() );
}

void testIteration() {
  TestMap m;

  m.insert( { 1, ContigSet( 0 ) } );
  m.insert( { 2, ContigSet( 1 ) } );
  m.insert( { 3, ContigSet( 2 ) } );
  m.insert( { 4, ContigSet( 3 ) } );

  assert( m.size() == 4 );

  int count = 0;

  // ensure all objects are iterated over with range check on expected values
  for( auto it = m.begin(); it != m.end(); it++ ) {
    assert( it->first >= 1 && it->first <= 4 && it->second.id >= 0 && it->second.id <= 3 );
    count++;
  }

  assert( count == m.size() );
}

void testDeletion() {
  TestMap m;

  m.insert( { 1, ContigSet( 0 ) } );
  m.insert( { 2, ContigSet( 1 ) } );
  m.insert( { 3, ContigSet( 2 ) } );
  m.insert( { 4, ContigSet( 3 ) } );

  assert( m.size() == 4 );

  auto check = m.extract( 1 );  // remove key 1
  assert( check.key() == 1 );
  assert( m.size() == 3 );
}

int main() {

  testInsertion();
  testFind();
  testIteration();
  testDeletion();

  return 0;
}
