#include "updown.h"

#include "rev-macros.h"
#include "syscalls.h"
#undef assert
#define assert TRACE_ASSERT

#ifdef DEBUG_MODE
#define printf rev_fast_print
#else
#define printf( ... )
#endif

//Fine to overload new at global scope, could also be done per class

/*void* operator new(std::size_t t){
     void* p = reinterpret_cast<void*>(rev_mmap(0,
              t,
              PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_PRIVATE | MAP_ANONYMOUS,
              -1,
              0));
    return p;
}*/

void printEvent( UpDown::event_t& ev ) {
  printf( "Setting the event word = %d, lane_id = %d, "
          "thread_id = %d, num_operands = %d, ev_word = 0x%lX\n",
          ev.get_EventLabel(),
          ( ev.get_NetworkId() ).get_LaneId(),
          ev.get_ThreadId(),
          ev.get_NumOperands(),
          ev.get_EventWord() );
}

void test1();

int  main() {

   // Create an empty event_
  UpDown::event_t empty_evnt;
  printEvent( empty_evnt );
  UpDown::event_t def_envt( 10, UpDown::networkid_t( 11 ) );
  printEvent( def_envt );
  UpDown::event_t def_envt2( 12, UpDown::networkid_t( 13, 14 ) );
  printEvent( def_envt2 );
  UpDown::event_t def_envt3( 15, UpDown::networkid_t( 16, 17 ), 18 );
  printEvent( def_envt3 );

  // Help operands
  UpDown::word_t     ops_data[] = { 1, 2, 3, 4 };
  UpDown::operands_t ops( 4, ops_data );

  // Events with operands
  UpDown::event_t    evnt_ops( 15, UpDown::networkid_t( 16, 17 ), 18, &ops );
  printEvent( evnt_ops );

 #if 0
  //
  // (bug) Cannot have empty ops
  //
  def_envt.set_event(1, UpDown::networkid_t(2));
  printEvent(def_envt);
  def_envt.set_event(3, UpDown::networkid_t(4, 5));
  printEvent(def_envt);
  def_envt.set_event(3, UpDown::networkid_t(4, 5), 6);
  printEvent(def_envt);
#endif

  def_envt.set_event( 3, UpDown::networkid_t( 4, 5 ), 6, &ops );
  printEvent( def_envt );

  test1();

  return 0;
}

void test1() {
  uint64_t           minus1     = 0xffffffffffffffffULL;
  uint64_t           ops_data[] = { minus1, minus1 - 1, minus1 - 2 };
  UpDown::operands_t ops( 3, ops_data );
  UpDown::event_t    e = UpDown::event_t(
    15, UpDown::networkid_t( 5 ), UpDown::CREATE_THREAD, &ops );
  // retrieve and check the ops
  uint64_t* p_data = e.get_Operands()->get_Data();
  assert( ops_data[0] == (uint64_t) p_data[1] );
  assert( ops_data[1] == (uint64_t) p_data[2] );
  assert( ops_data[2] == (uint64_t) p_data[3] );
  // check the remaining fields
  assert( e.get_EventWord() == ( 5UL << 32 ) | ( 0x0ffUL << 24 ) | 15 );
  assert( e.get_ThreadId() == UpDown::CREATE_THREAD );
  assert( e.get_NumOperands() == 3 );
  assert( e.get_EventLabel() == 15 );

  UpDown::networkid_t nwid2 = e.get_NetworkId();
  assert( nwid2.get_NetworkId() == 5 );
}
