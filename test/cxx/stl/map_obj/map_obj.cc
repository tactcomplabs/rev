//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <array>
#include <map>
#include <string>

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

class EntityEmbedding {
public:
  uint64_t               id;
  std::array<float, DIM> vals;

  EntityEmbedding( uint64_t id ) : id( id ) {}

  EntityEmbedding() {}
};

using TestMap = std::map<uint64_t, EntityEmbedding, std::less<uint64_t>, Allocator<std::pair<const uint64_t, EntityEmbedding>>>;

void testInsertion() {
  TestMap m;

  m.insert( { 0, EntityEmbedding( 0 ) } );
  m.insert( { 1, EntityEmbedding( 1 ) } );
  m.insert( { 0, EntityEmbedding( 2 ) } );

  assert( m.size() == 2 );  // make sure that only two entries exist here
  assert( m[0].id == 0 );   // the original value (0) should be returned
}

void testFind() {
  TestMap m;

  m.insert( { 0, EntityEmbedding( 0 ) } );
  m.insert( { 1, EntityEmbedding( 1 ) } );
  m.insert( { 0, EntityEmbedding( 2 ) } );
  m.insert( { 2, EntityEmbedding( 3 ) } );

  assert( m.size() == 3 );

  auto it1 = m.find( 1 );  // find key1
  assert( it1 != m.end() && it1->second.id == 1 );

  auto it2 = m.find( 4 );  // find key that does not exist
  assert( it2 == m.end() );
}

void testIteration() {
  TestMap m;

  m.insert( { 1, EntityEmbedding( 0 ) } );
  m.insert( { 2, EntityEmbedding( 1 ) } );
  m.insert( { 3, EntityEmbedding( 2 ) } );
  m.insert( { 4, EntityEmbedding( 3 ) } );

  assert( m.size() == 4 );

  // ensure that the map is iterated in the intended order
  int num = 1;
  for( auto it = m.begin(); it != m.end(); it++ ) {
    assert( it->first == num );
    num++;
  }
}

void testDeletion() {
  TestMap m;

  m.insert( { 1, EntityEmbedding( 0 ) } );
  m.insert( { 2, EntityEmbedding( 1 ) } );
  m.insert( { 3, EntityEmbedding( 2 ) } );
  m.insert( { 4, EntityEmbedding( 3 ) } );

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
