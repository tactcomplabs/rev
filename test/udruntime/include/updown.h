/*
 * Copyright (c) 2021 University of Chicago and Argonne National Laboratory
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original Author - Jose M Monsalve Diaz
 * Port To Rev - Dave Donofrio, Ken Griesser
 *
 */

#ifndef UPDOWN_H
#define UPDOWN_H
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <utility>

#include "debug.h"
#include "updown_config.h"

#include "event.h"
#include "ud_machine.h"

#include "revalloc.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

//TODO remove this. Only doing this since vscode intellisense isn't detecting it.
#ifndef ASST
#define ASST
#endif

namespace UpDown {

template< typename T >
class RevRTAllocPolicy {
public:
  RevRTAllocPolicy< T >()  = default;
  ~RevRTAllocPolicy< T >() = default;

  T* CreateMemMgr( ud_machine_t parms ) {
    T* p = reinterpret_cast< T* >( rev_mmap( 0,
                                             sizeof( T ),
                                             PROT_READ | PROT_WRITE | PROT_EXEC,
                                             MAP_PRIVATE | MAP_ANONYMOUS,
                                             -1,
                                             0 ) );
    //p = new(p) T(std::forward<ud_machine_t>(parms));
    p    = new( p ) T( parms );
    return p;
  }

  void DestroyMemMgr( T* p ) {
    std::size_t addr = reinterpret_cast< std::size_t >( p );
    rev_munmap( addr, sizeof( T ) );
  }
};

template< typename T >
class StdRTAllocPolicy {
public:
  StdRTAllocPolicy< T >()  = default;
  ~StdRTAllocPolicy< T >() = default;

  T* CreateMemMgr( ud_machine_t parms ) {
    return new T( parms );
  }

  void DestroyMemMgr( T* p ) {
    delete( p );
  }
};

/**
 * @brief Class that defines local memory segments based on global segments
 *
 */


/**
 * @brief Class containing the UpDown Runtime
 *
 * This class is the entry point of the UpDown runtime, it manages all
 * the necessary state, keeps track of pointers and memory allocation
 * and coordinates execution of code.
 *
 */

// Moved out some inner classes
// class UDRuntime_t {
// private:
/**
   * @brief Struct containing all the base addresses
   *
   * Base addresses are locations in memory where the
   * top to updown communication happens.
   *
   * ## Current Memory Structure
   * The memory is organized in two: Scratchpad memory and
   * Control mapped registers and queues
   *
   * ### Scratchpad memory address space
   * \verbatim
   *        MEMORY                     SIZE
   *   |--------------|  <-- Scratchpad Base Address
   *   |              |
   *   |   SPMEM UD0  |
   *   |              |
   *   |--------------|  <-- CapacityPerLane * CapacityNumLanes
   *   |              |
   *   |   SPMEM UD1  |
   *   |              |
   *   |--------------|  <-- 2 * CapacityPerLane * CapacityNumLanes
   *   |      ...     |
   *   |--------------|
   *   |              |
   *   |   SPMEM UDN  |
   *   |              |
   *   |--------------|  <-- NumUDs * CapacityPerLane * CapacityNumLanes
   * \endverbatim
   *
   * ### Scratchpad memory for 1 UD
   * \verbatim
   *        MEMORY                     SIZE
   *   |--------------|  <-- Scratchpad Base Address
   *   |              |
   *   |    Lane 0    |
   *   |              |
   *   |* * * * * * * |  <-- SPmem BankSize
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- CapacityPerLane
   *   |              |
   *   |    Lane 1    |
   *   |              |
   *   |* * * * * * * |  <-- CapacityPerLane + SPmem BankSize
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- 2 * CapacityPerLane
   *   |      ...     |
   *   |--------------|  <-- (NumLanes-1)*CapacityPerLane
   *   |              |
   *   |    Lane N    |
   *   |              |
   *   |* * * * * * * |  <-- (NumLanes-1)*CapacityPerLane + SPmem BankSize
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- (NumLanes) * CapacityPerLane
   *   | * * * * * * *|
   *   |* * * * * * * |   -|
   *   | * * * * * * *|    | For expansion purposes
   *   |* * * * * * * |   -|
   *   | * * * * * * *|
   *   |--------------|  <-- CapacityPerLane * CapacityNumLanes
   * \endverbatim
   *
   * ### Control signals memory address space
   * \verbatim
   *        MEMORY                     SIZE
   *   |--------------|  <-- Control Base Address
   *   |              |
   *   |  CONTROL UD0 |
   *   |              |
   *   |--------------|  <-- CapacityControlPerLane * CapacityNumLanes
   *   |              |
   *   |  CONTROL UD1 |
   *   |              |
   *   |--------------|  <-- 2 * CapacityControlPerLane * CapacityNumLanes
   *   |      ...     |
   *   |--------------|
   *   |              |
   *   |  CONTROL UD2 |
   *   |              |
   *   |--------------|  <-- NumUDs * CapacityControlPerLane * CapacityNumLanes
   * \endverbatim
   *
   * ### Control signals memory address space for 1 UD
   * \verbatim
   *        MEMORY                     SIZE
   *   |--------------|  <-- Control Base Address
   *   |    Lane 0    |
   *   |  Event Queue |
   *   | Oprnds Queue |
   *   |  Start Exec  |
   *   |     Lock     |
   *   |* * * * * * * |   -|
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- CapacityControlPerLane
   *   |    Lane 1    |
   *   |  Event Queue |
   *   | Oprnds Queue |
   *   |  Start Exec  |
   *   |     Lock     |
   *   |* * * * * * * |   -|
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- 2 * CapacityControlPerLane
   *   |      ...     |
   *   |--------------|  <-- (NumLanes-1) * CapacityControlPerLane
   *   |    Lane N    |
   *   |  Event Queue |
   *   | Oprnds Queue |
   *   |  Start Exec  |
   *   |     Lock     |
   *   |* * * * * * * |   -|
   *   | * * * * * * *|    |
   *   |* * * * * * * |    | For expansion purposes
   *   | * * * * * * *|    |
   *   |--------------|  <-- NumLanes * CapacityControlPerLane
   *   | * * * * * * *|
   *   |* * * * * * * |   -|
   *   | * * * * * * *|    | For expansion purposes
   *   |* * * * * * * |   -|
   *   | * * * * * * *|
   *   |--------------|  <-- CapacityControlPerLane * CapacityNumLanes
   * \endverbatim
   *
   */
// retained in class
// struct base_addr_t {
//   volatile ptr_t mmaddr;
//   volatile ptr_t spaddr;
//   volatile ptr_t ctrlAddr;
//   volatile ptr_t progAddr;
// };

/**
 * @brief This is a class to manage the mapped memory
 *
 * This class cosiders memory allocation and deallocation
 * it contains all the information needed to do this.
 */
class ud_mapped_memory_t {
private:
  /**
     * @brief A region in memory.
     *
     * This struct represents a region in memory, full or empty
     *
     */
  struct mem_region_t {
    uint64_t size;  // Size of the region
    bool     free;  // Is the region free or being used
  };

  // Map to keep track of the free and used regions.
  //std::map<void *, mem_region_t> regions_local;  // local segments
  //std::map<void *, mem_region_t> regions_global; // global segments area
  std::map< void*,
            mem_region_t,
            std::less< void* >,
            Allocator< std::pair< const void*, mem_region_t > > >
    regions_local;  // local segments
  std::map< void*,
            mem_region_t,
            std::less< void* >,
            Allocator< std::pair< const void*, mem_region_t > > >
    regions_global;  // global segments area

public:
  /**
     * @brief Construct a new ud_mapped_memory_t object
     *
     * Initializes the regions with a single region
     * that is free and contains all the elements
     *
     * @param machine information from the description of the current machine
     */
  ud_mapped_memory_t( ud_machine_t& machine ) {
    UPDOWN_INFOMSG( "Initializing Mapped Memory Manager for %lu at 0x%lX",
                    machine.MapMemSize,
                    machine.MapMemBase );
    // Create the first segment
    void* baseAddr           = reinterpret_cast< void* >( machine.MapMemBase );
    regions_local[baseAddr]  = { machine.MapMemSize, true };
    baseAddr                 = reinterpret_cast< void* >( machine.GMapMemBase );
    regions_global[baseAddr] = { machine.GMapMemSize, true };
  }

  /**
     * @brief Get a new region in memory
     *
     * This is equivalent to malloc, it finds a free region that
     * can allocate the current size, and, if there is free space
     * left, it creates a new region with the remaining free space.
     *
     * @param size size to be allocated in bytes.
     * @return void* pointer to the allocated memory
     */
  void* get_region( uint64_t size, bool global ) {
    UPDOWN_INFOMSG( "Allocating new region of size %lu bytes", size );
    // Iterate over the regions finding one that fits

    //std::map<void *, mem_region_t> *regions_to_use;
    std::map< void*,
              mem_region_t,
              std::less< void* >,
              Allocator< std::pair< const void*, mem_region_t > > >*
      regions_to_use;
    if( global )
      regions_to_use = &regions_global;
    else
      regions_to_use = &regions_local;
    auto  used_reg     = regions_to_use->end();
    void* aligned_addr = nullptr;
    for( auto it = regions_to_use->begin(); it != regions_to_use->end();
         ++it ) {
      // Check if region is free and have enough size
      if( it->second.free && size <= it->second.size ) {
        // Check alignment
        aligned_addr = it->first;
        if( reinterpret_cast< uint64_t >( aligned_addr ) % 8 != 0 ) {
          uint64_t alignment_offset =
            8 - ( reinterpret_cast< uint64_t >( aligned_addr ) % 8 );
          if( it->second.size >= size + alignment_offset ) {
            aligned_addr =
              static_cast< char* >( aligned_addr ) + alignment_offset;
          } else {
            continue;
          }
        }

        UPDOWN_INFOMSG( "Found a region at 0x%lX",
                        reinterpret_cast< uint64_t >( it->first ) );
        used_reg = it;
        break;
      }
    }
    // Check if we found space
    if( used_reg == regions_to_use->end() ) {
      UPDOWN_ERROR( "Allocator run out of memory. Cannot allocate %lu bytes",
                    size );
      return nullptr;
    }

    // Calculate alignment offset
    uint64_t alignment_offset = reinterpret_cast< uint64_t >( aligned_addr ) -
                                reinterpret_cast< uint64_t >( used_reg->first );
    void* alloc_addr = nullptr;

    // needs to be aligned
    if( alignment_offset != 0 ) {
      ( *regions_to_use )[aligned_addr] = { size, false };
      UPDOWN_INFOMSG( "Creating a new region 0x%lX with the size %lu for new "
                      "aligned allocation",
                      reinterpret_cast< uint64_t >( aligned_addr ),
                      size );
      alloc_addr = aligned_addr;
      if( used_reg->second.size - alignment_offset > size ) {
        // Create a new empty region for unused space
        void* new_region_addr = static_cast< char* >( aligned_addr ) + size;
        ( *regions_to_use )[new_region_addr] = {
          used_reg->second.size - alignment_offset - size, true };
        UPDOWN_INFOMSG(
          "Creating a new region 0x%lX with the remaining size %lu",
          reinterpret_cast< uint64_t >( new_region_addr ),
          used_reg->second.size - alignment_offset - size );
      }
      // Create a new empty region for alignment
      used_reg->second.size = alignment_offset;
      used_reg->second.free = true;
    } else {
      // no alignment needed
      // split the new region
      uint64_t new_size     = used_reg->second.size - size;
      used_reg->second.size = size;
      used_reg->second.free = false;
      alloc_addr            = used_reg->first;
      // Create a new empty region
      if( new_size != 0 ) {
        void* new_region_addr = static_cast< char* >( used_reg->first ) + size;
        ( *regions_to_use )[new_region_addr] = { new_size, true };
        UPDOWN_INFOMSG(
          "Creating a new region 0x%lX with the remaining size %lu",
          reinterpret_cast< uint64_t >( new_region_addr ),
          new_size );
      }
    }

    UPDOWN_INFOMSG( "Returning region 0x%lX = {%lu, Used}",
                    reinterpret_cast< uint64_t >( alloc_addr ),
                    size );
    // Return the new pointer
    return alloc_addr;
  }

  /**
     * @brief Get a new region in memory at a specific address
     *
     * @param addr address to allocate the region
     * @param size size to be allocated in bytes
     * @param global if the region is global or local
     * @return void* pointer to the allocated memory
     */
  void* get_region_at_addr( void* addr, uint64_t size, bool global ) {
#ifndef ASST
    UPDOWN_INFOMSG(
      "Trying reserve region at %p of size %lu bytes", addr, size );
#else
    UPDOWN_INFOMSG( "Trying reserve region at 0x%lx of size %lu bytes",
                    reinterpret_cast< uint64_t >( addr ),
                    size );
#endif
    // Iterate over the regions finding one that fits
    //std::map<void *, mem_region_t> *alloc_regions;
    std::map< void*,
              mem_region_t,
              std::less< void* >,
              Allocator< std::pair< const void*, mem_region_t > > >*
      alloc_regions;
    if( global )
      alloc_regions = &regions_global;
    else
      alloc_regions = &regions_local;
    auto sel_region = alloc_regions->end();
    for( auto it = alloc_regions->begin(); it != alloc_regions->end(); ++it ) {
      // Check if region is free and have enough size
      if( it->first <= addr && it->second.free && size <= it->second.size ) {
        UPDOWN_INFOMSG( "Found a region at 0x%lX",
                        reinterpret_cast< uint64_t >( it->first ) );
        sel_region = it;
      } else if( it->first > addr ) {
        break;
      }
    }
    // Check if we found space
    if( sel_region == alloc_regions->end() ) {
      UPDOWN_ERROR( "Allocator cannot allocate %lu bytes at %p", size, addr );
      return nullptr;
    }
    // split the new region
    if( sel_region->first < addr ) {
      // with leading free region
      uint64_t sel_region_size = sel_region->second.size;
      // change the size of the leading free region
      sel_region->second.size =
        reinterpret_cast< uint64_t >( addr ) -
        reinterpret_cast< uint64_t >( sel_region->first );
      // create new occupied region
      ( *alloc_regions )[addr] = { size, false };
      UPDOWN_INFOMSG( "Creating allocated region 0x%lX = {%lu, Used}",
                      reinterpret_cast< uint64_t >( addr ),
                      size );
      // create new free region if there is space left
      uint64_t new_size = sel_region_size - size - sel_region->second.size;
      if( new_size != 0 ) {
        void* new_reg               = static_cast< char* >( addr ) + size;
        ( *alloc_regions )[new_reg] = { new_size, true };
        UPDOWN_INFOMSG(
          "Creating a new free region 0x%lX with the remaining size %lu",
          reinterpret_cast< uint64_t >( new_reg ),
          new_size );
      }
    } else {
      // no leading free region
      uint64_t sel_region_size = sel_region->second.size;
      // change the size of the occupied region
      sel_region->second.size  = size;
      sel_region->second.free  = false;
      // create new free region if there is space left
      uint64_t new_size        = sel_region_size - size;
      if( new_size != 0 ) {
        void* new_reg = static_cast< char* >( sel_region->first ) + size;
        ( *alloc_regions )[new_reg] = { new_size, true };
        UPDOWN_INFOMSG(
          "Creating a new free region 0x%lX with the remaining size %lu",
          reinterpret_cast< uint64_t >( new_reg ),
          new_size );
      }
    }
    return addr;
  }

  /**
     * @brief Remove a region
     *
     * This is equivalent to free. It removes a region from the map
     * and extends the region before or after this one if they are free.
     * Otherwise it creates a new free region
     *
     * @param ptr Pointer to be free. It must be a pointer in the regions map
     *
     */
  void remove_region( void* ptr, bool global ) {
    UPDOWN_INFOMSG( "Freeing the space at 0x%lX",
                    reinterpret_cast< uint64_t >( ptr ) );
    // Find location to free
    //std::map<void *, mem_region_t> *regions_to_use;
    std::map< void*,
              mem_region_t,
              std::less< void* >,
              Allocator< std::pair< const void*, mem_region_t > > >*
      regions_to_use;
    if( global )
      regions_to_use = &regions_global;
    else
      regions_to_use = &regions_local;
    auto it = regions_to_use->find( ptr );
    if( it == regions_to_use->end() || it->second.free ) {
      UPDOWN_ERROR( "Trying to free pointer 0x%lX that is not in the regions"
                    " or the region is free (double free?)",
                    reinterpret_cast< uint64_t >( ptr ) );
      return;
    }
    // merge left if free
    if( it != regions_to_use->begin() && std::prev( it, 1 )->second.free ) {
      uint64_t size = it->second.size;
      it--;
      it->second.size += size;
      UPDOWN_INFOMSG( "Merging left 0x%lX to 0x%lX, adding %lu for a total of "
                      "region with %lu",
                      reinterpret_cast< uint64_t >( it->first ),
                      reinterpret_cast< uint64_t >( std::next( it, 1 )->first ),
                      size,
                      it->second.size );
      UPDOWN_INFOMSG( "Removing previous region at 0x%lX",
                      reinterpret_cast< uint64_t >( it->first ) );
      regions_to_use->erase( std::next( it, 1 ) );
    }
    // merge right if free
    auto nextIt = std::next( it, 1 );
    if( nextIt != regions_to_use->end() && nextIt->second.free ) {
      uint64_t size = nextIt->second.size;
      it->second.size += size;
      UPDOWN_INFOMSG( "Merging right 0x%lX to 0x%lX, adding %lu for a total of "
                      "region with %lu",
                      reinterpret_cast< uint64_t >( it->first ),
                      reinterpret_cast< uint64_t >( nextIt->first ),
                      size,
                      it->second.size );
      UPDOWN_INFOMSG( "Removing previous region at 0x%lX",
                      reinterpret_cast< uint64_t >( it->first ) );
      regions_to_use->erase( nextIt );
    }

    // Check if there were no merges
    if( it->first == ptr ) {
#ifndef ASST
      UPDOWN_INFOMSG( "No merges performed, just freeing 0x%lX = {%lu, %s}",
                      reinterpret_cast< uint64_t >( it->first ),
                      ( it->second.size ),
                      ( it->second.free ) ? "Free" : "Used" );
#else
      if( it->second.free ) {
        UPDOWN_INFOMSG( "No merges performed, just freeing 0x%lX = {%lu, Free}",
                        reinterpret_cast< uint64_t >( it->first ),
                        ( it->second.size ) );
      } else {
        UPDOWN_INFOMSG( "No merges performed, just freeing 0x%lX = {%lu, Used}",
                        reinterpret_cast< uint64_t >( it->first ),
                        ( it->second.size ) );
      }
#endif
      it->second.free = true;
    }
  }
};

// moved from above to avoid inner classes
template< class AllocationPolicy >
class UDRuntime_t : public AllocationPolicy {
private:
  struct base_addr_t {
    volatile ptr_t mmaddr;
    volatile ptr_t spaddr;
    volatile ptr_t ctrlAddr;
    volatile ptr_t progAddr;
  };

  /**
   * @brief This is a class to manage the mapped memory
   *
   * This class cosiders memory allocation and deallocation
   * it contains all the information needed to do this.
   */

protected:
  /**
   * @brief container for all base addresses
   *
   */
  base_addr_t         BaseAddrs;

  /**
   * @brief Contains configuration parameters of the machine abstraction
   *
   */
  ud_machine_t        MachineConfig;

  /**
   * @brief
   *
   */
  ud_mapped_memory_t* MappedMemoryManager;

  /**
   * @brief Initializes the base addresses for queues and states
   *
   * This function initializes BaseAddrs according to the configuration
   * file for their use in the rest of the runtime
   *
   */
  void                calc_addrmap();

  /**
   * @brief Get the aligned offset object
   *
   * @param ud_id UpDown ID
   * @param lane_num Lane ID
   * @param offset Offset in bytes
   * @return uint64_t
   */
  uint64_t    get_lane_aligned_offset( networkid_t nwid, uint32_t offset = 0 );

  /**
   * @brief Get the global UD number based on the network_id
   *
   * @param nid Network ID
   * @return uint32_t
   */
  uint32_t    get_globalUDNum( networkid_t& nid );

  /**
   * @brief Reset the Memory Manager when machine configuration is changed;
   *
   */

  inline void reset_memory_manager() {
    AllocationPolicy::DestroyMemMgr( MappedMemoryManager );
    //delete MappedMemoryManager;
    MappedMemoryManager = AllocationPolicy::CreateMemMgr( this->MachineConfig );
  }

public:
  UDRuntime_t() {
    UPDOWN_INFOMSG( "Initializing runtime" );
    calc_addrmap();
    MappedMemoryManager = AllocationPolicy::CreateMemMgr( this->MachineConfig );
    //MappedMemoryManager = new ud_mapped_memory_t(this->MachineConfig);
  }

  UDRuntime_t( ud_machine_t machineConfig ) : MachineConfig( machineConfig ) {
    UPDOWN_INFOMSG( "Initializing runtime with custom machineConfig" );
    calc_addrmap();
    MappedMemoryManager = AllocationPolicy::CreateMemMgr( this->MachineConfig );
    //MappedMemoryManager = new ud_mapped_memory_t(this->MachineConfig);
  }

  ~UDRuntime_t() {
    AllocationPolicy::DestroyMemMgr( MappedMemoryManager );
  }

  /**
   * @brief Sends an event to the UpDown
   *
   * Sends a new event to the updown event queues. Information about the
   * destination of the event is defined in the ev parameter, including
   * destination UD_ID, lane_id, thread_id. The event_t also contains the
   * operands
   *
   * @param ev the event to be sent
   */
  virtual void send_event( event_t ev );

  /**
   * @brief Signal lane to start execution
   *
   * @param ud_id UpDown ID
   * @param lane_num lane to signal
   */
  virtual void start_exec( networkid_t nwid );

  /**
   * @brief Memory allocator for the mapped memory
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @todo: This should be thread safe?
   *
   * @param size size in bytes
   * @return void * Pointer to the location.
   */

  void*        mm_malloc( uint64_t size );
  void*        mm_malloc_at_addr( void* addr, uint64_t size );

  /**
   * @brief Free an already existing memory allocation
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @todo: This should be thread safe?
   *
   * @param ptr pointer to deallocate
   */

  void         mm_free( void* ptr );

  /**
   * @brief Memory allocator for the mapped memory
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @todo: This should be thread safe?
   *
   * @param size size in bytes
   * @return void * Pointer to the location.
   */

  void*        mm_malloc_global( uint64_t size );
  void*        mm_malloc_global_at_addr( void* addr, uint64_t size );

  /**
   * @brief Free an already existing memory allocation in global segments
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @todo: This should be thread safe?
   *
   * @param ptr pointer to deallocate
   */

  void         mm_free_global( void* ptr );

  /**
   * @brief Copy to mapped memory from top in global segments
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @param offset Destination offset within the memory mapped region
   * @param src Source pointer
   * @param size size in bytes
   */

  void         t2mm_memcpy( uint64_t offset, void* src, uint64_t size = 1 );

  /**
   * @brief Copy from mapped memory to top
   *
   * Mapped memory is a region in DRAM that can be accessed
   * from updown. This is necessary due to the lack of support for
   * virtual memories in the UpDown. The UpDown
   * will be able to use pointers directly into this region
   * without need for address translation.
   *
   * @param offset Destination offset within the memory mapped region
   * @param dst Source pointer
   * @param size size in bytes
   */

  void         mm2t_memcpy( uint64_t offset, void* dst, uint64_t size = 1 );

  /**
   * @brief Copy data from the top to the scratchpad memory
   *
   * This function sends data to the bank associated with the lane_num
   * The address is calculated based on the offset.
   *
   * @param data pointer to the top data to be copied over to the lane
   * @param size The number of bytes to be copied to the scratchpad
   * memory
   * @param ud_id UpDown number
   * @param lane_num lane bank
   * @param offset offset within bank in bytes
   */
  virtual void
    t2ud_memcpy( void* data, uint64_t size, networkid_t nwid, uint32_t offset );

  /**
   * @brief Copy data from the scratchpad memory to the top
   *
   * @param data pointer to the top data to be contain values from the
   * scratchpad memory
   * @param size The number of bytes to be copied from the scratchpad
   * memory
   * @param ud_id UpDown number
   * @param lane_num lane bank
   * @param offset offset within bank in bytes
   */
  virtual void
    ud2t_memcpy( void* data, uint64_t size, networkid_t nwid, uint32_t offset );

  /**
   * @brief Test a memory location in the updwon bank for the expected value
   *
   * Reads the updown scratchpad memory bank, and check if the value is equal to
   * the expected value. Returns true if the values are equal
   *
   * @param ud_id UpDown number
   * @param lane_num LaneID to check
   * @param offset offset within the memory bank in bytes
   * @param expected value that is expected in the memory bank
   * @return word_t
   */
  virtual bool
    test_addr( networkid_t nwid, uint32_t offset, word_t expected = 1 );

  /**
   * @brief Test a memory location in the updown bank for the expected value.
   * Wait until it is the expected value
   *
   * Spinwaiting on a location of the updown scratchpad memory bank until the
   * value read is the expected value. Uses lane_test_memory.
   *
   * @param lane_num LaneID to check
   * @param offset offset within the memory bank in bytes
   * @param expected value that is expected in the memory bank
   */
  virtual void
    test_wait_addr( networkid_t nwid, uint32_t offset, word_t expected = 1 );

  /**
   * @brief Get offset in local memory
   *
   * This is a temporary solution to the absence of an appropriate virtual
   * memory mapping. This functions assumes that physical memory side is
   * contiguous and it does not have space for expansion. This is, instead
   * of using the Scratchpad Capacity that is explained in
   * UDRuntime_t::base_addr_t, it uses the actual size of the Scratchpad Memory.
   *
   * Currently, this function is used to allow pointers to be passed from top
   * to updown.
   *
   * ### Scratchpad physical memory for 1 UD
   * \verbatim
   *        MEMORY                     SIZE
   *   |--------------|  <-- Scratchpad Base Address
   *   |              |
   *   |    Lane 0    |
   *   |              |
   *   |--------------|  <-- SPmem BankSize
   *   |              |
   *   |    Lane 1    |
   *   |              |
   *   |--------------|  <-- 2 * SPmem BankSize
   *   |      ...     |
   *   |--------------|  <-- (NumLanes-1) * SPmem BankSize
   *   |              |
   *   |    Lane N    |
   *   |              |
   *   |--------------|  <-- (NumLanes) * SPmem BankSize
   * \endverbatim
   *
   * @param ud_id UpDown ID
   * @param lane_num Lane ID
   * @param offset Offset in bytes
   * @return uint64_t
   */
  uint64_t get_lane_physical_memory( networkid_t nwid, uint32_t offset = 0 );

  /**
   * @brief Get the ud physical memory object
   *
   * @param nwid
   * @return uint64_t
   */
  uint64_t get_ud_physical_memory( networkid_t nwid );

  /**
   * @brief Helper function to dump current base addresses
   *
   * This function has no effect on runtime, but it is useful for debugging
   *
   */
  void     dumpBaseAddrs() {
#ifndef ASST
    printf(
#else
    rev_udrt_print(
#endif
      "  mmaddr    = 0x%lX\n"
      "  spaddr    = 0x%lX\n"
      "  ctrlAddr  = 0x%lX\n"
      "  progAddr  = 0x%lX\n",
      reinterpret_cast< uint64_t >( BaseAddrs.mmaddr ),
      reinterpret_cast< uint64_t >( BaseAddrs.spaddr ),
      reinterpret_cast< uint64_t >( BaseAddrs.ctrlAddr ),
      reinterpret_cast< uint64_t >( BaseAddrs.progAddr ) );
  }

  /**
   * @brief Function to return the current Machine config
   *
   */
  ud_machine_t getMachineConfig() {
    return MachineConfig;
  }

  /**
   * @brief Function to dump the memory into a file
   * @param filename file to be dumped into
   * @param vaddr Start vaddr
   * @param size size of memory to be dumped
  */
  virtual void dumpMemory( const char* filename, void* vaddr, uint64_t size );

  /**
   * @brief Function to load memory from a file
   * @param filename file to be dumped into
   * @param vaddr Start vaddr
   * @param size size of memory to be dumped
   * @return std::pair<void *, uint64_t> pointer to the memory and size
  */
  virtual std::pair< void*, uint64_t > loadMemory( const char* filename,
                                                   void*       vaddr = nullptr,
                                                   uint64_t    size  = 0 );

  /**
   * @brief Function to dump the memory into a file
   * @param vaddr Start vaddr
   * @param size size of memory to be dumped
   * @param filename file to be dumped into
  */
  virtual void                         dumpLocalMemory( const char* filename,
                                                        networkid_t start_nwid = networkid_t(),
                                                        uint64_t    num_lanes = 0 );

  /**
   * @brief Function to load memory from a file
   * @param vaddr Start vaddr
   * @param size size of memory to be dumped
   * @param filename file to be dumped into
   * @return std::pair<networkid_t, uint64_t> load networkid and number of lanes
  */
  virtual std::pair< networkid_t, uint64_t >
       loadLocalMemory( const char* filename,
                        networkid_t start_nwid = networkid_t(),
                        uint64_t    num_lanes  = 0 );

  /**
   * @brief Helper function to dump current Machine Config
   *
   * This function has no effect on runtime, but it is useful for debugging
   */
  void dumpMachineConfig() {
#ifndef ASST
    printf( "  MapMemBase          = 0x%lX\n"
            "  GMapMemBase         = 0x%lX\n"
            "  UDbase              = 0x%lX\n"
            "  SPMemBase           = 0x%lX\n"
            "  ControlBase         = 0x%lX\n"
            "  ProgBase            = 0x%lX\n"
            "  EventQueueOffset    = (0x%lX)%lu\n"
            "  OperandQueueOffset  = (0x%lX)%lu\n"
            "  StartExecOffset     = (0x%lX)%lu\n"
            "  LockOffset          = (0x%lX)%lu\n"
            "  CapNumNodes         = (0x%lX)%lu\n"
            "  CapNumStacks        = (0x%lX)%lu\n"
            "  CapNumUDs           = (0x%lX)%lu\n"
            "  CapNumLanes         = (0x%lX)%lu\n"
            "  CapSPmemPerLane     = (0x%lX)%lu\n"
            "  CapControlPerLane   = (0x%lX)%lu\n"
            "  NumNodes            = (0x%lX)%lu\n"
            "  NumStacks           = (0x%lX)%lu\n"
            "  NumUDs              = (0x%lX)%lu\n"
            "  NumLanes            = (0x%lX)%lu\n"
            "  MapMemSize          = (0x%lX)%lu\n"
            "  GMapMemSize         = (0x%lX)%lu\n"
            "  SPBankSizse         = (0x%lX)%lu\n"
            "  SPBankSizeWords     = (0x%lX)%lu\n",
            MachineConfig.MapMemBase,
            MachineConfig.GMapMemBase,
            MachineConfig.UDbase,
            MachineConfig.SPMemBase,
            MachineConfig.ControlBase,
            MachineConfig.ProgBase,
            MachineConfig.EventQueueOffset,
            MachineConfig.EventQueueOffset,
            MachineConfig.OperandQueueOffset,
            MachineConfig.OperandQueueOffset,
            MachineConfig.StartExecOffset,
            MachineConfig.StartExecOffset,
            MachineConfig.LockOffset,
            MachineConfig.LockOffset,
            MachineConfig.CapNumNodes,
            MachineConfig.CapNumNodes,
            MachineConfig.CapNumStacks,
            MachineConfig.CapNumStacks,
            MachineConfig.CapNumUDs,
            MachineConfig.CapNumUDs,
            MachineConfig.CapNumLanes,
            MachineConfig.CapNumLanes,
            MachineConfig.CapSPmemPerLane,
            MachineConfig.CapSPmemPerLane,
            MachineConfig.CapControlPerLane,
            MachineConfig.CapControlPerLane,
            MachineConfig.NumNodes,
            MachineConfig.NumNodes,
            MachineConfig.NumStacks,
            MachineConfig.NumStacks,
            MachineConfig.NumUDs,
            MachineConfig.NumUDs,
            MachineConfig.NumLanes,
            MachineConfig.NumLanes,
            MachineConfig.MapMemSize,
            MachineConfig.MapMemSize,
            MachineConfig.GMapMemSize,
            MachineConfig.GMapMemSize,
            MachineConfig.SPBankSize,
            MachineConfig.SPBankSize,
            MachineConfig.SPBankSizeWords,
            MachineConfig.SPBankSizeWords );
#else
    rev_udrt_print( "  MapMemBase          = 0x%lX\n"
                    "  GMapMemBase         = 0x%lX\n"
                    "  UDbase              = 0x%lX\n"
                    "  SPMemBase           = 0x%lX\n"
                    "  ControlBase         = 0x%lX\n"
                    "  ProgramBase         = 0x%lX\n",
                    MachineConfig.MapMemBase,
                    MachineConfig.GMapMemBase,
                    MachineConfig.UDbase,
                    MachineConfig.SPMemBase,
                    MachineConfig.ControlBase,
                    MachineConfig.ProgBase );
    rev_udrt_print( "  EventQueueOffset    = (0x%lX)%lu\n"
                    "  OperandQueueOffset  = (0x%lX)%lu\n"
                    "  StartExecOffset     = (0x%lX)%lu\n",
                    MachineConfig.EventQueueOffset,
                    MachineConfig.EventQueueOffset,
                    MachineConfig.OperandQueueOffset,
                    MachineConfig.OperandQueueOffset,
                    MachineConfig.StartExecOffset,
                    MachineConfig.StartExecOffset );
    rev_udrt_print( "  LockOffset          = (0x%lX)%lu\n"
                    "  CapNumNodes         = (0x%lX)%lu\n"
                    "  CapNumStacks        = (0x%lX)%lu\n",
                    MachineConfig.LockOffset,
                    MachineConfig.LockOffset,
                    MachineConfig.CapNumNodes,
                    MachineConfig.CapNumNodes,
                    MachineConfig.CapNumStacks,
                    MachineConfig.CapNumStacks );
    rev_udrt_print( "  CapNumUDs           = (0x%lX)%lu\n"
                    "  CapNumLanes         = (0x%lX)%lu\n"
                    "  CapSPmemPerLane     = (0x%lX)%lu\n",
                    MachineConfig.CapNumUDs,
                    MachineConfig.CapNumUDs,
                    MachineConfig.CapNumLanes,
                    MachineConfig.CapNumLanes,
                    MachineConfig.CapSPmemPerLane,
                    MachineConfig.CapSPmemPerLane );
    rev_udrt_print( "  CapControlPerLane   = (0x%lX)%lu\n"
                    "  NumNodes            = (0x%lX)%lu\n"
                    "  NumStacks           = (0x%lX)%lu\n",
                    MachineConfig.CapControlPerLane,
                    MachineConfig.CapControlPerLane,
                    MachineConfig.NumNodes,
                    MachineConfig.NumNodes,
                    MachineConfig.NumStacks,
                    MachineConfig.NumStacks );
    rev_udrt_print( "  NumUDs              = (0x%lX)%lu\n"
                    "  NumLanes            = (0x%lX)%lu\n"
                    "  MapMemSize          = (0x%lX)%lu\n",
                    MachineConfig.NumUDs,
                    MachineConfig.NumUDs,
                    MachineConfig.NumLanes,
                    MachineConfig.NumLanes,
                    MachineConfig.MapMemSize,
                    MachineConfig.MapMemSize );
    rev_udrt_print( "  GMapMemSize         = (0x%lX)%lu\n"
                    "  SPBankSize          = (0x%lX)%lu\n"
                    "  SPBankSizeWords     = (0x%lX)%lu\n",
                    MachineConfig.GMapMemSize,
                    MachineConfig.GMapMemSize,
                    MachineConfig.SPBankSize,
                    MachineConfig.SPBankSize,
                    MachineConfig.SPBankSizeWords,
                    MachineConfig.SPBankSizeWords );
#endif
  }
};  //class UDRuntime_t

/*
 * updown.cpp
 */

// TODO: This should not be offset with the number of elements. Best to
// determine a fixed memory location for each section
template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::calc_addrmap() {
  BaseAddrs.mmaddr   = (ptr_t) MachineConfig.MapMemBase;
  BaseAddrs.spaddr   = (ptr_t) MachineConfig.SPMemBase;
  BaseAddrs.ctrlAddr = (ptr_t) MachineConfig.ControlBase;
  BaseAddrs.progAddr = (ptr_t) MachineConfig.ProgBase;
  UPDOWN_INFOMSG( "calc_addrmap: maddr: 0x%lX spaddr: 0x%lX ctrlAddr: 0x%lX",
                  reinterpret_cast< uint64_t >( BaseAddrs.mmaddr ),
                  reinterpret_cast< uint64_t >( BaseAddrs.spaddr ),
                  reinterpret_cast< uint64_t >( BaseAddrs.ctrlAddr ),
                  reinterpret_cast< uint64_t >( BaseAddrs.progAddr ) );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::send_event( event_t ev ) {
  uint64_t offset =
    ( ( ( ev.get_NetworkId() ).get_NodeId() * MachineConfig.CapNumStacks +
        ( ev.get_NetworkId() ).get_StackId() ) *
        MachineConfig.CapNumUDs +
      ( ev.get_NetworkId() ).get_UdId() ) *
      MachineConfig.CapNumLanes * MachineConfig.CapControlPerLane +
    ( ev.get_NetworkId() ).get_LaneId() * MachineConfig.CapControlPerLane;
  // Convert from bytes to words. the pointers are ptr_t
  offset /= sizeof( word_t );
  // Locking the lane's queues
  auto lock = ( BaseAddrs.ctrlAddr + offset + MachineConfig.LockOffset );
  UPDOWN_INFOMSG( "Locking 0x%lX", reinterpret_cast< uint64_t >( lock ) );
  *lock = 1;
  // Set the event Queue

  // TODO: Num Operands should reflect the continuation, this should not be a +
  // 1 in the for
  auto OpsQ =
    ( BaseAddrs.ctrlAddr + offset + MachineConfig.OperandQueueOffset );
  UPDOWN_INFOMSG( "Using Operands Queue 0x%lX",
                  reinterpret_cast< uint64_t >( OpsQ ) );
  if( ev.get_NumOperands() != 0 )
    for( uint8_t i = 0; i < ev.get_NumOperands() + 1; i++ ) {
      *( OpsQ ) = ev.get_OperandsData()[i];
      UPDOWN_INFOMSG( "OB[%u]: %lu (0x%lX)",
                      i,
                      ev.get_OperandsData()[i],
                      ev.get_OperandsData()[i] );
    }
  UPDOWN_INFOMSG( "Unlocking 0x%lX", reinterpret_cast< uint64_t >( lock ) );
  auto eventQ =
    ( BaseAddrs.ctrlAddr + offset + MachineConfig.EventQueueOffset );
#ifndef ASST
  UPDOWN_INFOMSG( "Sending Event:%u to [%u, %u, %u, %u, %u] to queue at 0x%lX",
                  ev.get_EventLabel(),
                  ev.get_NetworkId().get_NodeId(),
                  ev.get_NetworkId().get_StackId(),
                  ev.get_NetworkId().get_UdId(),
                  ev.get_NetworkId().get_LaneId(),
                  ev.get_ThreadId(),
                  reinterpret_cast< uint64_t >( eventQ ) );
#else
  UPDOWN_INFOMSG( "Sending Event:%u to [%u, %u, %u, %u] to queue at 0x%lX",
                  ev.get_EventLabel(),
                  ev.get_NetworkId().get_NodeId(),
                  ev.get_NetworkId().get_StackId(),
                  ev.get_NetworkId().get_UdId(),
                  ev.get_NetworkId().get_LaneId(),
                  reinterpret_cast< uint64_t >( eventQ ) );
  UPDOWN_INFOMSG( "Sending Event to threadID %u\n", ev.get_ThreadId() );
#endif
  *eventQ   = ev.get_EventWord();
  *( lock ) = 0;

  // Set the Operand Queue
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::start_exec( networkid_t nwid ) {
  uint8_t  ud_id    = nwid.get_UdId();
  uint8_t  lane_id  = nwid.get_LaneId();
  uint8_t  stack_id = nwid.get_StackId();
  uint8_t  node_id  = nwid.get_NodeId();
  uint64_t offset =
    ( node_id * MachineConfig.CapNumStacks * MachineConfig.CapNumUDs +
      stack_id * MachineConfig.CapNumUDs + ud_id ) *
      MachineConfig.CapNumLanes * MachineConfig.CapControlPerLane +
    lane_id * MachineConfig.CapControlPerLane;
  // Convert from bytes to words. the pointers are ptr_t
  offset /= sizeof( word_t );
  auto startSig = BaseAddrs.ctrlAddr + offset + MachineConfig.StartExecOffset;
  *( startSig ) = 1;
  UPDOWN_INFOMSG( "Starting execution UD %u, Lane %u. Signal in  0x%lX",
                  ud_id,
                  lane_id,
                  reinterpret_cast< uint64_t >( startSig ) );
}

template< class AllocationPolicy >
uint32_t UDRuntime_t< AllocationPolicy >::get_globalUDNum( networkid_t& nid ) {
  return nid.get_NodeId() *
           ( this->MachineConfig.NumStacks * this->MachineConfig.NumUDs ) +
         nid.get_StackId() * ( this->MachineConfig.NumUDs ) + nid.get_UdId();
}

template< class AllocationPolicy >
uint64_t
  UDRuntime_t< AllocationPolicy >::get_lane_aligned_offset( networkid_t nwid,
                                                            uint32_t offset ) {
  auto    alignment      = sizeof( word_t );
  auto    aligned_offset = offset - offset % alignment;
  uint8_t ud_id          = nwid.get_UdId();
  uint8_t lane_id        = nwid.get_LaneId();
  uint8_t stack_id       = nwid.get_StackId();
  uint8_t node_id        = nwid.get_NodeId();
  UPDOWN_WARNING_IF( offset % alignment != 0, "Unaligned offset %u", offset );
  uint64_t returned_offset =
    ( node_id * MachineConfig.CapNumStacks * MachineConfig.CapNumUDs +
      stack_id * MachineConfig.CapNumUDs + ud_id ) *
      MachineConfig.CapNumLanes * MachineConfig.CapSPmemPerLane +
    MachineConfig.CapSPmemPerLane * lane_id +  // Lane offset
    aligned_offset;
  return returned_offset;
}

template< class AllocationPolicy >
uint64_t
  UDRuntime_t< AllocationPolicy >::get_lane_physical_memory( networkid_t nwid,
                                                             uint32_t offset ) {
  uint8_t ud_id          = nwid.get_UdId();
  uint8_t lane_id        = nwid.get_LaneId();
  uint8_t stack_id       = nwid.get_StackId();
  uint8_t node_id        = nwid.get_NodeId();
  auto    alignment      = sizeof( word_t );
  auto    aligned_offset = offset - offset % alignment;
  UPDOWN_WARNING_IF( offset % alignment != 0, "Unaligned offset %u", offset );
  uint64_t returned_offset =
    ( ( node_id * MachineConfig.NumStacks + stack_id ) * MachineConfig.NumUDs +
      ud_id ) *
      MachineConfig.NumLanes * MachineConfig.SPBankSize +
    lane_id * MachineConfig.SPBankSize +  // Lane offset
    aligned_offset + MachineConfig.SPMemBase;
  return returned_offset;
}

template< class AllocationPolicy >
uint64_t
  UDRuntime_t< AllocationPolicy >::get_ud_physical_memory( networkid_t nwid ) {
  uint8_t  ud_id    = nwid.get_UdId();
  uint8_t  stack_id = nwid.get_StackId();
  uint8_t  node_id  = nwid.get_NodeId();
  uint64_t returned_offset =
    ( ( node_id * MachineConfig.NumStacks + stack_id ) * MachineConfig.NumUDs +
      ud_id ) *
      MachineConfig.NumLanes * MachineConfig.SPBankSize +
    MachineConfig.SPMemBase;
  return returned_offset;
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::dumpMemory( const char* filename,
                                                  void*       vaddr,
                                                  uint64_t    size ) {
#ifdef ASST
  int fd = rev_openat( AT_FDCWD,
                       filename,
                       O_WRONLY | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  assert( fd != -1 );
  // Follow gem5 format. G=gem5, D=DRAM
  char byteHeader[2] = { 'G', 'D' };
  rev_write( fd, (const char*) byteHeader, 2 );
  // File offset, address, size
  uint64_t wordHeader[3] = {
    0x001aUL, reinterpret_cast< uint64_t >( vaddr ), size };
  rev_write( fd, (const char*) wordHeader, 3 * sizeof( uint64_t ) );
  // Now write the data
  rev_write( fd, (const char*) vaddr, size );
  rev_close( fd );
#elif defined( GEM5_MODE )
  m5_dump_mem( vaddr, size, filename );
#endif
}

template< class AllocationPolicy >
std::pair< void*, uint64_t > UDRuntime_t< AllocationPolicy >::loadMemory(
  const char* filename, void* vaddr, uint64_t size ) {
#ifdef ASST
  int fd = rev_openat( AT_FDCWD, filename, 0, O_RDONLY );
  assert( fd != -1 );
  char byteHeader[2];
  rev_read( fd, byteHeader, 2 );
  assert( byteHeader[0] = 'G' );  // gem5
  assert( byteHeader[1] = 'D' );  // dram
  uint64_t wordHeader[3];
  rev_read( fd, (char*) wordHeader, 3 * sizeof( uint64_t ) );
  uint64_t dump_start_file_offset = wordHeader[0];
  uint64_t dump_vaddr             = wordHeader[1];
  uint64_t dump_size              = wordHeader[2];
  assert( dump_start_file_offset = 0x1aUL );
  if( vaddr == nullptr ) {
    vaddr = reinterpret_cast< void* >( dump_vaddr );
    UPDOWN_INFOMSG( "DRAM Dump vaddr (from dump file): 0x%lx", dump_vaddr );
  } else {
    UPDOWN_INFOMSG( "DRAM Dump vaddr (user specified): 0x%lx",
                    reinterpret_cast< uint64_t >( vaddr ) );
  }
  if( vaddr == nullptr || size == 0 ) {
    size = dump_size;
    UPDOWN_INFOMSG( "DRAM Dump size (from dump file): %lu", size );
  } else {
    UPDOWN_INFOMSG( "DRAM Dump size (user specified): %lu", size );
  }

  if( this->mm_malloc_global_at_addr( vaddr, size ) == nullptr &&
      this->mm_malloc_at_addr( vaddr, size ) == nullptr ) {  // allocate memory
    UPDOWN_ERROR( "DRAM dump load failed! Cannot allocate memory at (%p, %lu)",
                  vaddr,
                  size );
  }

  // Read the entire file to the allocated dram address
  // TODO check stack limitations.
  assert( size % sizeof( uint64_t ) == 0 );  // make sure aligned uint64_t;
  rev_read( fd, (char*) vaddr, size );
  rev_close( fd );

  return std::make_pair( vaddr, size );
#elif defined( GEM5_MODE )
  FILE* mem_file = fopen( filename, "rb" );
  if( !mem_file ) {
    printf( "Could not open %s\n", filename );
    exit( 1 );
  }

  fseek( mem_file, 0, SEEK_SET );
  // Read 'F' to indicate dump by Fastsim
  char dump_type;
  fread( &dump_type, sizeof( char ), 1, mem_file );
  UPDOWN_ERROR_IF( dump_type != 'G',
                   "DRAM dump load failed! Not a Gem5 dump file!\n" );
  // Read 'D' to indicate DRAM dump
  fread( &dump_type, sizeof( char ), 1, mem_file );
  UPDOWN_ERROR_IF( dump_type != 'D',
                   "DRAM dump load failed! Not a DRAM dump file!\n" );
  // Read dump start file offset
  uint64_t dump_start_file_offset;
  fread( &dump_start_file_offset, sizeof( uint64_t ), 1, mem_file );
  UPDOWN_INFOMSG( "DRAM Dump start file offset: %lu", dump_start_file_offset );
  // Read dump vaddr
  uint64_t dump_vaddr;
  fread( &dump_vaddr, sizeof( uint64_t ), 1, mem_file );
  if( vaddr == nullptr ) {
    vaddr = reinterpret_cast< void* >( dump_vaddr );
    UPDOWN_INFOMSG( "DRAM Dump vaddr (from dump file): %p", vaddr );
  } else {
    UPDOWN_INFOMSG( "DRAM Dump vaddr (user specified): %p", vaddr );
  }
  // Read dump size
  uint64_t dump_size;
  fread( &dump_size, sizeof( uint64_t ), 1, mem_file );
  if( vaddr == nullptr || size == 0 ) {
    size = dump_size;
    UPDOWN_INFOMSG( "DRAM Dump size (from dump file): %lu", size );
  } else {
    UPDOWN_INFOMSG( "DRAM Dump size (user specified): %lu", size );
  }
  fclose( mem_file );

  if( this->mm_malloc_global_at_addr( vaddr, size ) == nullptr &&
      this->mm_malloc_at_addr( vaddr, size ) == nullptr ) {  // allocate memory
    UPDOWN_ERROR( "DRAM dump load failed! Cannot allocate memory at (%p, %lu)",
                  vaddr,
                  size );
  }

  m5_load_mem( vaddr, size, filename );
  // m5_load_mem(0, 0, filename);

  return std::make_pair( vaddr, size );
#else
  return std::make_pair( nullptr, 0 );
#endif
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::dumpLocalMemory( const char* filename,
                                                       networkid_t start_nwid,
                                                       uint64_t    num_lanes ) {
#ifdef ASST
  assert( false );  // TODO
#elif defined( GEM5_MODE )
  if( start_nwid.get_NetworkId_UdName() == 0 && num_lanes == 0 ) {
    num_lanes = this->MachineConfig.NumLanes * this->MachineConfig.NumUDs *
                this->MachineConfig.NumStacks * this->MachineConfig.NumNodes;
  }
  m5_dump_udlm(
    BaseAddrs.spaddr, start_nwid.get_NetworkId_UdName(), num_lanes, filename );
#endif
}

template< class AllocationPolicy >
std::pair< networkid_t, uint64_t >
  UDRuntime_t< AllocationPolicy >::loadLocalMemory( const char* filename,
                                                    networkid_t start_nwid,
                                                    uint64_t    num_lanes ) {
#ifdef ASST
  assert( false );  // TODO
  return std::make_pair( networkid_t( 0, false, 0 ), 0 );
#elif defined( GEM5_MODE )
  FILE* spd_file = fopen( filename, "rb" );
  if( !spd_file ) {
    printf( "Could not open %s\n", filename );
    exit( 1 );
  }

  fseek( spd_file, 0, SEEK_SET );
  // Read 'F' to indicate dump by Fastsim
  char dump_type;
  fread( &dump_type, sizeof( char ), 1, spd_file );
  UPDOWN_ERROR_IF( dump_type != 'G',
                   "DRAM dump load failed! Not a Gem5 dump file!\n" );
  // Read 'L' to indicate LM dump
  fread( &dump_type, sizeof( char ), 1, spd_file );
  UPDOWN_ERROR_IF( dump_type != 'L',
                   "DRAM dump load failed! Not a LM dump file!\n" );
  // Read dump start file offset
  uint64_t dump_start_file_offset;
  fread( &dump_start_file_offset, sizeof( uint64_t ), 1, spd_file );
  UPDOWN_INFOMSG( "LM Dump start file offset: %lu", dump_start_file_offset );
  // Read dump start nwid (ud_name only)
  uint64_t dump_start_nwid_raw;
  fread( &dump_start_nwid_raw, sizeof( uint64_t ), 1, spd_file );
  networkid_t dump_start_nwid( dump_start_nwid_raw, false, 0 );
  UPDOWN_INFOMSG( "LM Dump start nwid (from dump file): %lu",
                  dump_start_nwid_raw );
  // Read num lanes dumped
  uint64_t dump_num_lanes;
  fread( &dump_num_lanes, sizeof( uint64_t ), 1, spd_file );
  UPDOWN_INFOMSG( "LM Dump num lanes (from dump file): %lu", dump_num_lanes );
  // Read LM size per lane
  uint64_t lane_lm_size;
  fread( &lane_lm_size, sizeof( uint64_t ), 1, spd_file );
  UPDOWN_INFOMSG( "LM Dump size per lane (from dump file): %lu", lane_lm_size );

  // FIXME
  // Disable using the pseudo instruction interface due to crash
  // m5_load_udlm(BaseAddrs.spaddr, start_nwid.get_NetworkId_UdName(), num_lanes, filename);

  // Use t2ud_memcpy interface for now. this should be executed before switch_cpus (for fast simulation time)
  fseek( spd_file, dump_start_file_offset, SEEK_SET );
  uint8_t* data = nullptr;
  if( start_nwid.get_NetworkId_UdName() == 0 && num_lanes == 0 ) {
    // LM specified from the dump
    uint64_t total_lm_size = dump_num_lanes * lane_lm_size;
    data = (uint8_t*) malloc( total_lm_size * sizeof( uint8_t ) );
    fread( data, sizeof( uint8_t ), total_lm_size, spd_file );
    uint64_t lm_offset = 0;
    for( uint32_t i = dump_start_nwid.get_NetworkId_UdName();
         i < dump_start_nwid.get_NetworkId_UdName() + dump_num_lanes;
         i++ ) {
      t2ud_memcpy(
        &data[lm_offset], lane_lm_size, networkid_t( i, false, 0 ), 0 );
      lm_offset += lane_lm_size;
    }
  } else {
    // all LMs
    uint64_t total_lm_size = num_lanes * DEF_SPMEM_BANK_SIZE;
    data = (uint8_t*) malloc( total_lm_size * sizeof( uint8_t ) );
    fread( data, sizeof( uint8_t ), total_lm_size, spd_file );
    uint64_t lm_offset = 0;
    for( uint32_t i = start_nwid.get_NetworkId_UdName();
         i < start_nwid.get_NetworkId_UdName() + num_lanes;
         i++ ) {
      t2ud_memcpy(
        &data[lm_offset], DEF_SPMEM_BANK_SIZE, networkid_t( i, false, 0 ), 0 );
      lm_offset += DEF_SPMEM_BANK_SIZE;
    }
  }

  fclose( spd_file );

  if( start_nwid.get_NetworkId_UdName() == 0 && num_lanes == 0 ) {
    return std::make_pair( dump_start_nwid, dump_num_lanes );
  } else {
    return std::make_pair( start_nwid, num_lanes );
  }
#else
  return std::make_pair( networkid_t( 0, false, 0 ), 0 );
#endif
}

template< class AllocationPolicy >
void* UDRuntime_t< AllocationPolicy >::mm_malloc( uint64_t size ) {
  UPDOWN_INFOMSG( "Calling mm_malloc %lu", size );
  return MappedMemoryManager->get_region( size, false );
}

template< class AllocationPolicy >
void* UDRuntime_t< AllocationPolicy >::mm_malloc_at_addr( void*    addr,
                                                          uint64_t size ) {
#ifndef ASST
  UPDOWN_INFOMSG( "Calling mm_malloc_at_addr (%p, %lu)", addr, size );
#else
  UPDOWN_INFOMSG( "Calling mm_malloc_at_addr (0x%lx, %lu)",
                  reinterpret_cast< uint64_t >( addr ),
                  size );
#endif

  return MappedMemoryManager->get_region_at_addr( addr, size, false );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::mm_free( void* ptr ) {
  UPDOWN_INFOMSG( "Calling mm_free  0x%lX",
                  reinterpret_cast< uint64_t >( ptr ) );
  return MappedMemoryManager->remove_region( ptr, false );
}

template< class AllocationPolicy >
void* UDRuntime_t< AllocationPolicy >::mm_malloc_global( uint64_t size ) {
  UPDOWN_INFOMSG( "Calling mm_malloc_global %lu", size );
  return MappedMemoryManager->get_region( size, true );
}

template< class AllocationPolicy >
void* UDRuntime_t< AllocationPolicy >::mm_malloc_global_at_addr(
  void* addr, uint64_t size ) {
#ifndef ASST
  UPDOWN_INFOMSG( "Calling mm_malloc_global_at_addr (%p, %lu)", addr, size );
#else
  UPDOWN_INFOMSG( "Calling mm_malloc_global_at_addr (0x%lx, %lu)",
                  reinterpret_cast< uint64_t >( addr ),
                  size );
#endif
  return MappedMemoryManager->get_region_at_addr( addr, size, true );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::mm_free_global( void* ptr ) {
  UPDOWN_INFOMSG( "Calling mm_free_global  0x%lX",
                  reinterpret_cast< uint64_t >( ptr ) );
  return MappedMemoryManager->remove_region( ptr, true );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::mm2t_memcpy( uint64_t offset,
                                                   void*    dst,
                                                   uint64_t size ) {
  ptr_t src = BaseAddrs.mmaddr + offset / sizeof( word_t );
  UPDOWN_ASSERT(
    src + size / sizeof( word_t ) <
      BaseAddrs.mmaddr + MachineConfig.MapMemSize / sizeof( word_t ),
    "mm2t_memcpy: memory access to 0x%lX out of mapped memory bounds "
    "with offset %lu bytes and size %lu bytes. Mapped memory Base Address "
    "0x%lX mapped memory size %lu bytes",
    (unsigned long) ( BaseAddrs.mmaddr + offset ),
    (unsigned long) ( offset * sizeof( word_t ) ),
    (unsigned long) size,
    (unsigned long) BaseAddrs.mmaddr,
    (unsigned long) MachineConfig.MapMemSize );
  UPDOWN_INFOMSG(
    "Copying %lu bytes from mapped memory (0x%lX = %ld) to top (0x%lX = %ld)",
    size,
    reinterpret_cast< uint64_t >( src ),
    *src,
    reinterpret_cast< uint64_t >( dst ),
    *reinterpret_cast< word_t* >( dst ) );
  std::memcpy( dst, src, size );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::t2mm_memcpy( uint64_t offset,
                                                   void*    src,
                                                   uint64_t size ) {
  ptr_t dst = BaseAddrs.mmaddr + offset / sizeof( word_t );
  UPDOWN_ASSERT(
    dst + size / sizeof( word_t ) <
      BaseAddrs.mmaddr + MachineConfig.MapMemSize / sizeof( word_t ),
    "t2mm_memcpy: memory access to 0x%lX out of mapped memory bounds "
    "with offset %lu bytes and size %lu bytes. Mapped memory Base "
    "Address 0x%lX mapped memory size %lu bytes",
    (unsigned long) ( BaseAddrs.mmaddr + offset ),
    (unsigned long) ( offset * sizeof( word_t ) ),
    (unsigned long) size,
    (unsigned long) BaseAddrs.mmaddr,
    MachineConfig.MapMemSize );
  UPDOWN_INFOMSG(
    "Copying %lu bytes from top (0x%lX = %ld) to mapped memory (0x%lX = %ld)",
    size,
    reinterpret_cast< uint64_t >( src ),
    *reinterpret_cast< word_t* >( src ),
    reinterpret_cast< uint64_t >( dst ),
    *dst );
  std::memcpy( dst, src, size );
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::t2ud_memcpy( void*       data,
                                                   uint64_t    size,
                                                   networkid_t nwid,
                                                   uint32_t    offset ) {
  uint64_t apply_offset = get_lane_aligned_offset( nwid, offset );
  apply_offset /= sizeof( word_t );
  UPDOWN_ASSERT(
    BaseAddrs.spaddr + apply_offset + size / sizeof( word_t ) <
      BaseAddrs.spaddr + MachineConfig.SPSize() / sizeof( word_t ),
    "t2ud_memcpy: memory access to 0x%lX out of scratchpad memory bounds "
    "with offset %lu bytes and size %lu bytes. Scratchpad memory Base "
    "Address 0x%lX scratchpad memory size %lu bytes",
    (unsigned long) ( BaseAddrs.spaddr + apply_offset ),
    (unsigned long) ( apply_offset * sizeof( word_t ) ),
    (unsigned long) size,
    (unsigned long) BaseAddrs.spaddr,
    MachineConfig.SPSize() );
  std::memcpy( BaseAddrs.spaddr + apply_offset, data, size );
#ifndef ASST
  UPDOWN_INFOMSG(
    "Copying %lu bytes from Top to Node:%u, Stack:%u, UD %u, "
    "Lane %u, offset %u, Apply offset %lu. Signal in 0x%lX",
    size,
    nwid.get_NodeId(),
    nwid.get_StackId(),
    nwid.get_UdId(),
    nwid.get_LaneId(),
    offset,
    apply_offset,
    reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ) );
#else
  UPDOWN_INFOMSG( "Copying %lu bytes from Top to Node:%u, Stack:%u, UD %u, "
                  "Lane %u, ",
                  size,
                  nwid.get_NodeId(),
                  nwid.get_StackId(),
                  nwid.get_UdId(),
                  nwid.get_LaneId() );
  UPDOWN_INFOMSG(
    "offset %u, Apply offset %lu. Signal in 0x%lX\n",
    offset,
    apply_offset,
    reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ) );
#endif
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::ud2t_memcpy( void*       data,
                                                   uint64_t    size,
                                                   networkid_t nwid,
                                                   uint32_t    offset ) {
  uint64_t apply_offset = get_lane_aligned_offset( nwid, offset );
  apply_offset /= sizeof( word_t );
  UPDOWN_ASSERT(
    BaseAddrs.spaddr + apply_offset + size / sizeof( word_t ) <
      BaseAddrs.spaddr + MachineConfig.SPSize() / sizeof( word_t ),
    "ud2t_memcpy: memory access to 0x%lX out of scratchpad memory bounds "
    "with offset %lu bytes and size %lu bytes. Scratchpad memory Base "
    "Address 0x%lX scratchpad memory size %lu bytes",
    (unsigned long) ( BaseAddrs.spaddr + apply_offset ),
    (unsigned long) ( apply_offset * sizeof( word_t ) ),
    (unsigned long) size,
    (unsigned long) BaseAddrs.spaddr,
    MachineConfig.SPSize() );
  std::memcpy( data, BaseAddrs.spaddr + apply_offset, size );
  UPDOWN_INFOMSG(
    "Copying %lu bytes from UD %u, Lane %u to Top, offset %u, "
    "Apply offset %lu. Signal in 0x%lX",
    size,
    nwid.get_UdId(),
    nwid.get_LaneId(),
    offset,
    apply_offset,
    reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ) );
}

template< class AllocationPolicy >
bool UDRuntime_t< AllocationPolicy >::test_addr( networkid_t nwid,
                                                 uint32_t    offset,
                                                 word_t      expected ) {
  uint64_t apply_offset = get_lane_aligned_offset( nwid, offset );
  apply_offset /= sizeof( word_t );
  UPDOWN_ASSERT(
    BaseAddrs.spaddr + apply_offset <
      BaseAddrs.spaddr + MachineConfig.SPSize() / sizeof( word_t ),
    "test_addr: memory access to 0x%lX out of scratchpad memory bounds "
    "with offset %lu bytes and size 4 bytes. Scratchpad memory Base Address "
    "0x%lX scratchpad memory size %lu bytes",
    (unsigned long) ( BaseAddrs.spaddr + apply_offset ),
    (unsigned long) ( apply_offset * sizeof( word_t ) ),
    (unsigned long) BaseAddrs.spaddr,
    MachineConfig.SPSize() );
  UPDOWN_INFOMSG(
    "Testing UD %u, Lane %u to Top, offset %u."
    " Addr 0x%lX. Expected = %lu, read = %lu",
    nwid.get_UdId(),
    nwid.get_LaneId(),
    offset,
    reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ),
    expected,
    *( BaseAddrs.spaddr + apply_offset ) );
  return *( BaseAddrs.spaddr + apply_offset ) == expected;
}

template< class AllocationPolicy >
void UDRuntime_t< AllocationPolicy >::test_wait_addr( networkid_t nwid,
                                                      uint32_t    offset,
                                                      word_t      expected ) {
  uint64_t apply_offset = get_lane_aligned_offset( nwid, offset );
  apply_offset /= sizeof( word_t );
  UPDOWN_ASSERT(
    BaseAddrs.spaddr + apply_offset <
      BaseAddrs.spaddr + MachineConfig.SPSize() / sizeof( word_t ),
    "test_wait_addr: memory access to 0x%lX out of scratchpad memory bounds "
    "with offset %lu bytes and size 4 bytes. Scratchpad memory Base Address "
    "0x%lX scratchpad memory size %lu bytes",
    (unsigned long) ( BaseAddrs.spaddr + apply_offset ),
    (unsigned long) ( apply_offset * sizeof( word_t ) ),
    (unsigned long) BaseAddrs.spaddr,
    MachineConfig.SPSize() );
#ifndef ASST
  UPDOWN_INFOMSG(
    "Testing UD %u, Lane %u to Top, offset %u."
    " Addr 0x%lX. Expected = %lu, read = %lu. (%s)",
    nwid.get_UdId(),
    nwid.get_LaneId(),
    offset,
    reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ),
    expected,
    *( BaseAddrs.spaddr + apply_offset ),
    *( BaseAddrs.spaddr + apply_offset ) != expected ? "Waiting" :
                                                       "Returning" );
#else
  if( *( BaseAddrs.spaddr + apply_offset ) != expected ) {
    UPDOWN_INFOMSG(
      "Testing UD %u, Lane %u to Top, offset %u."
      " Addr 0x%lX. Expected = %lu, read = %lu. (Waiting)",
      nwid.get_UdId(),
      nwid.get_LaneId(),
      offset,
      reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ),
      expected,
      *( BaseAddrs.spaddr + apply_offset ) );
  } else {
    UPDOWN_INFOMSG(
      "Testing UD %u, Lane %u to Top, offset %u."
      " Addr 0x%lX. Expected = %lu, read = %lu. (Returning)",
      nwid.get_UdId(),
      nwid.get_LaneId(),
      offset,
      reinterpret_cast< uint64_t >( BaseAddrs.spaddr + apply_offset ),
      expected,
      *( BaseAddrs.spaddr + apply_offset ) );
  }
#endif
  while( *( BaseAddrs.spaddr + apply_offset ) != expected )
    ;
}

}  // namespace UpDown
#endif
