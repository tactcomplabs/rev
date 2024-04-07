#include "rev-macros.h"
#include <updown.h>
#undef assert
#define assert TRACE_ASSERT

int main() {

  UpDown::ud_machine_t machine;
  uint32_t             source_nwid = 2;
  uint32_t             node_id     = 0;
  uint32_t             cl_id       = source_nwid / machine.NumUDs;
  uint32_t             ud_id       = source_nwid % machine.NumUDs;
  uint32_t             lane_id     = source_nwid % machine.NumLanes;

  assert( cl_id == 0 );    // 2/4  = 0
  assert( ud_id == 2 );    // 2%4  = 2
  assert( lane_id == 2 );  // 2%64 = 2

  uint32_t check_nwid = ( ud_id << 6 ) & 0xc0 | ( lane_id & 0x3f );
  assert( check_nwid == 0x82 );

  UpDown::networkid_t nwid1( lane_id, ud_id, cl_id, node_id );
  assert( nwid1.get_NetworkId() == 0x82 );  // 2<<6 | 2 = 0x82

  UpDown::networkid_t nwid2( check_nwid, 0, 0 );
  assert( nwid2.get_NetworkId() == 0x82 );  // 2<<6 | 2 = 0x82


  UpDown::word_t     op_data[3];
  UpDown::operands_t ops( 3, op_data );
  ops.set_operand( 0, 0x8000 );
  ops.set_operand( 1, 0x100 );
  for( int ln = 0; ln < 64; ln += 8 ) {
    for( int udid = 0; udid < 4; udid++ ) {
      for( int stack_id = 0; stack_id < 8; stack_id++ ) {
        for( int node_id = 0; node_id < 8; node_id += 2 ) {
          UpDown::networkid_t nwid( ln, udid, stack_id, node_id );
          ops.set_operand( 2, 0x1000 * ln );
          UpDown::event_t ev( 0, nwid, 0xff, &ops );
          assert( nwid.get_LaneId() == ln );
          assert( ev.get_NetworkId().get_LaneId() == ln );
          assert( nwid.get_UdId() == udid );
          assert( ev.get_NetworkId().get_UdId() == udid );
          assert( nwid.get_StackId() == stack_id );
          assert( ev.get_NetworkId().get_StackId() == stack_id );
          assert( nwid.get_NodeId() == node_id );
          assert( ev.get_NetworkId().get_NodeId() == node_id );
        }
      }
    }
  }

  return 0;
}
