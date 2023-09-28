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

    // load imm4,v4,v4,v4,v4,v4 into %r13d
    movl (%rax), %r13d

    // ABI arg reg 1 (rsi) <- num_args
    movl %r13d, %esi
    andl $0x000F, %esi

    test %esi, %esi
    je .Linvoke

    // Reserve stack args
    leal (, %esi, 8), %r14d
    subq %r14, %rsp
    andq $-16, %rsp

.macro load_calli_dyn_short_arg i
    movl %r13d, %r14d
    shrl $(4 * (\i + 1) - 3), %r14d
    andl $(0xF << 3), %r14d
    movq (%rbx, %r14), %r14
    movq %r14, (8 * \i)(%rsp)
.endm

    load_calli_dyn_short_arg 0
    cmpl $1, %esi
    je .Linvoke

    load_calli_dyn_short_arg 1
    cmpl $2, %esi
    je .Linvoke

    load_calli_dyn_short_arg 2
    cmpl $3, %esi
    je .Linvoke

    load_calli_dyn_short_arg 3
    cmpl $4, %esi
    je .Linvoke

    load_calli_dyn_short_arg 4
    jmp .Linvoke
