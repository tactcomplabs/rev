/*
 *  This wrapper will be removed once memory mapped based communication is in place.
 *  That is, this is part of a planned transition to using updown.h natively.
 */

#ifndef _REVUPDOWN_H_
#define _REVUPDOWN_H_

// Use local Rev variant
#include "updown.h"

namespace UpDown {

typedef UpDown::UDRuntime_t<
  UpDown::RevRTAllocPolicy< UpDown::ud_mapped_memory_t > >
  UDRuntimeA_t;

class RevUDRuntime_t : public UDRuntimeA_t {

public:
  RevUDRuntime_t( UpDown::ud_machine_t m ) : UDRuntimeA_t( m ) {
    // ECALL 4000: Initialize Rev UpDown CoProcessor
    // TODO Eliminate by using shared simulator configuration data
    int rc;
    asm volatile( "add  a1, %1, x0 \n\t"
                  "li a7, 4000     \n\t"
                  "ecall           \n\t"
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( &m )
                  : "a0", "a1", "a7" );
    assert( rc == 0 );
  };

  virtual ~RevUDRuntime_t(){};

  int rt_wait( unsigned timeout = 1024 ) {
    // ECALL 4001: Wait for CoProcessor busy to clear
    int rc;
    asm volatile(
      "move a1, %1              \n\t"  // init timeout
      "li a7, 4001              \n\t"
      "1:                       \n\t"  // rt_wait_loop
      "ecall                    \n\t"
      "beq a0, x0, 2f           \n\t"  // if equal branch to rt_wait_done
      "addi a1,a1,-1            \n\t"  // timeout--
      "bne a1, x0, 1b           \n\t"  // if not equal branch to rt_wait_loop
      "2:                       \n\t"  // rt_wait_done
      "move %0, a0              \n\t"
      : "=r"( rc )
      : "r"( timeout )
      : "a0", "a1", "a7" );
    return rc;
  }

  void t2ud_memcpy( void*       data,
                    uint64_t    size,
                    networkid_t nwid,
                    uint32_t    offset ) override {
    // ECALL 4010: Copy TOP data to UD scratchpad
    if( rt_wait() != 0 )
      assert( false );
    int rc;
    asm volatile( "add  a1, %1, x0 \n\t"  // data
                  "add  a2, %2, x0 \n\t"  // size
                  "add  a3, %3, x0 \n\t"  // &nwid
                  "add  a4, %4, x0 \n\t"  // offset
                  "li a7, 4010     \n\t"
                  "ecall           \n\t"
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( data ), "r"( size ), "r"( &nwid ), "r"( offset )
                  : "a0", "a1", "a2", "a3", "a4", "a7" );
    assert( rc == 0 );
  };

  void ud2t_memcpy( void*       data,
                    uint64_t    size,
                    networkid_t nwid,
                    uint32_t    offset ) override {
    // ECALL 4011: Copy UD data to TOP
    if( rt_wait() != 0 )
      assert( false );
    int rc;
    asm volatile( "add  a1, %1, x0 \n\t"  // data
                  "add  a2, %2, x0 \n\t"  // size
                  "add  a3, %3, x0 \n\t"  // &nwid
                  "add  a4, %4, x0 \n\t"  // offset
                  "li a7, 4011     \n\t"
                  "ecall           \n\t"
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( data ), "r"( size ), "r"( &nwid ), "r"( offset )
                  : "a0", "a1", "a2", "a3", "a4", "a7" );
    assert( rc == 0 );
  };

  void send_event( event_t ev ) override {
    // ECALL 4020: Send an event to the UpDown event queues
    if( rt_wait() != 0 )
      assert( false );
    int         rc;
    operands_t* ops =
      ev.get_Operands();  // Very confusing data type to work with
    uint64_t* data         = ops->get_Data();
    uint64_t  cont         = data[0];
    uint64_t  num_operands = ops->get_NumOperands();
    asm volatile( "add  a1, %1, x0 \n\t"  // event_t* ev
                  "add  a2, %2, x0 \n\t"  // uint64_t cont
                  "add  a3, %3, x0 \n\t"  // uint64_t* data
                  "add  a4, %4, x0 \n\t"  // uint64_t num_operands
                  "li a7, 4020     \n\t"  //
                  "ecall           \n\t"
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( &ev ), "r"( cont ), "r"( data ), "r"( num_operands )
                  : "a0", "a1", "a2", "a3", "a4", "a7" );
    assert( rc == 0 );
  };

  void start_exec( networkid_t nwid ) override {
    // ECALL 4021: Start a lane
    if( rt_wait() != 0 )
      assert( false );
    int rc;
    asm volatile( "add  a1, %1, x0 \n\t"  // nwid
                  "li a7, 4021     \n\t"
                  "ecall           \n\t"
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( &nwid )
                  : "a0", "a1", "a7" );
    assert( rc == 0 );
  };

  bool test_addr( networkid_t nwid, uint32_t offset, word_t expected = 1 ) {
    // ECALL 4022: Test a memory location for a value
    if( rt_wait() != 0 )
      assert( false );
    int          rc;
    volatile int result = 0;
    asm volatile( "add  a1, %1, x0 \n\t"  // nwid
                  "add  a2, %2, x0 \n\t"  // offset
                  "add  a3, %3, x0 \n\t"  // expected
                  "add  a4, %4, x0 \n\t"  // &result
                  "li   a7, 4022   \n\t"
                  "ecall           \n\t"  // launches memory read request
                  "add %0, a0, x0  \n\t"
                  : "=r"( rc )
                  : "r"( &nwid ), "r"( offset ), "r"( expected ), "r"( &result )
                  : "a0", "a1", "a2", "a3", "a4", "a7" );
    assert( rc == 0 );
    rt_wait();  // wait for memory request to finish
    //assert(result==1); // TODO remove this when accelerator working. Also change in RevSysCalls.cc which is forcing result to true
    return result;
  };


};  // class RevUDRuntime_t

}  // namespace UpDown

#endif  //_REVUPDOWN_H_
