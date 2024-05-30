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

class WikiDataEdge {
public:
  uint64_t src;
  uint64_t dst;
  uint64_t type;

  WikiDataEdge( uint64_t s, uint64_t d, uint64_t t ) : src( s ), dst( d ), type( t ) {}

  WikiDataEdge() {}
};

using TestMultiMap = std::multimap<
  uint64_t,                                             // key
  WikiDataEdge,                                         // value
  std::less<uint64_t>,                                  // sorting method
  Allocator<std::pair<const uint64_t, WikiDataEdge>>>;  // revalloc Allocator

void testInsertion() {
  TestMultiMap m;

  m.insert( { 0, WikiDataEdge( 1, 2, 0 ) } );
  m.insert( { 1, WikiDataEdge( 1, 4, 1 ) } );
  m.insert( { 0, WikiDataEdge( 2, 1, 0 ) } );

  assert( m.size() == 3 );
}

void testDuplicateKeyInsertion() {
  TestMultiMap m;
  m.insert( { 0, WikiDataEdge( 1, 2, 0 ) } );
  m.insert( { 1, WikiDataEdge( 1, 4, 1 ) } );
  m.insert( { 0, WikiDataEdge( 2, 1, 0 ) } );

  assert( m.size() == 3 );

  auto range = m.equal_range( 0 );  // all values for key 0
  int  count = 0;
  for( auto it = range.first; it != range.second; it++ ) {
    count++;
  }
  assert( count == 2 );  // should be able to count 3 values for key 1
}

void testOrderPreservation() {
  TestMultiMap m;
  m.insert( { 0, WikiDataEdge( 1, 2, 0 ) } );
  m.insert( { 0, WikiDataEdge( 1, 4, 1 ) } );
  m.insert( { 0, WikiDataEdge( 2, 1, 0 ) } );

  // note the order of insertion and checks is not the same
  auto it = m.begin();
  assert( it->second.src == 1 && it->second.dst == 2 );
  it++;
  assert( it->second.src == 1 && it->second.dst == 4 );
  it++;
  assert( it->second.src == 2 && it->second.dst == 1 );
}

void testDeletion() {
  TestMultiMap m;
  m.insert( { 0, WikiDataEdge( 1, 2, 0 ) } );
  m.insert( { 1, WikiDataEdge( 1, 4, 1 ) } );
  m.insert( { 0, WikiDataEdge( 2, 1, 0 ) } );

  assert( m.size() == 3 );

  // delete all entries with key 1
  m.extract( 1 );

  assert( m.size() == 2 );
}

int main() {
  testInsertion();
  testDuplicateKeyInsertion();
  testOrderPreservation();
  testDeletion();
  return 0;
}
