/* stdint.h -- shim for Symbian GCCE 3.4.3 (no stdint.h in SDK).
 * Provides standard integer types using native C types.
 * Must NOT use Symbian TInt32 etc. to avoid conflicts with e32def.h. */
#ifndef _STDINT_H
#define _STDINT_H

/* Direct native type mappings. On ARM 32-bit GCCE:
 *   char = 1 byte, short = 2 bytes, int = 4 bytes, long long = 8 bytes.
 * Do NOT use Symbian TInt32/TUint32 — they're typedef'd as 'long int' in
 * e32def.h, which conflicts with our 'int' typedefs when both are visible. */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

/* intptr_t / uintptr_t */
typedef signed int          intptr_t;
typedef unsigned int        uintptr_t;

/* intmax_t / uintmax_t */
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
#define INT8_MIN     (-128)
#define INT8_MAX     127
#define UINT8_MAX    255U
#define INT16_MIN    (-32768)
#define INT16_MAX    32767
#define UINT16_MAX   65535U
#define INT32_MIN    (-2147483647 - 1)
#define INT32_MAX    2147483647
#define UINT32_MAX   4294967295U
#define INT64_MIN    (-9223372036854775807LL - 1)
#define INT64_MAX    9223372036854775807LL
#define UINT64_MAX   18446744073709551615ULL

/* Format macros */
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"

#endif /* _STDINT_H */
