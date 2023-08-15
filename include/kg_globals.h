#ifndef _KG_GLOBALS_H_
#define _KG_GLOBALS_H_

#include <stdint.h>
#include <iostream>

//#define USE_KG_GLOBAL

namespace KG_GLOBAL {
#ifdef USE_KG_GLOBAL
    #define KG_DEBUG(msg) std::cout << "[kg_debug]" \
        << std::dec << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl;
    static void spinner(unsigned id)
    {
        uint64_t spinner = 1;
        while (spinner > 0)
        {
            spinner++;
            // set breakpoint here and clear spinner
            if (spinner % 1000000000 == 0)
                KG_DEBUG(id << ":spinner=" << spinner);
        }
        KG_DEBUG(id << ":spinner exiting");
    }
#else
    #define KG_DEBUG(msg)
    static void spinner(unsigned id) {};
#endif
} // namespace KG_GLOBAL

#endif // _KG_GLOBALS_H_
