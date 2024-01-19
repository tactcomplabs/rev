#ifndef __REV_MACROS_H__
#define __REV_MACROS_H__

// Supported values for TRC_OP: slti sltiu slli srli srai

#ifndef NO_REV_MACROS
#define TRACE_OFF      asm volatile("slli x0,x0,0"); 
#define TRACE_ON       asm volatile("slli x0,x0,1"); 
#define TRACE_PUSH_OFF asm volatile("slli x0,x0,2"); 
#define TRACE_PUSH_ON  asm volatile("slli x0,x0,3"); 
#define TRACE_POP      asm volatile("slli x0,x0,4");
#define TRACE_ASSERT(x) { TRACE_PUSH_ON; \
  if (!(x)) { asm volatile(".word 0x0"); }; \
  TRACE_PUSH_OFF }
#else
#define TRACE_OFF
#define TRACE_ON 
#define TRACE_PUSH_OFF
#define TRACE_PUSH_ON
#define TRACE_POP
#define TRACE_ASSERT(x) { if (!(x)) { asm volatile(".word 0x0"); } };
#endif

#endif // __REV_MACROS_H__
