#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <utility>

#include "debug.h"

namespace UpDown {

/**
 * @brief Contains encoding of hardware structures in UpDown System
 *
 * This class constructs the networkID description. The networkID are
 * the MSB of the event word, and they describe a unique location in
 * the system. There are two formats that can be enconded in the
 * networkID. This is determined by the TopUd bit.
 *
 * # Top/UD = 0 (UD Network IDs):
 * * Lane (6 bits)
 * * UDs (2 bits)
 * * Stack (3 bits)
 * * Node (16 bits)
 * * Send Policy (3 bits)
 *
 * # Top/UD = 1 (Top core or LLC Network IDs):
 * * Core or LLC ID (5 bits)
 * * Core Structures (3 bits): Cache/Prefetch buffer/Load-store queues
 * * Operation (3 bits): Invalidate, write, etc...
 * * Node (16 bits)
 *
 * @todo: Add encoding for top network elements
 */
class networkid_t {
private:
  uint32_t NetworkId;  // Complete networkID encoded
public:
  uint32_t get_NetworkId() {
    return NetworkId;
  }

  uint32_t get_NetworkId_UdName() {
    return NetworkId & ( ( 1 << ( 6 + 2 + 3 + 16 ) ) - 1 );
  }

  uint8_t get_LaneId() {
    return ( NetworkId & 0x3f );
  }

  uint8_t get_UdId() {
    return ( NetworkId & 0xc0 ) >> 6;
  }

  uint8_t get_StackId() {
    return ( NetworkId & 0x700 ) >> 8;
  }

  uint16_t get_NodeId() {
    return ( NetworkId & 0x7fff800 ) >> 11;
  }

  uint8_t get_SendPolicy() {
    return ( NetworkId & 0x38000000 ) >> 27;
  }

  uint8_t get_TopUd() {
    return ( NetworkId & 0x80000000 ) >> 31;
  }

  void set_SendPolicy( uint8_t SendPolicy ) {
    NetworkId = NetworkId & 0xC7FFFFFF | ( ( SendPolicy << 27 ) & 0x38000000 );
  }

  /**
   * @brief Construct a new network id
   *
   */
  networkid_t() : NetworkId( 0 ) {
    // rev_fast_print restricted to 6 words
    assert( get_TopUd() == 0 );
    UPDOWN_INFOMSG(
      "(a) Creating a new network id TopUd = 0, SendPolicy = %d, NodeId = %d, "
      "StackId=%d, UDId = 0x%X, lane_id=%d, network_id=0x%X",
      //get_TopUd(),
      get_SendPolicy(),
      get_NodeId(),
      get_StackId(),
      get_UdId(),
      get_LaneId(),
      get_NetworkId() );
  }

  networkid_t( uint8_t  lane_id,
               uint8_t  udid       = 0,
               uint8_t  stack_id   = 0,
               uint32_t nodeid     = 0,
               uint8_t  topud      = 0,
               uint8_t  sendpolicy = 0 ) {
    NetworkId =
      ( ( topud << 31 ) & 0x80000000 ) | ( ( sendpolicy << 27 ) & 0x38000000 ) |
      ( ( nodeid << 11 ) & 0x7fff800 ) | ( ( stack_id << 8 ) & 0x700 ) |
      ( ( udid << 6 ) & 0xC0 ) | ( ( lane_id & 0x3f ) );
    assert( get_TopUd() == 0 );
    UPDOWN_INFOMSG(
      "(b) Creating a new network id TopUd = 0, SendPolicy = %d, NodeId = %d, "
      "StackId=%d, UDId = 0x%X, lane_id=%d, network_id=0x%X",
      //get_TopUd(),
      get_SendPolicy(),
      get_NodeId(),
      get_StackId(),
      get_UdId(),
      get_LaneId(),
      get_NetworkId() );
  }

  networkid_t( uint32_t ud_name, bool topud, uint8_t sendpolicy ) :
    NetworkId( ( ( topud << 31 ) & 0x80000000 ) |
               ( ( sendpolicy << 27 ) & 0x38000000 ) |
               ( ( ud_name & 0x7FFFFFF ) ) ) {
    assert( get_TopUd() == 0 );
    UPDOWN_INFOMSG(
      "(c) Creating a new network id TopUd = 0, SendPolicy = %d, NodeId = %d, "
      "StackId=%d, UDId = 0x%X, lane_id=%d, network_id=0x%X",
      //get_TopUd(),
      get_SendPolicy(),
      get_NodeId(),
      get_StackId(),
      get_UdId(),
      get_LaneId(),
      get_NetworkId() );
  }

  // REV
  networkid_t( networkid_t& rhs ) {
    NetworkId = rhs.get_NetworkId();
  }

  networkid_t( networkid_t&& ) = default;

  networkid_t& operator=( networkid_t& rhs ) {
    NetworkId = rhs.get_NetworkId();
    return *this;
  }

  networkid_t& operator=( networkid_t&& ) = default;
};

}  // namespace UpDown
