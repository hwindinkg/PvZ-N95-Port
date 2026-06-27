/* stdint.h -- shim for Symbian GCCE 3.4.3 (no stdint.h in SDK).
 * Symbian's e32def.h macro-substitutes int32_t -> TInt32, so we must
 * #undef those macros before defining our own types. */
#ifndef _STDINT_H
#define _STDINT_H

/* Symbian e32def.h may #define int8_t TInt8, int32_t TInt32, etc.
 * Undef all of them so our typedefs don't get mangled. */
#undef int8_t
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef int64_t
#undef uint64_t
#undef intptr_t
#undef uintptr_t
#undef intmax_t
#undef uintmax_t

/* Direct native type mappings. */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

/* intptr_t may already be defined by Common.h or e32def.h as 'long int'.
 * Don't redefine if already declared. */
#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
typedef signed int          intptr_t;
#endif
#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
typedef unsigned int        uintptr_t;
#endif

typedef int64_t             intmax_t;
typedef uint64_t            uintmax_t;

/* Fast/least types */
typedef int8_t              int_least8_t;
typedef uint8_t             uint_least8_t;
typedef int16_t             int_least16_t;
typedef uint16_t            uint_least16_t;
typedef int32_t             int_least32_t;
typedef uint32_t            uint_least32_t;
typedef int64_t             int_least64_t;
typedef uint64_t            uint_least64_t;

typedef int8_t              int_fast8_t;
typedef uint8_t             uint_fast8_t;
typedef int16_t             int_fast16_t;
typedef uint16_t            uint_fast16_t;
typedef int32_t             int_fast32_t;
typedef uint32_t            uint_fast32_t;
typedef int64_t             int_fast64_t;
typedef uint64_t            uint_fast64_t;

/* Limit macros */
#ifndef INT8_MIN
#define INT8_MIN     (-128)
#endif
#ifndef INT8_MAX
#define INT8_MAX     127
#endif
#ifndef UINT8_MAX
#define UINT8_MAX    255U
#endif
#ifndef INT16_MIN
#define INT16_MIN    (-32768)
#endif
#ifndef INT16_MAX
#define INT16_MAX    32767
#endif
#ifndef UINT16_MAX
#define UINT16_MAX   65535U
#endif
#ifndef INT32_MIN
#define INT32_MIN    (-2147483647 - 1)
#endif
#ifndef INT32_MAX
#define INT32_MAX    2147483647
#endif
#ifndef UINT32_MAX
#define UINT32_MAX   4294967295U
#endif
#ifndef INT64_MIN
#define INT64_MIN    (-9223372036854775807LL - 1)
#endif
#ifndef INT64_MAX
#define INT64_MAX    9223372036854775807LL
#endif
#ifndef UINT64_MAX
#define UINT64_MAX   18446744073709551615ULL
#endif

#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"

#endif /* _STDINT_H */
