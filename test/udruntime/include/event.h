#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <utility>

#include "debug.h"

#include "networkid.h"
#include "operands.h"

namespace UpDown {

/**
 * @brief Contains information of an )event in the UpDown
 *
 * This class constructs the information of the event word based on
 * each of its parameters. It also contains a pointer to the operands
 * that is used when sending the event
 *
 * @todo UpDown ID is not being used
 * @todo, event_t considers a 4 byte word size
 */
class event_t {
private:
  word_t         EventWord = 0x0ffUL << 24;  // Fully encoded
  networkid_t    NetworkId = networkid_t( 0 );
  operands_t*    Operands  = nullptr;  // Operands to be sent with this event
  static uint8_t save_tid;

public:
  word_t get_EventWord() {
    return EventWord;
  }

  operands_t* get_Operands() {
    return Operands;
  }

  void set_operands( operands_t* ops ) {
    Operands = ops;
  }

  networkid_t get_NetworkId() {
    return NetworkId;
  }

  uint8_t get_ThreadId() {
    return ( EventWord >> 24 ) & 0xff;
  }

  uint32_t get_EventLabel() {
    return EventWord & 0xfffff;
  }

  void set_EventLabel( uint32_t label ) {
    EventWord = ( EventWord & 0xFFFFFFFFFFF00000 ) | ( label & 0xfffff );
  }

  uint8_t get_NumOperands() {
    return ( Operands != nullptr ) ? Operands->get_NumOperands() : 0;
  }

  uint8_t get_EncodedNumOperands() {
    return get_EventWord() >> 20 & 0x7;
  }

  ptr_t get_OperandsData() {
    return ( Operands != nullptr ) ? Operands->get_Data() : nullptr;
  }

  /**
   * @brief Construct a new empty event object
   *
   */

  event_t() {
    //UPDOWN_INFOMSG("event_t\n");
    EventWord = ( ( ( ( 0xff << 24 ) & 0xff000000 ) |
                    ( ( 0 << 20 ) & 0xf00000 ) | ( 0 & 0xfffff ) ) &
                  0xffffffffffffffff );

    UPDOWN_INFOMSG( "Creating a new event label = %d, "
                    "thread_id = %d, num_operands = %d, ev_word = 0x%lX",
                    get_EventLabel(),
                    get_ThreadId(),
                    0,
                    get_EventWord() );
  };

  /**
   * @brief Construct a new event_t object
   *
   * @param e_label Event label ID
   * @param nwid Network ID representing Lane, Updown, Node, etc
   * @param tid Thread ID
   * @param operands Pointer to operands. Must be pre-initialized
   */
  event_t( uint32_t    e_label,
           networkid_t nwid,
           uint8_t     tid      = CREATE_THREAD,
           operands_t* operands = nullptr ) {
// UPDOWN_INFOMSG("event_t(%d, %d, %d, %lx)\n",
//    e_label, nwid.get_NetworkId(), tid, reinterpret_cast<uint64_t>(operands));

// uncomment next line to pass O2
//#define O2_WORKAROUND
// for some reason tid appears to be getting optimized out in O2.
// printing it causes the test to pass.
// Saving into static appears to have the same effect.
#ifdef O2_WORKAROUND
    save_tid = tid;
#endif

    NetworkId = nwid;
    Operands  = operands;
    if( operands ) {
      assert( Operands->get_NumOperands() > 1 );
    }
    EventWord = ( NetworkId.get_NetworkId() & 0xffffffff );
    EventWord =
      ( ( EventWord << 32 ) & 0xffffffff00000000 ) |
      ( ( tid << 24 ) & 0xff000000 ) | ( ( 0 << 23 ) & 0x800000 ) |
      ( ( ( Operands != nullptr ? Operands->get_NumOperands() - 2 : 0 )
          << 20 ) &
        0xf00000 ) |
      ( e_label & 0xfffff );
    UPDOWN_INFOMSG( "Creating a new event label = %d, network_id = %d, "
                    "thread_id = %d, num_operands = %d, ev_word = 0x%lx",
                    get_EventLabel(),
                    NetworkId.get_NetworkId(),
                    get_ThreadId(),
                    ( Operands != nullptr ) ? Operands->get_NumOperands() : 0,
                    EventWord );
  }

  /**
   * @brief Set the event word object with new values
   *
   * @param e_label the ID of the event in the updown
   * @param noperands the number of operands
   * @param lid Lane ID
   * @param tid Thread ID
   */
  void set_event( uint32_t    e_label,
                  networkid_t nwid,
                  uint8_t     tid      = CREATE_THREAD,
                  operands_t* operands = nullptr ) {
    // UPDOWN_INFOMSG("set_event(%d, %d, %d, %lx\n",
    // 	   e_label, nwid.get_NetworkId(), tid, reinterpret_cast<uint64_t>(operands));
    NetworkId = nwid;
    Operands  = operands;
    if( Operands ) {
      assert( Operands->get_NumOperands() > 1 );
    }
    EventWord =
      ( ( ( static_cast< uint64_t >( nwid.get_NetworkId() ) << 32 ) &
          0xffffffff00000000 ) |
        ( ( tid << 24 ) & 0xff000000 ) | ( ( 0 << 23 ) & 0x0800000 ) |
        ( ( ( Operands != nullptr ? Operands->get_NumOperands() - 2 : 0 )
            << 20 ) &
          0xf00000 ) |
        ( e_label & 0xfffff ) );
    UPDOWN_INFOMSG( "Setting the event word = %d, network_id = %d, "
                    "thread_id = %d, num_operands = %d, ev_word = 0x%lX",
                    get_EventLabel(),
                    nwid.get_NetworkId(),
                    tid,
                    ( Operands != nullptr ) ? Operands->get_NumOperands() : 0,
                    (unsigned long) EventWord );
  }

  // REV
  event_t( event_t& rhs ) {
    EventWord = rhs.get_EventWord();
    NetworkId = rhs.get_NetworkId();
    Operands  = rhs.get_Operands();
  }

  event_t( event_t&& ) = default;

  event_t& operator=( event_t& rhs ) {
    EventWord = rhs.get_EventWord();
    NetworkId = rhs.get_NetworkId();
    Operands  = rhs.get_Operands();
    return *this;
  }

  event_t& operator=( event_t&& ) = default;
};

uint8_t event_t::save_tid = 0xff;
}  // namespace UpDown
