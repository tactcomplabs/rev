//

#include <stdlib.h>
#include <math.h>
#include "minfft.h"


#define P  16
/*minfft_aux*
minfft_mkaux_dft_1d_local (int N) {
        minfft_aux *a;
        int n;
        minfft_real *e;
        a=malloc(sizeof(minfft_aux));
        a->N=N;
        if (N>=16) {
                a->t=malloc(N*sizeof(minfft_cmpl));
                if (a->t==NULL)
                        goto err;
                a->e=malloc(N*sizeof(minfft_cmpl));
                if (a->e==NULL)
                        goto err;
                e=(minfft_real*)a->e;
                while (N>=16) {
                        for (n=0; n<N/4; ++n) {
                                *e++=ncos(-n, N);
                                *e++=nsin(-n, N);
                                *e++=ncos(-3*n, N);
                                *e++=nsin(-3*n, N);
                        }
                        N/=2;
                }
        } else {
                a->t=NULL;
                a->e=NULL;
        }
        a->sub1=a->sub2=NULL;
        return a;
err:    // memory allocation error
        minfft_free_aux(a);
        return NULL;
}*/

static const minfft_real pi_l=3.141592653589793238462643383279502884L;
static minfft_real nsin_l (int, int);
// cos(2*pi*n/N)
static minfft_real
ncos_l (int n, int N) {
        // reduce n to 0..N/8
        if (n<0)
                return ncos_l(-n, N);
        else if (n>=N/2)
                return -ncos_l(n-N/2, N);
        else if (n>=N/4)
                return -nsin_l(n-N/4, N);
        else if (n>N/8)
                return nsin_l(N/4-n, N);
        else
#if MINFFT_SINGLE
                return cosf(2*pi_l*n/N);
#elif MINFFT_EXTENDED
                return cosl(2*pi_l*n/N);
#else
                return (2*pi_l*n/N);
#endif
}

// sin(2*pi*n/N)
static minfft_real
nsin_l (int n, int N) {
        // reduce n to 0..N/8
        if (n<0)
                return -nsin_l(-n, N);
        else if (n>=N/2)
                return -nsin_l(n-N/2, N);
        else if (n>=N/4)
                return ncos_l(n-N/4, N);
        else if (n>N/8)
                return ncos_l(N/4-n, N);
        else
#if MINFFT_SINGLE
                return sinf(2*pi_l*n/N);
#elif MINFFT_EXTENDED
                return sinl(2*pi_l*n/N);
#else
                return (2*pi_l*n/N);
#endif
}

int main()
{
        minfft_cmpl x[P], y[P]; // input and output buffers
        minfft_cmpl t[P], e2[P]; // input and output buffers
        //minfft_aux *a; // aux data
        // prepare aux data
        //a=minfft_mkaux_dft_1d(P);
  minfft_aux a;
        minfft_real *e;
  a.N = P;
  int ddd0 = rand();
  int ddd1 = rand();
  int ddd2 = rand();
  int ddd3 = rand();
  int N = P;
 // if (N>=16) {
//      a.t=t;
//              a.e=e2;
//              e=(minfft_real*)a.e;
//              while (N>=16) {
//                      //for (int n=0; n<N/4; ++n) {
//                              *e++=ncos_l(-n, N);
//                              *e++=nsin_l(-n, N);
//                              *e++=ncos_l(-3*n, N);
//                              *e++=nsin_l(-3*n, N) + ddd0;
//                      }
//                      N/2;
//              }
//      }
  if(N >= 16){
    for(int n=0; n< N; ++n){
      *e++=ddd1*pi_l+(n*ddd2);
    }
  }else{
    a.t = NULL;
    a.e = NULL;
  }
  a.sub1 = a.sub2 = NULL;

        // do transforms
  // volatile int* rev = 0xDEADBEEF;
  // *rev = 0x0AAA0001;
          minfft_dft(x, y, &a);
        //minfft_invdft(y, x, &a);
        // free aux data
        //minfft_free_aux(a);

    // exit normally
    return 0;
}
