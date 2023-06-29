//
// _RevRF_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVRF_H_
#define _SST_REVCPU_REVRF_H_

#include <iostream>
#include <type_traits>

namespace SST{
  namespace RevCPU {

    template<typename DType>
    struct is_valid {
      using result_type = typename std::conditional<
        std::is_pointer<DType>::value,
        typename std::is_integral<std::decay<DType>>::type,
        typename std::is_integral<DType>::type
      >::type;
  };

  template <typename DType, int T_MaxElem>
  class RevRF {

   using is_valid_dtype = typename is_valid<DType>::type;
   static_assert(is_valid_dtype::value, "invalide DType");

      RevRF() {};
      ~RevRF(){};

      DType operator[] (unsigned index) const {
         if(index > 0) {
          return rf[index];
         }else {
          return 0;
         }
      };

      DType& operator[] (unsigned index){
        if(index > 0) {
          return rf[index];
        }else {
          rf[0] = 0;
          return rf[index];
        }
      }

      private:
      DType rf[T_MaxElem];
    };
  }
}
 #endif 