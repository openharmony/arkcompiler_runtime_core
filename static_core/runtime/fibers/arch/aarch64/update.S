/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/aarch64/context_layout.h"

.extern abort

LOCAL_FUNCTION_START(FiberExitBridge)
    // indicate abnormal exit by calling exit(-1)
    // because fibers should not return from FiberProc
    // mov x0, #-1
    // bl exit

    // let's make the exit more violent than exit(-1) for now
    // TODO(konstanting, #IAD5MH): revert to exit() when everything is tested and works
    bl abort
FUNCTION_END(FiberExitBridge)

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg

/**
 * arguments:
 * x0: uint8_t* ctx_memory
 * x1: void (*func)(void*)
 * x2: void* argument
 * x3: uint8_t* stack
 * x4: size_t stack_size_bytes
 */
FUNCTION_START(UpdateContext)
    // context->PC = func
    str x1, [x0, GPR_O(PC)]
    // context->LR = FiberExitBridge (for the case when Fiber returns)
    adr x8, FiberExitBridge
    str x8, [x0, GPR_O(LR)]
    // context->X0 = argument
    str x2, [x0, GPR_O(R0)]

    /**
     * %x8 will hold the new SP value:
     * NEW_SP = align((stack + stack_size), 16B)
     */

    // %x8 = stack + stack_size_bytes
    add x8, x3, x4
    // %x8 = align(%x8, 16B)
    and x8, x8, #-16
    // context->SP = %x8
    str x8, [x0, GPR_O(SP)]
    // context->FP = 0
    str xzr, [x0, GPR_O(FP)]

    // return success
    mov x0, 0
    ret
FUNCTION_END(UpdateContext)

/**
 * arguments:
 * x0: uint8_t* ctx_memory
 * x1: void (*func)(void*)
 * x2: void* argument
 */
FUNCTION_START(UpdateContextKeepStack)
    // %x8 = context->PC
    ldr x8, [x0, GPR_O(PC)]
    // context->PC = func
    str x1, [x0, GPR_O(PC)]
    // context->X0 = argument
    str x2, [x0, GPR_O(R0)]

    // pretend that we called the func(), i.e. put context's PC to LR
    // context->LR = %x8
    str x8, [x0, GPR_O(LR)]

    // return success
    mov x0, 0
    ret
FUNCTION_END(UpdateContextKeepStack)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits