#include <stdlib.h>
#include <limits>
#include <memory>
#include "syscalls.h"
#include "unistd.h"

#ifndef __REV_REVALLOC__
#define __REV_REVALLOC__

/*void* operator new(std::size_t t){
     void* p = reinterpret_cast<void*>(rev_mmap(0,                
              t,
              PROT_READ | PROT_WRITE | PROT_EXEC, 
              MAP_PRIVATE | MAP_ANONYMOUS, 
              -1,                   
              0));          
    return p;
}*/

void* mynew(std::size_t t){
     void* p = reinterpret_cast<void*>(rev_mmap(0,                
              t,
              PROT_READ | PROT_WRITE | PROT_EXEC, 
              MAP_PRIVATE | MAP_ANONYMOUS, 
              -1,                   
              0));          
    return p;
}

template<typename T>
class StandardAllocPolicy {
public : 
    //    typedefs
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public : 
    //    convert an StandardAllocPolicy<T> to StandardAllocPolicy<U>
    template<typename U>
    struct rebind {
        typedef StandardAllocPolicy<U> other;
    };

public : 
    inline explicit StandardAllocPolicy() {}
    inline ~StandardAllocPolicy() {}
    inline explicit StandardAllocPolicy(StandardAllocPolicy const&) {}
    template <typename U>
    inline explicit StandardAllocPolicy(StandardAllocPolicy<U> const&) {}
    
    //    memory allocation
    inline pointer allocate(size_type cnt, 
      typename std::allocator<void>::const_pointer = 0) { 
        return reinterpret_cast<pointer>(
              rev_mmap(0,                 // Let rev choose the address
              cnt * sizeof(T),
              PROT_READ | PROT_WRITE | PROT_EXEC, // RWX permissions
              MAP_PRIVATE | MAP_ANONYMOUS, // Not shared, anonymous
              -1,                   // No file descriptor because it's an anonymous mapping
              0));                   // No offset, irrelevant for anonymous mappings

    }
    inline void deallocate(pointer p, size_type n) 
                            {   std::size_t addr = reinterpret_cast<std::size_t>(p);
                                rev_munmap(addr, n); }
    //inline void deallocate(pointer p, size_type n) 
    //                        { rev_munmap(*p, n); }

    //    size
    inline size_type max_size() const { 
        return std::numeric_limits<size_type>::max(); 
    }

     //    construction/destruction
    //inline void construct(pointer p, const T& t) { pointer z = new(sizeof(T); new(z) T(t); p = z; }
    template<class U, class... Args>
    inline void construct(U* p, Args&&... args){ new(p) U(std::forward<Args>(args)...); };
    //template<class U, class... Args>
    //inline void construct(U* p, Args&&... args){ pointer z = reinterpret_cast<U*>(mynew(sizeof(U))); new(z) U(std::forward<Args>(args)...); p = z;}
    inline void destroy(pointer p) { p->~T(); }
};    //    end of class StandardAllocPolicy

// determines if memory from another
// allocator can be deallocated from this one
template<typename T, typename T2>
inline bool operator==(StandardAllocPolicy<T> const&, 
                        StandardAllocPolicy<T2> const&) { 
    return true;
}
template<typename T, typename OtherAllocator>
inline bool operator==(StandardAllocPolicy<T> const&, 
                                     OtherAllocator const&) { 
    return false; 
}

template<typename T, typename Policy = 
  StandardAllocPolicy<T> >
class Allocator : public Policy {
private : 
    typedef Policy AllocationPolicy;

public : 
    typedef typename AllocationPolicy::size_type size_type;
    typedef typename AllocationPolicy::difference_type difference_type;
    typedef typename AllocationPolicy::pointer pointer;
    typedef typename AllocationPolicy::const_pointer const_pointer;
    typedef typename AllocationPolicy::reference reference;
    typedef typename AllocationPolicy::const_reference const_reference;
    typedef typename AllocationPolicy::value_type value_type;

public : 
    template<typename U>
    struct rebind {
        typedef Allocator<U, typename AllocationPolicy::rebind<U>::other> other;
    };

public : 
    inline explicit Allocator() {}
    inline ~Allocator() {}
    inline Allocator(Allocator const& rhs): Policy(rhs) {}
    template <typename U>
    inline Allocator(Allocator<U> const&) {}
    template <typename U, typename P >
    inline Allocator(Allocator<U, P> const& rhs): Policy(rhs) {}
};    //    end of class Allocator

// determines if memory from another
// allocator can be deallocated from this one
template<typename T, typename P>
inline bool operator==(Allocator<T, P> const& lhs, 
                        Allocator<T, P> const& rhs) { 
    return operator==(static_cast<P&>(lhs), 
                       static_cast<P&>(rhs)); 
}

template<typename T, typename P,  
        typename T2, typename P2>
inline bool operator==(Allocator<T, P> const& lhs, 
                        Allocator<T2, P2> const& rhs) { 
      return operator==(static_cast<P&>(lhs), 
                       static_cast<P2&>(rhs)); 
}
template<typename T, typename P, typename OtherAllocator>
inline bool operator==(Allocator<T, P> const& lhs, OtherAllocator const& rhs) { 
    return operator==(static_cast<P&>(lhs), rhs); 
}
template<typename T, typename P>
inline bool operator!=(Allocator<T, P> const& lhs, 
                         Allocator<T, P> const& rhs) { 
    return !operator==(lhs, rhs); 
}
template<typename T, typename P, 
           typename T2, typename P2>
inline bool operator!=(Allocator<T, P> const& lhs, 
                   Allocator<T2, P2> const& rhs) { 
    return !operator==(lhs, rhs); 
}
template<typename T, typename P, typename OtherAllocator>
inline bool operator!=(Allocator<T, P> const& lhs, 
                       OtherAllocator const& rhs) { 
    return !operator==(lhs, rhs); 
}

#endif