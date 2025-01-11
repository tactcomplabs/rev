//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "revalloc.h"
#include <vector>

#include "rev-macros.h"

#include <cstdint>
#include <cstdio>
#include <stdexcept>

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

#define N 10
#define M 20

typedef struct {
  int src;
  int dst;
} edge_t;

class Edge {
public:
  int src;
  int dst;

  Edge() {}

  Edge( int src, int dst ) : src( src ), dst( dst ) {}

  Edge( const Edge& e ) : src( e.src ), dst( e.dst ) {}
};

void test_struct_edge_push_back() {
  std::vector<edge_t, Allocator<edge_t>> v;
  v.push_back( { 1, 1 } );
  v.push_back( { 2, 2 } );
  assert( v.size() == 2 && v[0].src == 1 && v[0].dst == 1 && v[1].src == 2 && v[1].dst == 2 );
}

void test_struct_edge_reserve() {
  std::vector<edge_t, Allocator<edge_t>> v;
  v.reserve( N );

  auto initial_address = v.data();
  bool reserve_failed  = false;

  // add N elements, the vector should not resize
  for( int i = 0; i < N; i++ ) {
    v.push_back( { i, i } );
    if( initial_address != v.data() ) {
      reserve_failed = true;
      break;
    }
  }

  assert( !reserve_failed && v.size() == N );
}

void test_struct_edge_resize() {
  // assert(false);
  std::vector<edge_t, Allocator<edge_t>> v;
  v.resize( N );
  assert( v.size() == N );

  // fill using the access operator
  for( int i = 0; i < N; i++ ) {
    v[i] = { i, i };
  }

  assert( v.size() == N && v[0].src == 0 && v[0].dst == 0 && ( v[N / 2].src = N / 2 ) && ( v[N / 2].dst = N / 2 ) );
}

void test_struct_edge_begin_and_end() {
  std::vector<edge_t, Allocator<edge_t>> v;
  for( int i = 0; i < N; i++ ) {
    v.push_back( { i, i } );
  }

  // use an iterator to check if the vector contains the correct edges
  int i = 0;
  for( auto it = v.begin(); it != v.end(); it++, i++ ) {
    assert( it->src == i && it->dst == i );
  }
}

void test_struct_edge_at() {
  std::vector<edge_t, Allocator<edge_t>> v;
  for( int i = 0; i < N; i++ ) {
    v.push_back( { i, i } );
  }

  for( int i = 0; i < N; i++ ) {
    assert( v.at( i ).src == i && v.at( i ).dst == i );
  }

  try {
    edge_t check = v.at( N + 1 );
    assert( false );  // this should never run
  } catch( const std::out_of_range& e ) {
  }
}

void test_class_edge_reserve() {
  std::vector<Edge, Allocator<Edge>> v;
  v.reserve( N );

  auto initial_address = v.data();
  bool reserve_failed  = false;

  // add N elements, the vector should not resize
  for( int i = 0; i < N; i++ ) {
    v.push_back( Edge( i, i ) );
    if( initial_address != v.data() ) {
      reserve_failed = true;
      break;
    }
  }

  assert( !reserve_failed && v.size() == N );
}

void test_class_edge_resize() {
  std::vector<Edge, Allocator<Edge>> v;
  v.resize( N );
  assert( v.size() == N );

  // fill using the access operator
  for( int i = 0; i < N; i++ ) {
    v[i] = Edge( i, i );
  }

  assert( v.size() == N && v[0].src == 0 && v[0].dst == 0 && ( v[N / 2].src = N / 2 ) && ( v[N / 2].dst = N / 2 ) );
}

void test_class_edge_begin_and_end() {
  std::vector<Edge, Allocator<Edge>> v;
  for( int i = 0; i < N; i++ ) {
    v.push_back( Edge( i, i ) );
  }

  // use an iterator to check if the vector contains the correct edges
  int i = 0;
  for( auto it = v.begin(); it != v.end(); it++, i++ ) {
    assert( it->src == i && it->dst == i );
  }
}

void test_class_edge_at() {
  std::vector<Edge, Allocator<Edge>> v;
  for( int i = 0; i < N; i++ ) {
    v.push_back( Edge( i, i ) );
  }

  for( int i = 0; i < N; i++ ) {
    assert( v.at( i ).src == i && v.at( i ).dst == i );
  }

  try {
    Edge check = v.at( N + 1 );
    assert( false );  // this should never execute
  } catch( const std::out_of_range& e ) {
  }
}

int main() {
  // edge struct tests
  test_struct_edge_push_back();
  test_struct_edge_reserve();
  test_struct_edge_resize();
  test_struct_edge_begin_and_end();
  test_struct_edge_at();

  // edge class tests
  test_class_edge_reserve();
  test_class_edge_resize();
  test_class_edge_begin_and_end();
  test_class_edge_at();

  return 0;
}
