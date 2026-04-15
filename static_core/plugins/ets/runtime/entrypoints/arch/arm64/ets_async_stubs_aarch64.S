/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "arch/aarch64/helpers_aarch64.S"

.macro POP_CALLEE_GP_REGS base_reg
    ldp x19, x20, [\base_reg], #16
    ldp x21, x22, [\base_reg], #16
    ldp x23, x24, [\base_reg], #16
    ldp x25, x26, [\base_reg], #16
    ldp x27, x28, [\base_reg], #16
.endm

.macro POP_CALLEE_FP_REGS base_reg
    ldp d8, d9, [\base_reg], #16
    ldp d10, d11, [\base_reg], #16
    ldp d12, d13, [\base_reg], #16
    ldp d14, d15, [\base_reg], #16
.endm

.extern EtsAsyncDispatchImpl
.extern EtsAsyncSuspendImpl

.global EtsAsyncDispatchStub
.type EtsAsyncDispatchStub, %function
EtsAsyncDispatchStub:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    str lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(lr, 0)

    // Move async context from arg0 to arg1
    mov x1, x0
    // Pass current managed thread as arg0
    mov x0, THREAD_REG
    // Pass current compiled frame base pointer as arg2
    mov x2, fp

    bl EtsAsyncDispatchImpl

    ldr lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_ADJUST_CFA_OFFSET(-16)

    // Check whether runtime returned a native resume address.
    cmp x0, #0
    // If there is no resume target, leave through the normal return path.
    beq .Ldispatch_exit

    // Jump directly to resume address (x0)
    br x0

.Ldispatch_exit:
    // Return to the original caller when there is nothing to resume.
    ret
    // Finish unwind information for the dispatch stub.
    CFI_ENDPROC

.global EtsAsyncSuspendStub
.type EtsAsyncSuspendStub, %function
EtsAsyncSuspendStub:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // Move async context from arg0 to arg1
    mov x1, x0
    // Pass current managed thread as arg0
    mov x0, THREAD_REG
    // Pass current compiled frame base pointer as arg2
    mov x2, fp
    // Pass caller return address as arg3 (from lr)
    mov x3, lr

    bl EtsAsyncSuspendImpl

    // Rewind to the first saved callee register slot in the current compiled CFrame.
    ldr lr, [fp, #(-CFRAME_FLAGS_SLOT * SLOT_SIZE)]
    tbz lr, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, .Lrestore_gp_regs

    sub sp, fp, #(CFRAME_CALLEE_VREGS_END_SLOT * SLOT_SIZE)
    POP_CALLEE_FP_REGS sp

.Lrestore_gp_regs:
    sub sp, fp, #(CFRAME_CALLEE_REGS_END_SLOT * SLOT_SIZE)
    POP_CALLEE_GP_REGS sp

    // Pop frame and return
    mov sp, fp
    ldp fp, lr, [sp], #16
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC