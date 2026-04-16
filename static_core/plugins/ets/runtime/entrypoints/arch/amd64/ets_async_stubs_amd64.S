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
#include "arch/amd64/helpers_amd64.S"

.extern EtsAsyncDispatchImpl
.extern EtsAsyncSuspendImpl

.global EtsAsyncDispatchStub
.type EtsAsyncDispatchStub, %function
EtsAsyncDispatchStub:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // Move async context from arg0 to arg1.
    movq %rdi, %rsi
    // Pass current managed thread as arg0.
    movq %THREAD_REG, %rdi
    // Pass current compiled frame base pointer as arg2.
    movq %rbp, %rdx

    subq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(8)
    callq EtsAsyncDispatchImpl@plt
    addq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(-8)

    // Check whether runtime returned a native resume address.
    testq %rax, %rax
    // If there is no resume target, leave through the normal return path.
    jz .Ldispatch_exit
    // Remove return address from stack.
    addq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(-8)
    // Otherwise jump directly to the compiled resume entry.
    jmp *%rax

    // Local exit path when dispatch falls through.
.Ldispatch_exit:
    // Return to the original caller when there is nothing to resume.
    retq
    // Finish unwind information for the dispatch stub.
    CFI_ENDPROC

.global EtsAsyncSuspendStub
.type EtsAsyncSuspendStub, %function
EtsAsyncSuspendStub:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // Move async context from arg0 to arg1.
    movq %rdi, %rsi
    // Pass current managed thread as arg0.
    movq %THREAD_REG, %rdi
    // Pass current compiled frame base pointer as arg2.
    movq %rbp, %rdx
    // Pass the caller return address as arg3.
    movq (%rsp), %rcx

    subq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(8)
    callq EtsAsyncSuspendImpl@plt
    addq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(-8)

    // Rewind to the first saved callee register slot in the current compiled CFrame.
    leaq -(CFRAME_CALLEE_VREGS_END_SLOT * SLOT_SIZE)(%rbp), %rsp

    // Restore callee-saved registers in the same order as the compiled epilogue.
    popq %rbx
    popq %r12
    popq %r13
    popq %r14
    popq %THREAD_REG

    movq %rbp, %rsp
    popq %rbp
    CFI_DEF_CFA(rsp, 8)
    retq
    CFI_ENDPROC
