@ newop.s - ARM assembly stubs
.text
.align 2

.global _Znwj; .type _Znwj, %function; _Znwj: b _ZN4User5AllocEi
.global _Znaj; .type _Znaj, %function; _Znaj: b _ZN4User5AllocEi
.global _ZdlPv; .type _ZdlPv, %function; _ZdlPv: b _ZN4User4FreeEPv
.global _ZdaPv; .type _ZdaPv, %function; _ZdaPv: b _ZN4User4FreeEPv

@ 32-bit unsigned division
.global __aeabi_uidivmod
.type __aeabi_uidivmod, %function
__aeabi_uidivmod:
    cmp r1, #0; moveq r0, #0; bxeq lr
    mov r2, r0; mov r0, #0
1:  cmp r2, r1; blo 2f; sub r2, r2, r1; add r0, r0, #1; b 1b
2:  mov r1, r2; bx lr

@ 32-bit signed division
.global __aeabi_idivmod
.type __aeabi_idivmod, %function
__aeabi_idivmod:
    cmp r1, #0; moveq r0, #0; bxeq lr
    push {r4, lr}; mov r4, r0; eor r4, r4, r1
    cmp r0, #0; rsblt r0, r0, #0; cmp r1, #0; rsblt r1, r1, #0
    bl __aeabi_uidivmod
    tst r4, #0x80000000; rsbne r0, r0, #0
    pop {r4, pc}

@ 64-bit unsigned division (n=r1:r0, d=r3:r2)
@ Uses repeated subtraction (slow but correct for all values)
.global __aeabi_uldivmod
.type __aeabi_uldivmod, %function
__aeabi_uldivmod:
    cmp r2, #0; cmpne r3, #0
    moveq r0, #0; moveq r1, #0; bxeq lr
    push {r4, r5, r6, r7, r8, lr}
    mov r4, r0; mov r5, r1; mov r6, r2; mov r7, r3
    mov r8, #0      @ quotient lo
    cmp r7, #0
    bne UDIV64_BIG
    @ 64/32 division loop
    mov r0, r5; mov r1, r6
    bl __aeabi_uidivmod
    mov r5, r1      @ remainder from hi word
    @ Divide (r5<<32 | r4) by r6 using subtraction
    mov r0, r4
    mov r1, r5
    mov r2, r6
UDIV64_LP:
    cmp r1, #0
    bne UDIV64_SUB
    cmp r0, r2
    blo UDIV64_DN
UDIV64_SUB:
    cmp r0, r2
    blo UDIV64_SK
    sub r0, r0, r2
    add r8, r8, #1
UDIV64_SK:
    cmp r1, #0
    subne r1, r1, #1
    addne r8, r8, #1
    b UDIV64_LP
UDIV64_DN:
    mov r0, r8; mov r1, #0; mov r2, r0; mov r3, #0
    pop {r4, r5, r6, r7, r8, pc}
UDIV64_BIG:
    mov r0, #0; mov r1, #0; mov r2, #0; mov r3, #0
    pop {r4, r5, r6, r7, r8, pc}

@ 64-bit shift right
.global __aeabi_llsr
.type __aeabi_llsr, %function
__aeabi_llsr:
    and r2, r2, #63; cmp r2, #32
    movlo r3, r2; rsblo r3, r3, #32
    movlo r0, r1, lsr r2; orrlo r0, r0, r0, lsl r3
    movhs r2, r2, lsr #5; movhs r0, r1, lsr r2
    movhs r1, #0
    bx lr

@ 64-bit shift left
.global __aeabi_llsl
.type __aeabi_llsl, %function
__aeabi_llsl:
    and r2, r2, #63; cmp r2, #32
    movlo r3, r2; rsblo r3, r3, #32
    movlo r1, r0, lsl r2; orrlo r1, r1, r1, lsr r3
    movhs r2, r2, lsr #5; movhs r1, r0, lsl r2
    movhs r0, #0
    bx lr


@ NOTE: C++ EH ABI routines (__cxa_begin_catch/__cxa_end_catch/__cxa_end_cleanup,
@ __aeabi_unwind_cpp_pr0/1, _Unwind_VRS_Get/Set/Pop, __cxa_begin_cleanup,
@ __cxa_call_unexpected, __cxa_type_match) are intentionally NOT stubbed here.
@ EKA2 implements User::Leave as a real C++ throw; TRAP is try/catch. The above
@ routines MUST resolve to euser.dll's real exports. Dead 'bx lr' stubs broke
@ stack unwinding -> KERN-EXEC 3 on the first User::Leave (in BaseConstructL).