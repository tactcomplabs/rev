#include "rev-macros.h"
#include "updown.h"
#undef assert
#define assert TRACE_ASSERT

int main() {

  // constructor using total lanes and bool
  uint64_t            total_lanes = 0x100;
  UpDown::networkid_t tl_nwid( total_lanes, false, 0 );
  assert( tl_nwid.get_NetworkId_UdName() == 0x100 );
  assert( tl_nwid.get_TopUd() == 0 );
  assert( tl_nwid.get_SendPolicy() == 0 );
  assert( tl_nwid.get_NodeId() == 0 );
  assert( tl_nwid.get_StackId() == 1 );
  assert( tl_nwid.get_UdId() == 0 );
  assert( tl_nwid.get_LaneId() == 0 );
  assert( tl_nwid.get_NetworkId() == 0x100 );

  total_lanes = 0x283;
  tl_nwid     = UpDown::networkid_t( total_lanes, false, 1 );
  assert( tl_nwid.get_NetworkId_UdName() == 0x283 );
  assert( tl_nwid.get_TopUd() == 0 );
  assert( tl_nwid.get_SendPolicy() == 1 );
  assert( tl_nwid.get_NodeId() == 0 );
  assert( tl_nwid.get_StackId() == 2 );
  assert( tl_nwid.get_UdId() == 2 );
  assert( tl_nwid.get_LaneId() == 3 );
  assert( tl_nwid.get_NetworkId() == 0x283 + ( 1 << 27 ) );

  total_lanes = -1;
  tl_nwid     = UpDown::networkid_t( total_lanes, false, 1 );
  assert( tl_nwid.get_NetworkId_UdName() == 0x7ffffff );
  assert( tl_nwid.get_TopUd() == 0 );
  assert( tl_nwid.get_SendPolicy() == 1 );
  assert( tl_nwid.get_NodeId() == 0xffff );
  assert( tl_nwid.get_StackId() == 7 );
  assert( tl_nwid.get_UdId() == 3 );
  assert( tl_nwid.get_LaneId() == 63 );
  assert( tl_nwid.get_NetworkId() == 0x7ffffff + ( 1 << 27 ) );

  UpDown::ud_machine_t machine;
  uint32_t             source_nwid = 7;
  uint32_t             node_id     = 3;
  uint32_t             cl_id       = source_nwid / machine.NumUDs;
  uint32_t             ud_id       = source_nwid % machine.NumUDs;
  uint32_t             lane_id     = source_nwid % machine.NumLanes;

  assert( cl_id == 1 );    // 7/4  = 1
  assert( ud_id == 3 );    // 7%4  = 3
  assert( lane_id == 7 );  // 7%64 = 7

  uint32_t check_nwid =
    ( node_id << 11 ) | ( cl_id << 8 ) | ( ud_id << 6 ) | ( lane_id );
  assert( check_nwid == 0x19c7 );

  UpDown::networkid_t nwid1( lane_id, ud_id, cl_id, node_id );
  assert( nwid1.get_NetworkId() == 0x19c7 );

  UpDown::networkid_t nwid2( check_nwid, 0, 0 );
  assert( nwid2.get_NetworkId() == 0x19c7 );


  return 0;
}
