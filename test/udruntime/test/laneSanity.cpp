/*
 * laneLatency.cpp - modified to just check the runtime
 * Environment
 *    UPDOWN=1
 *    PIM_TYPE=2
 */

// Standards includes
#include "stdlib.h"

// Updown includes
// Runtime modifications are required for C++ on REV
// <rev-home>/test/udruntime/include
#define GEM5_MODE
#include "updown.h"

// Rev includes
#include "rev-macros.h"
#include "syscalls.h"

// Assert customization (must be after all include directives)
#undef assert
#define assert         TRACE_ASSERT

// PIM backend simulator types
#define PIM_TYPE_NONE  0
#define PIM_TYPE_TEST  1
#define PIM_TYPE_BASIM 2

// REV Runtime port
using namespace UpDown;
typedef UDRuntime_t< UpDown::RevRTAllocPolicy< UpDown::ud_mapped_memory_t > >
  Runtime_t;

#if 0
#define USAGE                                                              \
  "USAGE: ./lanelatency <num_lanes>  <num_uds> <num_stacks>  <num_nodes> " \
  " <souce_nwid> <dest_nwid> <num_trans>"
#endif

void isav2test( uint32_t              core_id,
                uint32_t              source_nwid,
                uint32_t              dest_nwid,
                Runtime_t*            rt,
                UpDown::ud_machine_t* machine,
                uint32_t              num_trans ) {

  UpDown::operands_t ops( 2 );
  uint32_t node_id = source_nwid / ( machine->NumUDs * machine->NumStacks );
  assert( node_id == 0 );  // TODO remove constraint

  uint32_t cl_id   = source_nwid / machine->NumUDs;
  uint32_t ud_id   = source_nwid % machine->NumUDs;
  uint32_t lane_id = source_nwid % machine->NumLanes;

  assert( cl_id == 0 );    // 2/4  = 0
  assert( ud_id == 2 );    // 2%4  = 2
  assert( lane_id == 2 );  // 2%64 = 2

  uint32_t check_nwid = ( ud_id << 6 ) & 0xc0 | ( lane_id & 0x3f );
#if 1
  assert( check_nwid == 0x82 );
#endif

#if 1
  UpDown::networkid_t nwid( lane_id, ud_id, cl_id, node_id );
#else
  //UpDown::networkid_t nwid(check_nwid, 0, 0);
  UpDown::networkid_t nwid;
  nwid.set( lane_id, ud_id, cl_id, node_id );
#endif

#if 1
  assert( nwid.get_LaneId() == 2 );
  assert( nwid.get_UdId() == 2 );
  assert( nwid.get_NetworkId() ==
          0x82 );  // 2<<6 | 2 = 0x82  (has passed before)
#endif

  // printf("start nwid = %d, end nwid = %d num trans =%d\n", source_nwid, dest_nwid, num_trans);
  ops.set_operand( 0, dest_nwid );
  ops.set_operand( 1, num_trans );

  // Events with operands

  UpDown::event_t evnt_ops( 0 /*Event Label*/,
                            nwid /*Lane ID*/,
                            UpDown::CREATE_THREAD /*Thread ID*/,
                            &ops /*Operands*/ );

  assert( evnt_ops.get_EventWord() == 0x82ff000000 );
  assert( evnt_ops.get_ThreadId() == 0xff );
  assert( evnt_ops.get_NumOperands() == 2 );
  networkid_t enwid = evnt_ops.get_NetworkId();
  assert( enwid.get_LaneId() == 2 );
  assert( enwid.get_UdId() == 2 );
  assert( enwid.get_NetworkId() == 0x82 );


  //rt->send_event(evnt_ops);
  //rt->start_exec(nwid);

  //printf("Check for termination now\n");
  //rt->test_wait_addr(nwid, 0 /*Offset*/, 1 /*Expected value*/);

  //printf("All Events launched and threads terminated\n");
}

/// TODO: This microbenchmark does not support multiple UDs or Threads. Fixme
int main( int argc, char* argv[] ) {

#if 1
  // Issue #214
  uint32_t source_nwid = 2;
  uint32_t dest_nwid   = 8;
  uint32_t num_trans   = 16;
  uint32_t core_id     = 0;
#else
  char* testname;
  if( argc < 2 ) {
    printf( "Insufficient Input Params\n" );
    printf( "%s\n", USAGE );
    exit( 1 );
  }

  uint32_t source_nwid = atoi( argv[1] );
  uint32_t dest_nwid   = atoi( argv[2] );
  uint32_t num_trans   = atoi( argv[3] );
  uint32_t core_id     = atoi( argv[4] );
#endif

  // printf("Sending %d transactions from SRD:%d to DEST:%d\n", num_trans, source_nwid, dest_nwid);

  UpDown::ud_machine_t machine;
  machine.LocalMemAddrMode = 1;

  assert( machine.NumStacks == 8 );
  assert( machine.NumUDs == 4 );

  // auto *rt = new Runtime_t(machine);
  Runtime_t rt = Runtime_t( machine );

  // #if 1
  // // sanity check backend simulator type
  // uint64_t pim_id = 0;
  // networkid_t nwid(0);
  // rt.ud2t_memcpy(&pim_id, 8, nwid, 0);
  // assert(pim_id==PIM_TYPE_BASIM);
  // #endif

  isav2test( core_id, source_nwid, dest_nwid, &rt, &machine, num_trans );

  //printf("ISAv2 test done\n");
}
