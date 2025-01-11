#ifndef __REV_REVALLOC__
#define __REV_REVALLOC__

#include "syscalls.h"
#include "unistd.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>

template<typename T>
struct StandardAllocPolicy {
  //    typedefs
  typedef T                 value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef std::size_t       size_type;
  typedef std::ptrdiff_t    difference_type;

  //    convert an StandardAllocPolicy<T> to StandardAllocPolicy<U>
  template<typename U>
  struct rebind {
    typedef StandardAllocPolicy<U> other;
  };

  explicit StandardAllocPolicy()                    = default;
  ~StandardAllocPolicy()                            = default;
  StandardAllocPolicy( StandardAllocPolicy const& ) = default;

  template<typename U>
  explicit StandardAllocPolicy( StandardAllocPolicy<U> const& ) {}

  //    memory allocation
  pointer allocate( size_type cnt, typename std::allocator<void>::const_pointer = 0 ) {
    return reinterpret_cast<pointer>( rev_mmap(
      0,  // Let rev choose the address
      cnt * sizeof( T ),
      PROT_READ | PROT_WRITE | PROT_EXEC,  // RWX permissions
      MAP_PRIVATE | MAP_ANONYMOUS,         // Not shared, anonymous
      -1,                                  // No file descriptor because it's an anonymous mapping
      0                                    // No offset, irrelevant for anonymous mappings
    ) );
  }

  void deallocate( pointer p, size_type n ) {
    std::size_t addr = reinterpret_cast<std::size_t>( p );
    rev_munmap( addr, n );
  }

  //    size
  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  //    construction/destruction
  template<class U, class... Args>
  void construct( U* p, Args&&... args ) {
    new( p ) U( std::forward<Args>( args )... );
  };

  void destroy( pointer p ) { p->~T(); }
};  //    end of class StandardAllocPolicy

// determines if memory from another
// allocator can be deallocated from this one
template<typename T, typename T2>
bool operator==( StandardAllocPolicy<T> const&, StandardAllocPolicy<T2> const& ) {
  return true;
}

template<typename T, typename OtherAllocator>
bool operator==( StandardAllocPolicy<T> const&, OtherAllocator const& ) {
  return false;
}

template<typename T, typename Policy = StandardAllocPolicy<T>>
class Allocator : public Policy {
  typedef Policy AllocationPolicy;

public:
  typedef typename AllocationPolicy::size_type       size_type;
  typedef typename AllocationPolicy::difference_type difference_type;
  typedef typename AllocationPolicy::pointer         pointer;
  typedef typename AllocationPolicy::const_pointer   const_pointer;
  typedef typename AllocationPolicy::reference       reference;
  typedef typename AllocationPolicy::const_reference const_reference;
  typedef typename AllocationPolicy::value_type      value_type;

  template<typename U>
  struct rebind {
    typedef Allocator<U, typename AllocationPolicy::rebind<U>::other> other;
  };

  explicit Allocator()          = default;
  ~Allocator()                  = default;
  Allocator( Allocator const& ) = default;

  template<typename... U>
  Allocator( Allocator<U...> const& rhs ) : Policy( rhs ) {}
};  //    end of class Allocator

// determines if memory from another
// allocator can be deallocated from this one
template<typename T, typename P>
bool operator==( Allocator<T, P> const& lhs, Allocator<T, P> const& rhs ) {
  return operator==( static_cast<P&>( lhs ), static_cast<P&>( rhs ) );
}

template<typename T, typename P, typename T2, typename P2>
bool operator==( Allocator<T, P> const& lhs, Allocator<T2, P2> const& rhs ) {
  return operator==( static_cast<P&>( lhs ), static_cast<P2&>( rhs ) );
}

template<typename T, typename P, typename OtherAllocator>
bool operator==( Allocator<T, P> const& lhs, OtherAllocator const& rhs ) {
  return operator==( static_cast<P&>( lhs ), rhs );
}

template<typename T, typename P>
bool operator!=( Allocator<T, P> const& lhs, Allocator<T, P> const& rhs ) {
  return !operator==( lhs, rhs );
}

template<typename T, typename P, typename T2, typename P2>
bool operator!=( Allocator<T, P> const& lhs, Allocator<T2, P2> const& rhs ) {
  return !operator==( lhs, rhs );
}

template<typename T, typename P, typename OtherAllocator>
bool operator!=( Allocator<T, P> const& lhs, OtherAllocator const& rhs ) {
  return !operator==( lhs, rhs );
}

#endif
