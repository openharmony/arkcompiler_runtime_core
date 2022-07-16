/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

// handle calli.dyn.short || calli.dyn
// regs set as follow
// %rax - insn_ptr, %rbx - frame.vregs, %r12 - method, %r13, %r14, %r15 - temp

    // ABI arg reg 0 (rdi) <- panda::Method*
    movq %r12, %rdi

    // ABI arg reg 1 (rsi) <- num_args
    // load num args into %rsi and function object into %rdx
    movzbl (%rax), %r13d
    addq $1, %rax // advance insn_ptr
    movl %r13d, %esi
    andl $0x0F, %esi

    cmpl $0, %esi
    je .Linvoke

    // ABI arg reg 2 (rdx) <- boxed arg0 from user's code
    andl $0xF0, %r13d
    shrl $1, %r13d
    movq (%rbx, %r13), %rdx

    cmpl $1, %esi // No more args from user's code?
    je .Linvoke

    // Read next insn byte for user's arg1 and arg2:
    movzbl (%rax), %r14d
    movl %r14d, %r13d
    addq $1, %rax // advance insn_ptr

    // ABI arg reg 3 (rcx) <- boxed arg1 from user's code
    andl $0x0F, %r14d
    shll $3, %r14d
    movq (%rbx, %r14), %rcx
    cmpl $2, %esi // No more args from user's code?
    je .Linvoke

    // ABI arg reg 4 (r8) <- boxed arg2 from user's code
    andl $0xF0, %r13d
    shrl $1, %r13d
    movq (%rbx, %r13), %r8
    cmpl $3, %esi // No more args from user's code?
    je .Linvoke

    // Read next insn byte for user's arg3 and arg4:
    movzbl (%rax), %r14d
    movl %r14d, %r13d
    addq $1, %rax // advance insn_ptr

    // ABI arg reg 5 (r9) <- boxed arg3 from user's code
    andl $0x0F, %r14d
    shll $3, %r14d
    movq (%rbx, %r14), %r9
    cmpl $4, %esi // No more args from user's code?
    je .Linvoke

    // Stack slot <- boxed arg4 from user's code
    subq $8, %rsp
    andl $0xF0, %r13d
    shrl $1, %r13d
    movq (%rbx, %r13), %r13
    movq %r13, (%rsp)

    jmp .Linvoke
