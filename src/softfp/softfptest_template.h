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
#if F_SIZE == 32
#define F_UINT uint32_t
#define F_ULONG uint64_t
#define MANT_SIZE 23
#define EXP_SIZE 8
#elif F_SIZE == 64
#define F_UHALF uint32_t
#define F_UINT uint64_t
#define F_ULONG uint128_t
#define MANT_SIZE 52
#define EXP_SIZE 11
#elif F_SIZE == 128
#define F_UHALF uint64_t
#define F_UINT uint128_t
#define MANT_SIZE 112
#define EXP_SIZE 15
#else
#error unsupported F_SIZE
#endif

#define EXP_MASK ((1 << EXP_SIZE) - 1)
#define MANT_MASK (((F_UINT)1 << MANT_SIZE) - 1)
#define SIGN_MASK ((F_UINT)1 << (F_SIZE - 1))
#define IMANT_SIZE (F_SIZE - 2) /* internal mantissa size */
#define RND_SIZE (IMANT_SIZE - MANT_SIZE)
#define QNAN_MASK ((F_UINT)1 << (MANT_SIZE - 1))

#define F_QNAN glue(F_QNAN, F_SIZE)
#define pack_sf glue(pack_sf, F_SIZE)
#define exec_ref_op glue(exec_ref_op, F_SIZE)
#define test_op glue(test_op, F_SIZE)
#define test_op_all glue(test_op_all, F_SIZE)
#define float_union glue(float_union, F_SIZE)
#define print_sf glue(print_sf, F_SIZE)
#define rrandom_u glue(rrandom_u, F_SIZE)
#define rrandom_sf glue(rrandom_sf, F_SIZE)
#define special_sf glue(special_sf, F_SIZE)
#define test_float glue(test_float, F_SIZE)
#define softfloat_float glue(float, F_SIZE)
#define softfloat_to_sf glue(softfloat_to_sf, F_SIZE)
#define sf_to_softfloat glue(sf_to_softfloat, F_SIZE)
#define test_cvt_sf32_sf glue(test_cvt_sf32_sf, F_SIZE)
#define test_cvt_sf64_sf glue(test_cvt_sf64_sf, F_SIZE)
#define test_cvt_int_sf glue(test_cvt_int_sf, F_SIZE)
#define test_fma glue(test_fma, F_SIZE)
#define test_cmp glue(test_cmp, F_SIZE)

static const F_UINT F_QNAN = (((F_UINT)EXP_MASK << MANT_SIZE) | ((F_UINT)1 << (MANT_SIZE - 1)));

static void print_sf(F_UINT a)
{
    uint32_t a_sign, a_exp;
    F_UINT a_mant;
    a_sign = a >> (F_SIZE - 1);
    a_exp = (a >> MANT_SIZE) & EXP_MASK;
    a_mant = a & MANT_MASK;
    printf("%u %0*x ",
           a_sign,
           (EXP_SIZE + 3) / 4, a_exp);
#if F_SIZE == 32
    printf("%0*x", (MANT_SIZE + 3) / 4, a_mant);
#elif F_SIZE == 64
    printf("%0*" PRIx64 , (MANT_SIZE + 3) / 4, a_mant);
#elif F_SIZE == 128
    printf("%0*" PRIx64 "%016" PRIx64, 
           ((MANT_SIZE - 64) + 3) / 4, (uint64_t)(a_mant >> 64), (uint64_t)a_mant);
#else
#error unsupported F_SIZE
#endif
} 

/* random integer with long sequences of '0' and '1' */
F_UINT rrandom_u(int len)
{
    int bit, pos, n, end;
    F_UINT a;
    
    bit = random() & 1;
    pos = 0;
    a = 0;
    for(;;) {
        n = (random() % len) + 1;
        end = pos + n;
        if (end > len)
            end = len;
        if (bit) {
            n = end - pos;
            if (n == F_SIZE)
                a = (F_UINT)(-1);
            else
                a |= (((F_UINT)1 << n) - 1) << pos;
        }
        if (end >= len)
            break;
        pos = end;
        bit ^= 1;
    }
    return a;
}

static inline F_UINT pack_sf(uint32_t a_sign, uint32_t a_exp, F_UINT a_mant)
{
    return ((F_UINT)a_sign << (F_SIZE - 1)) |
        ((F_UINT)a_exp << MANT_SIZE) | 
        (a_mant & MANT_MASK);
}

F_UINT rrandom_sf(void)
{
    uint32_t a_exp, a_sign;
    F_UINT a_mant;
    a_sign = random() & 1;

    /* generate exponent close to the min/max more often than random */
    switch(random() & 15) {
    case 0:
        a_exp = (random() % (2 * MANT_SIZE)) & EXP_MASK;
        break;
    case 1:
        a_exp = (EXP_MASK - (random() % (2 * MANT_SIZE))) & EXP_MASK;
        break;
    default:
        a_exp = random() & EXP_MASK;
        break;
    }
    a_mant = rrandom_u(MANT_SIZE);
    return pack_sf(a_sign, a_exp, a_mant);
}

#define SPECIAL_COUNT 12

F_UINT special_sf(int i)
{
    switch(i) {
    case 0: /* zero */
    case 1:
        return pack_sf(i & 1, 0, 0);
    case 2: /* infinity */
    case 3:
        return pack_sf(i & 1, EXP_MASK, 0);
    case 4: /* 1.0 */
    case 5:
        return pack_sf(i & 1, EXP_MASK / 2, 0);
    case 6: /* QNAN */
    case 7:
        return pack_sf(i & 1, EXP_MASK, QNAN_MASK);
    case 8: /* SNAN */
    case 9:
        return pack_sf(i & 1, EXP_MASK, 1);
    case 10: /* subnormal */
    case 11:
        return pack_sf(i & 1, 0, 1);
    default:
        abort();
    }
}

static inline F_UINT softfloat_to_sf(softfloat_float a)
{
#if F_SIZE <= 64
    return a;
#else
    return ((uint128_t)a.high << 64) | a.low;
#endif
}

static inline softfloat_float sf_to_softfloat(F_UINT a)
{
#if F_SIZE <= 64
    return a;
#else
    softfloat_float a1;
    a1.low = a;
    a1.high = a >> 64;
    return a1;
#endif
}

#if defined(USE_REF_X86) && F_SIZE <= 64

typedef union {
    F_UINT u;
#if F_SIZE == 32
    float f;
#elif F_SIZE == 64
    double f;
#endif
} float_union;

#if F_SIZE == 32
#define SS "s"
#elif F_SIZE == 64
#define SS "d"
#else
#error unsupported SSE size
#endif

no_inline F_UINT exec_ref_op(uint32_t *pfflags, FloatOpEnum op,
                             F_UINT a1, F_UINT b1,
                             RoundingModeEnum rm)
{
    uint32_t mxcsr;
    float_union r, a, b;
    a.u = a1;
    b.u = b1;

    /* set rounding mode and mask all exceptions */
    mxcsr = (rm_to_rc[rm] << 13) | (0x3f << 7);
    /* Note: using assembler is the safe way to be sure no
       reordering is done */
    r = a;
    switch(op) {
    case OP_FADD:
        asm volatile ("ldmxcsr %0\n" 
                      "adds" SS " %2, %1\n"
                      "stmxcsr %0\n" 
                      : "+m" (mxcsr), "+x" (r.f)
                      : "x" (b.f));
        break;
    case OP_FMUL:
        asm volatile ("ldmxcsr %0\n" 
                      "muls" SS " %2, %1\n"
                      "stmxcsr %0\n" 
                      : "+m" (mxcsr), "+x" (r.f)
                      : "x" (b.f));
        break;
    case OP_FDIV:
        asm volatile ("ldmxcsr %0\n" 
                      "divs" SS " %2, %1\n"
                      "stmxcsr %0\n" 
                      : "+m" (mxcsr), "+x" (r.f)
                      : "x" (b.f));
        break;
    case OP_FSQRT:
        asm volatile ("ldmxcsr %0\n" 
                      "sqrts" SS " %2, %1\n"
                      "stmxcsr %0\n" 
                      : "+m" (mxcsr), "=x" (r.f)
                      : "x" (a.f));
        break;
    default:
        abort();
    }
    if (isnan(r.f))
        r.u = F_QNAN;
    *pfflags = mxcsr_to_fflags(mxcsr);
    return r.u;
}

#undef SS

#else

F_UINT exec_ref_op(uint32_t *pfflags, FloatOpEnum op,
                   F_UINT a, F_UINT b, RoundingModeEnum rm)
{
    F_UINT r;
    float_status st;
    softfloat_float a1, b1, r1;

    softfloat_init_ctx(&st, rm);
    a1 = sf_to_softfloat(a);
    b1 = sf_to_softfloat(b);

    switch(op) {
    case OP_FADD:
        r1 = glue(glue(float, F_SIZE), _add)(a1, b1, &st);
        break;
    case OP_FMUL:
        r1 = glue(glue(float, F_SIZE), _mul)(a1, b1, &st);
        break;
    case OP_FDIV:
        r1 = glue(glue(float, F_SIZE), _div)(a1, b1, &st);
        break;
    case OP_FSQRT:
        r1 = glue(glue(float, F_SIZE), _sqrt)(a1, &st);
        break;
#if F_SIZE <= 64
    case OP_FMIN:
        r1 = glue(glue(float, F_SIZE), _minnum)(a1, b1, &st);
        break;
    case OP_FMAX:
        r1 = glue(glue(float, F_SIZE), _maxnum)(a1, b1, &st);
        break;
#endif
    default:
        abort();
    }
    r = softfloat_to_sf(r1);
    *pfflags = softfloat_to_fflags(&st);
    return r;
}
#endif

void test_op(FloatOpEnum op, F_UINT a, F_UINT b, RoundingModeEnum rm,
             int it)
{
    F_UINT ref, r;
    uint32_t fflags, ref_fflags;

    ref = exec_ref_op(&ref_fflags, op, a, b, rm);
    fflags = 0;
    switch(op) {
    case OP_FADD:
        r = glue(add_sf, F_SIZE)(a, b, rm, &fflags);
        break;
    case OP_FMUL:
        r = glue(mul_sf, F_SIZE)(a, b, rm, &fflags);
        break;
    case OP_FDIV:
        r = glue(div_sf, F_SIZE)(a, b, rm, &fflags);
        break;
    case OP_FSQRT:
        r = glue(sqrt_sf, F_SIZE)(a, rm, &fflags);
        break;
    case OP_FMIN:
        r = glue(min_sf, F_SIZE)(a, b, &fflags);
        break;
    case OP_FMAX:
        r = glue(max_sf, F_SIZE)(a, b, &fflags);
        break;
    default:
        abort();
    }
    if (r != ref || ref_fflags != fflags) {
        printf("ERROR op=%s size=%d rm=%s it=%d:\n", 
               op_to_str[op], F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        if (op != OP_FSQRT) {
            printf("b  = "); print_sf(b); printf("\n");
        }
        printf("ref= "); print_sf(ref);
        printf(" fflags=0x%02x\n", ref_fflags);
        printf("r  = "); print_sf(r);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
}

#if F_SIZE >= 64
void test_cvt_sf32_sf(F_UINT a, RoundingModeEnum rm, int it)
{
    uint32_t a32, a32_ref;
    float_status st;
    uint32_t fflags;
    F_UINT a_ref;

    /* to 32 bit */
    fflags = 0;
    a32 = glue(glue(cvt_sf, F_SIZE), _sf32)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    a32_ref = softfloat_to_sf32(glue(glue(float, F_SIZE), _to_float32)(sf_to_softfloat(a), &st));
    if (a32 != a32_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_sf32 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= "); print_sf32(a32_ref); printf("\n");
        printf("r  = "); print_sf32(a32); printf("\n");
        exit(1);
    }
    
    /* from 32 bit */
    fflags = 0;
    a = glue(cvt_sf32_sf, F_SIZE)(a32, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(float32_to_float, F_SIZE)(sf_to_softfloat32(a32), &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf32_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf32(a32); printf("\n");
        printf("ref= "); print_sf(a_ref); printf("\n");
        printf("r  = "); print_sf(a); printf("\n");
        exit(1);
    }
}
#endif

#if F_SIZE >= 128
void test_cvt_sf64_sf(F_UINT a, RoundingModeEnum rm, int it)
{
    uint64_t a64, a64_ref;
    float_status st;
    uint32_t fflags;
    F_UINT a_ref;

    /* to 64 bit */
    fflags = 0;
    a64 = glue(glue(cvt_sf, F_SIZE), _sf64)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    a64_ref = softfloat_to_sf64(glue(glue(float, F_SIZE), _to_float64)(sf_to_softfloat(a), &st));
    if (a64 != a64_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_sf64 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= "); print_sf64(a64_ref); printf("\n");
        printf("r  = "); print_sf64(a64); printf("\n");
        exit(1);
    }
    
    /* from 64 bit */
    fflags = 0;
    a = glue(cvt_sf64_sf, F_SIZE)(a64, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(float64_to_float, F_SIZE)(sf_to_softfloat64(a64), &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf64_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf64(a64); printf("\n");
        printf("ref= "); print_sf(a_ref); printf("\n");
        printf("r  = "); print_sf(a); printf("\n");
        exit(1);
    }
}
#endif

void test_cvt_int_sf(F_UINT a, RoundingModeEnum rm, int it)
{
    int32_t i32, i32_ref;
    int64_t i64, i64_ref;
    float_status st;
    uint32_t fflags;
    F_UINT a_ref;

    /* to int32 */
    fflags = 0;
    i32 = glue(glue(cvt_sf, F_SIZE), _i32)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    i32_ref = glue(glue(float, F_SIZE), _to_int32)(sf_to_softfloat(a), &st);
    if (i32 != i32_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_i32 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= %d", i32_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = %d", i32); 
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }

#if F_SIZE <= 64
    /* XXX: softfloat_to_uint32() not available */
    /* to uint32 */
    fflags = 0;
    i32 = glue(glue(cvt_sf, F_SIZE), _u32)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    i32_ref = glue(glue(float, F_SIZE), _to_uint32)(sf_to_softfloat(a), &st);
    if (i32 != i32_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_u32 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= %u", i32_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = %u", i32); 
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
#endif

    /* to int64 */
    fflags = 0;
    i64 = glue(glue(cvt_sf, F_SIZE), _i64)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    i64_ref = glue(glue(float, F_SIZE), _to_int64)(sf_to_softfloat(a), &st);
    if (i64 != i64_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_i64 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= %" PRId64 , i64_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = %" PRId64, i64); 
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }

#if F_SIZE <= 64
    /* XXX: softfloat_to_uint64() not available */
    /* to uint64 */
    fflags = 0;
    i64 = glue(glue(cvt_sf, F_SIZE), _u64)(a, rm, &fflags);

    softfloat_init_ctx(&st, rm);
    i64_ref = glue(glue(float, F_SIZE), _to_uint64)(sf_to_softfloat(a), &st);
    if (i64 != i64_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_sf_u64 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= %" PRIu64 , i64_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = %" PRIu64, i64); 
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
#endif

    /* from int32 */
    i32 = rrandom_u32(32);

    fflags = 0;
    a = glue(cvt_i32_sf, F_SIZE)(i32, rm, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(int32_to_float, F_SIZE)(i32, &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_i32_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = %d", i32); printf("\n");
        printf("ref= "); print_sf(a_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = "); print_sf(a);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }

#if F_SIZE <= 64
    /* from uint32 */
    fflags = 0;
    a = glue(cvt_u32_sf, F_SIZE)(i32, rm, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(uint32_to_float, F_SIZE)(i32, &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_u32_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = %ud", i32); printf("\n");
        printf("ref= "); print_sf(a_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = "); print_sf(a);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
#endif

    /* from int64 */
    i64 = rrandom_u64(64);

    fflags = 0;
    a = glue(cvt_i64_sf, F_SIZE)(i64, rm, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(int64_to_float, F_SIZE)(i64, &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_i64_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = %" PRId64, i64); printf("\n");
        printf("ref= "); print_sf(a_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = "); print_sf(a);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }

#if F_SIZE <= 64
    /* from uint64 */
    fflags = 0;
    a = glue(cvt_u64_sf, F_SIZE)(i64, rm, &fflags);
    
    softfloat_init_ctx(&st, rm);
    a_ref = softfloat_to_sf(glue(uint64_to_float, F_SIZE)(i64, &st));
    if (a != a_ref || softfloat_to_fflags(&st) != fflags) {
        printf("ERROR op=cvt_u64_sf size=%d rm=%s it=%d\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = %" PRIu64, i64); printf("\n");
        printf("ref= "); print_sf(a_ref);
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = "); print_sf(a);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
#endif

#ifdef HAVE_INT128
    {
    int128_t i128, i128_ref;
    /* to/from int128 */
    /* XXX: no reference */
    fflags = 0;
    i128_ref = rrandom_u128(128) & (((uint128_t)1 << (MANT_SIZE + 1)) - 1);
    if (random() & 1)
        i128_ref = -i128_ref;
    a = glue(cvt_i128_sf, F_SIZE)(i128_ref, rm, &fflags);
    fflags = 0;
    i128 = glue(glue(cvt_sf, F_SIZE), _i128)(a, rm, &fflags);

    if (i128 != i128_ref) {
        printf("ERROR op=cvt_sf_i128 size=%d rm=%s it=%d:\n",
               F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("ref= "); print_u128(i128_ref); printf("\n");
        printf(" fflags=0x%02x\n", softfloat_to_fflags(&st));
        printf("r  = "); print_u128(i128); printf("\n");
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
    }
#endif
}

void test_fma(F_UINT a, F_UINT b, F_UINT c, RoundingModeEnum rm, int it)
{
#if F_SIZE <= 64
    F_UINT r, ref;
    float_status st;
    uint32_t fflags, ref_fflags;

    fflags = 0;
    r = glue(fma_sf, F_SIZE)(a, b, c, rm, &fflags);
    
    softfloat_init_ctx(&st, rm);

    ref = softfloat_to_sf(glue(glue(float, F_SIZE), _muladd)(
                sf_to_softfloat(a),
                sf_to_softfloat(b),
                sf_to_softfloat(c),
                0, &st));
    ref_fflags = softfloat_to_fflags(&st);

    if (r != ref || ref_fflags != fflags) {
        printf("ERROR op=%s size=%d rm=%s it=%d:\n", 
               "fma", F_SIZE, rm_to_str[rm], it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("b  = "); print_sf(b); printf("\n");
        printf("c  = "); print_sf(c); printf("\n");
        printf("ref= "); print_sf(ref);
        printf(" fflags=0x%02x\n", ref_fflags);
        printf("r  = "); print_sf(r);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }

#endif
}

void test_cmp(F_UINT a, F_UINT b, int it)
{
    int r, ref;
    float_status st;
    uint32_t fflags, ref_fflags;
    const char *op_str;

    fflags = 0;
    r = glue(eq_quiet_sf, F_SIZE)(a, b, &fflags);
    
    softfloat_init_ctx(&st, RM_RNE);

    ref = glue(glue(float, F_SIZE), _eq_quiet)(sf_to_softfloat(a), 
                                               sf_to_softfloat(b), &st);
    ref_fflags = softfloat_to_fflags(&st);
    
    if (r != ref || ref_fflags != fflags) {
        op_str = "eq_quiet";
        goto error;
    }

    fflags = 0;
    r = glue(le_sf, F_SIZE)(a, b, &fflags);
    
    softfloat_init_ctx(&st, RM_RNE);

    ref = glue(glue(float, F_SIZE), _le)(sf_to_softfloat(a), 
                                         sf_to_softfloat(b), &st);
    ref_fflags = softfloat_to_fflags(&st);
    
    if (r != ref || ref_fflags != fflags) {
        op_str = "le";
        goto error;
    }

    fflags = 0;
    r = glue(lt_sf, F_SIZE)(a, b, &fflags);
    
    softfloat_init_ctx(&st, RM_RNE);

    ref = glue(glue(float, F_SIZE), _lt)(sf_to_softfloat(a), 
                                         sf_to_softfloat(b), &st);
    ref_fflags = softfloat_to_fflags(&st);
    
    if (r != ref || ref_fflags != fflags) {
        op_str = "lt";
    error:
        printf("ERROR op=%s size=%d it=%d:\n", 
               op_str, F_SIZE, it);
        printf("a  = "); print_sf(a); printf("\n");
        printf("b  = "); print_sf(b); printf("\n");
        printf("ref= %d", ref);
        printf(" fflags=0x%02x\n", ref_fflags);
        printf("r  = %d", r);
        printf(" fflags=0x%02x\n", fflags);
        exit(1);
    }
}

void test_op_all(F_UINT a, F_UINT b, RoundingModeEnum rm, int it)
{
    test_op(OP_FADD, a, b, rm, it);
    test_op(OP_FMUL, a, b, rm, it);
    test_op(OP_FDIV, a, b, rm, it);
    test_op(OP_FSQRT, a, b, rm, it);
#if F_SIZE <= 64
    test_op(OP_FMIN, a, b, rm, it);
#endif
#if F_SIZE <= 64
    test_op(OP_FMAX, a, b, rm, it);
#endif
    test_cmp(a, b, it);

#if F_SIZE >= 64
    test_cvt_sf32_sf(a, rm, it);
#endif
#if F_SIZE >= 128
    test_cvt_sf64_sf(a, rm, it);
#endif
    test_cvt_int_sf(a, rm, it);
}

void test_float(int step)
{
    F_UINT a, b, c;
    int rm_count, i, j, k;
    RoundingModeEnum rm;

#ifdef USE_REF_X86
    rm_count = 4;
#else
    rm_count = 5;
#endif
    if (step == 0) {
        /* test all simple cases */
        for(i = 0; i < SPECIAL_COUNT; i++) {
            a = special_sf(i);
            for(j = 0; j < SPECIAL_COUNT; j++) {
                b = special_sf(j);
                for(rm = 0; rm < rm_count; rm++) {
                    test_op_all(a, b, rm, i * SPECIAL_COUNT + j);
                    for(k = 0; k < SPECIAL_COUNT; k++) {
                        c = special_sf(k);
                        test_fma(a, b, c, rm, (i * SPECIAL_COUNT + j) * SPECIAL_COUNT + k);
                    }
                }
            }
        }
    } else {
        for(i = 0; i < 1000; i++) {
            /* Note: we cannot test RM_RMM */
            a = rrandom_sf();
            b = rrandom_sf();
            c = rrandom_sf();
            for(rm = 0; rm < rm_count; rm++) {
                test_op_all(a, b, rm, i);
                test_fma(a, b, c, rm, i);
            }
        }
    }
}

#undef pack_sf
#undef test_fma
#undef exec_ref_op
#undef test_op
#undef test_op_all
#undef float_union
#undef print_sf
#undef rrandom_u
#undef rrandom_sf
#undef special_sf
#undef test_float
#undef softfloat_to_sf
#undef sf_to_softfloat
#undef test_cvt_sf32_sf
#undef test_cvt_sf64_sf
#undef test_cvt_int_sf
#undef test_cmp

#undef F_SIZE
#undef F_UINT
#undef F_ULONG
#undef F_UHALF
#undef MANT_SIZE
#undef EXP_SIZE
#undef EXP_MASK
#undef MANT_MASK
#undef SIGN_MASK
#undef IMANT_SIZE
#undef RND_SIZE
#undef QNAN_MASK
#undef F_QNAN
