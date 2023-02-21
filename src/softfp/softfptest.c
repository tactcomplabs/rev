/*
 * SoftFP Test
 * 
 * Copyright (c) 2016 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <xmmintrin.h>
#include <string.h>

#include "cutils.h"
#include "softfloat.h"
#include "softfp.h"

#define USE_FPUTEST
//#define USE_REF_X86

typedef enum {
    OP_FADD,
    OP_FMUL,
    OP_FDIV,
    OP_FSQRT,
    OP_FMIN,
    OP_FMAX,
} FloatOpEnum;

static const char *op_to_str[] = {
    "fadd", "fmul", "fdiv", "fsqrt", "fmin", "fmax",
};

static const char *rm_to_str[5] = { "RNE", "RTZ", "RDN", "RUP", "RMM" };

#ifdef USE_REF_X86
static const int rm_to_rc[5] = { 0, 3, 1, 2, 0 };

static uint32_t mxcsr_to_fflags(uint32_t v)
{
    uint32_t ret;
    ret = 0;
    if (v != 0) {
        if (v & (1 << 0))
            ret |= FFLAG_INVALID_OP;
        if (v & (1 << 2))
            ret |= FFLAG_DIVIDE_ZERO;
        if (v & (1 << 3))
            ret |= FFLAG_OVERFLOW;
        if (v & (1 << 4))
            ret |= FFLAG_UNDERFLOW;
        if (v & (1 << 5))
            ret |= FFLAG_INEXACT;
    }
    return ret;
}
#endif

static void softfloat_init_ctx(float_status *st, RoundingModeEnum rm)
{
    static const int rm_to_frc[5] = { float_round_nearest_even,
                                      float_round_to_zero,
                                      float_round_down,
                                      float_round_up,
                                      float_round_ties_away };
    memset(st, 0, sizeof(*st));
    st->float_rounding_mode = rm_to_frc[rm];
    st->default_nan_mode = 1;
}

static uint32_t softfloat_to_fflags(float_status *st)
{
    uint32_t fflags;
    fflags = 0;
    if (st->float_exception_flags & float_flag_invalid)
        fflags |= FFLAG_INVALID_OP;
    if (st->float_exception_flags & float_flag_divbyzero)
        fflags |= FFLAG_DIVIDE_ZERO;
    if (st->float_exception_flags & float_flag_overflow)
        fflags |= FFLAG_OVERFLOW;
    if (st->float_exception_flags & float_flag_underflow)
        fflags |= FFLAG_UNDERFLOW;
    if (st->float_exception_flags & float_flag_inexact)
        fflags |= FFLAG_INEXACT;
    return fflags;
}


uint32_t rrandom_u32(int len);
uint64_t rrandom_u64(int len);

#ifdef HAVE_INT128
uint128_t rrandom_u128(int len);

static void print_u128(uint128_t a)
{
    printf("%016" PRIx64 "%016" PRIx64, 
           (uint64_t)(a >> 64), (uint64_t)a);
}
#endif


#define F_SIZE 32
#include "softfptest_template.h"

#define F_SIZE 64
#include "softfptest_template.h"

#ifdef HAVE_INT128
#define F_SIZE 128
#include "softfptest_template.h"
#endif

#if 0
void sqrtrem32_test(void)
{
    uint64_t a, a0, a1;
    uint32_t r;
    int inexact, inexact_ref;
    for(;;) {
        a = (uint64_t)random_uint(32) << 32;
        inexact = sqrtrem_u32(&r, a >> 32, a);
        a0 = (uint64_t)r * r;
        a1 = (uint64_t)(r + 1) * (r + 1);
        inexact_ref = (a0 != a);
        if (!(a0 <= a && a < a1) || inexact != inexact_ref) {
            printf("ERROR: a=%" PRId64 " r=%d\n", a, r);
            exit(1);
        }
    }
}
#endif

int main(int argc, char **argv)
{
    int seed;

    seed = 1;
    if (argc >= 2)
        seed = atoi(argv[1]);
    srandom(seed);

    printf("Starting softfptest (stop with Ctrl-C)\n");
    test_float32(0);
    test_float64(0);
#ifdef HAVE_INT128
    test_float128(0);
#endif
    for(;;) {
        test_float32(1);
        test_float64(1);
#ifdef HAVE_INT128
        test_float128(1);
#endif
    }
    return 0;
}
