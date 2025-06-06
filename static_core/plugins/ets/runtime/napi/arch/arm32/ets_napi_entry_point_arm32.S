/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arch/asm_support.h"
#include "arch/arm/asm_constants.h"
#include "arch/arm/shorty.S"
#include "shorty_values.h"

// uint32_t EtsNapiCalcStackArgsSpaceSize (Method *method, bool is_critical)
.extern EtsNapiCalcStackArgsSpaceSize
// void EtsNapiBeginCritical (Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern EtsNapiBeginCritical
// void EtsNapiBegin (Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern EtsNapiBegin
// void EtsNapiEnd (Method *method)
.extern EtsNapiEnd
// ObjectHeader *EtsNapiObjEnd (Method *method, Reference *ref)
.extern EtsNapiObjEnd
// bool IsEtsMethodFastNative (Method *method)
.extern IsEtsMethodFastNative

// The entrypoint for EtsNapi critical method.
// there are two nuances:
// 1. Each panda method accepts Method* in the first argument.
//    We have to drop this argument and shift others back
// 2. We have to convert double arguments and the return value to floats
//    Panda runtime operates only with doubles.
// The function handles all the cases above.
.global EtsNapiCriticalNativeEntryPoint
.type EtsNapiCriticalNativeEntryPoint, %function
EtsNapiCriticalNativeEntryPoint:
    CFI_STARTPROC
    CFI_DEF_CFA(sp,0)

    push {fp, lr}
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)

    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #8
    str r0, [sp, #4]
    mov lr, #CFRAME_KIND_NATIVE
    str lr, [sp]

    push {r4, r7}
    CFI_REL_OFFSET(r7, -12)
    CFI_REL_OFFSET(r4, -16)

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 4)

    // save argument to the stack
    push {r0 - r3}

    mov r4, r0 // save method to r4 to survive the call

    // Update current frame in the thread
    push {r0}
    blx GetCurrentManagedThread
    str fp, [r0, #MANAGED_THREAD_FRAME_OFFSET]
    ldrb r7, [r0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov r1, #1
    strb r1, [r0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov THREAD_REG, r0
    pop {r0}

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    ldr r0, [r4, #METHOD_NUM_ARGS_OFFSET]
    cmp r0, #2 // max 2 long argument
    // reserve space for GPR and FPR args
    mov r0, #16
    ble .Lprepare_stack_critical

    mov r0, r4
    mov r1, #1
    blx EtsNapiCalcStackArgsSpaceSize

.Lprepare_stack_critical:
    mov r1, sp
    add r2, fp, #8
    sub sp, sp, r0
    mov r3, sp
    mov r0, r4
    // necessary for stack alignment
    sub sp, sp, #4
    push {THREAD_REG}
    blx EtsNapiBeginCritical
    add sp, sp, #8

    // load the argument
    pop {r0 - r3}

    // call the method
    ldr lr, [r4, #METHOD_NATIVE_POINTER_OFFSET]
    cmp lr, #0
    beq .Ldone
    blx lr

.Ldone:
    // return
    // signal handler of the sampling profiler use stack space below sp, 
    // so change it carefully only after registers restoration
    sub sp, fp, #16
    
    mov r2, r7

    pop {r4}
    CFI_RESTORE(r4)
    ldr r7, [sp], #12
    CFI_RESTORE(r7)
    pop {fp}
    CFI_DEF_CFA(sp, 4)
    CFI_RESTORE(fp)
    CFI_REMEMBER_STATE

    strb r2, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // check native exception
    ldr r3, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp r3, #0
    beq 1f

    // check frame is compiled
    cmp r2, #0
    beq 1f

    // check prev fame is true CFRAME and not BYPASS
    ldr r3, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    cmp r3, #BYPASS_BRIDGE
    beq 1f

    add lr, fp, #-CALLER_REG0_OFFSET
    ldm lr, {r0-r3}

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    ldr lr, [fp, #(-CFRAME_FLAGS_SLOT * 4)]
    tst lr, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
    beq 4f

    add lr, fp, #-CALLER_VREG0_OFFSET
    vldm lr, {d0-d7}
4:
#endif

    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)

    b ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

1:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)
    bx lr
    CFI_ENDPROC

// |                        |
// |       Prev Frame       |
// |                        |
// +------------------------+
// |          ...           |
// |       stack args       |
// |          ...           |-11
// +---+---+----------------+
// |   |   |       LR       |-10
// |   |   |       FP       | -9
// |   |   |     Method *   | -8
// |   |   |      FLAGS     | -7
// |   |   +----------------+
// |   |   |       ...      | -6
// |   |   |      locals    |
// |   |   |       ...      | -1
// |   | c +--------+-------+
// |   | f |        | d15.1 | 0
// |   | r |        | d15.0 | 1
// |   | a |        | d14.1 | 2
// |   | m |        | d14.0 | 3
// |   | e |        | d13.1 | 4
// |   |   |        | d13.0 | 5
// |   |   |        | d12.1 | 6
// |   |   |        | d12.0 | 7
// |   |   |        | d11.1 | 8
// |   |   |        | d11.0 | 9
// |   |   |        | d10.1 | 10
// | N |   |        | d10.0 | 11
// | a |   | callee |  d9.1 | 12
// | p |   |  saved |  d9.0 | 13
// | i |   |        |  d8.1 | 14
// |   |   |        |  d8.0 | 15
// | f |   |        +-------+
// | r |   |        |   r10 | 16
// | a |   |        |    r9 | 17
// | m |   |        |    r8 | 18
// | e |   |        |    r7 | 19
// |   |   |        |    r6 | 20
// |   |   |        |    r5 | 21
// |   |   |        |    r4 | 22
// |   |   |        | align | 23
// |   +---+--------+-------+
// |   |            |    r3 | 24
// |   |            |    r2 | 25
// |   |    args    |    r1 | 26
// |   |            |    r0 | 27
// |   +-------+----+-------+
// |   |       |    Napi r3 | 28
// |   |       |    Napi r2 | 29
// |   |       |    Napi r1 | 30
// |   |  Napi |    Napi r0 | 31
// |   |  args +------------+
// |   |       | stack_argN | 32
// |   |       |   ...      |
// |   |       | stack_arg1 | 32+(N-1)
// |   |       | stack_arg0 | 32+(N-0)
// +---+-------+------------+
// |                        |

// The entrypoint for EtsNapi method
// Each panda method accepts *Method in the first argument
// The goal of this function is just drop this argument and shift other arguments back
.global EtsNapiEntryPoint
.type EtsNapiEntryPoint, %function
EtsNapiEntryPoint:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    push {fp, lr}
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)

    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #8
    str r0, [sp, #4]
    mov lr, #CFRAME_KIND_NATIVE
    str lr, [sp]

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 4)

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    vpush {d8 - d15}
    CFI_REL_OFFSET(d15, -((CFRAME_LOCALS_COUNT + 2 + 2) * 4))
    CFI_REL_OFFSET(d14, -((CFRAME_LOCALS_COUNT + 2 + 4) * 4))
    CFI_REL_OFFSET(d13, -((CFRAME_LOCALS_COUNT + 2 + 6) * 4))
    CFI_REL_OFFSET(d12, -((CFRAME_LOCALS_COUNT + 2 + 8) * 4))
    CFI_REL_OFFSET(d11, -((CFRAME_LOCALS_COUNT + 2 + 10) * 4))
    CFI_REL_OFFSET(d10, -((CFRAME_LOCALS_COUNT + 2 + 12) * 4))
    CFI_REL_OFFSET(d9,  -((CFRAME_LOCALS_COUNT + 2 + 14) * 4))
    CFI_REL_OFFSET(d8,  -((CFRAME_LOCALS_COUNT + 2 + 16) * 4))
#else
    sub sp, sp, #(8 * 8)
#endif
    push {r4 - r10}
    CFI_REL_OFFSET(r10, -((CFRAME_LOCALS_COUNT + 18 + 1) * 4)) // Thread pointer
    CFI_REL_OFFSET(r9,  -((CFRAME_LOCALS_COUNT + 18 + 2) * 4)) // Shorty return value 
    CFI_REL_OFFSET(r8,  -((CFRAME_LOCALS_COUNT + 18 + 3) * 4)) // IsFastNative state
    CFI_REL_OFFSET(r7,  -((CFRAME_LOCALS_COUNT + 18 + 4) * 4))
    CFI_REL_OFFSET(r6,  -((CFRAME_LOCALS_COUNT + 18 + 5) * 4))
    CFI_REL_OFFSET(r5,  -((CFRAME_LOCALS_COUNT + 18 + 6) * 4))
    CFI_REL_OFFSET(r4,  -((CFRAME_LOCALS_COUNT + 18 + 7) * 4)) // Method ptr

    // align to 8
    sub sp, sp, #4

    // save arguments to the stack
    push {r0 - r3}

    mov r4, r0 // save method to r4 to survive the call

    // save shorty return value to r9 
    ldr r9, [r4, #METHOD_SHORTY_OFFSET]
    ldr r9, [r9]

    // save IsFastNative state to r8 
    mov r0, r4
    blx IsEtsMethodFastNative
    mov r8, r0
    
    // Update current frame in the thread
    blx GetCurrentManagedThread
    str fp, [r0, #MANAGED_THREAD_FRAME_OFFSET]
    ldrb r7, [r0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov r1, #1
    strb r1, [r0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov THREAD_REG, r0
    mov r0, r4

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    ldr r0, [r4, #METHOD_NUM_ARGS_OFFSET]
    cmp r0, #1 // max one long argument
    // reserve space for GPR and FPR args
    mov r0, #16
    ble .Lprepare_stack

    mov r0, r4
    mov r1, #0
    blx EtsNapiCalcStackArgsSpaceSize

.Lprepare_stack:
    mov r1, sp
    add r2, fp, #8
    sub sp, sp, r0
    mov r3, sp
    mov r0, r4
    // necessary for stack alignment
    sub sp, sp, #4
    push {THREAD_REG}
    blx EtsNapiBegin
    add sp, sp, #8

    // load the arguments
    mov r5, r0
    ldmia r5!, {r0 - r3}
#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    fldmiax r5!, {d0 - d7}
#endif

    // call the method
    ldr lr, [r4, #METHOD_NATIVE_POINTER_OFFSET]
    cmp lr, #0
    beq .Lend
    blx lr

    // handle the result
    and r9, r9, #0xF
    cmp r9, #SHORTY_REFERENCE
    bne 2f
    // it is a reference.
    mov r3, r8
    mov r2, THREAD_REG
    mov r1, r0
    mov r0, r4
    blx EtsNapiObjEnd
    b .Lreturn

2:  mov r5, r0
    mov r6, r1
    mov r0, r4
    mov r1, THREAD_REG
    mov r2, r8
    blx EtsNapiEnd
    mov r0, r5
    mov r1, r6
    b .Lreturn

.Lend:
    mov r0, r4
    mov r1, THREAD_REG
    mov r2, r8
    bl EtsNapiEnd

.Lreturn:
    // Restore callee registers, since GC may change its values while moving objects.
    sub sp, fp, #(CFRAME_ARM_SOFTFP_CALLEE_REGS_OFFSET - 4)

    mov r2, r7

    ldm sp, {r4 - r10}
    CFI_RESTORE(r10)
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)
    mov sp, fp
    CFI_DEF_CFA(sp, 8)
    pop {fp}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(fp)
    CFI_REMEMBER_STATE

    strb r2, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // check native exception
    ldr r3, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp r3, #0
    beq 3f

    // check frame is compiled
    cmp r2, #0
    beq 3f

    // check prev frame is true CFRAME and not BYPASS
    ldr r3, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    cmp r3, #BYPASS_BRIDGE
    beq 3f

    add lr, fp, #-CALLER_REG0_OFFSET
    ldm lr, {r0-r3}

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    ldr lr, [fp, #(-CFRAME_FLAGS_SLOT * 4)]
    tst lr, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
    beq 5f

    add lr, fp, #-CALLER_VREG0_OFFSET
    vldm lr, {d0-d7}
5:
#endif

    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)

    b ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

3:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)
    bx lr
    CFI_ENDPROC

