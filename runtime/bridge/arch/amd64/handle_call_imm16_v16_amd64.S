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

// handle calli.dyn.range
// regs set as follow
// %rax - insn_ptr, %rbx - frame.vregs, %r12 - method, %r13, %r14, %r15 - temp

    // ABI arg reg 0 (rdi) <- panda::Method*
    movq %r12, %rdi

    // ABI arg reg 1 (rsi) <- num_args
    movzwl (%rax), %esi

    cmpl $0, %esi
    je .Linvoke

    // rbx <- vreg pointed by the insn (user's first argument)
    movzwl 2(%rax), %eax
    shll $3, %eax
    addq %rax, %rbx

    // rax <- counter of args
    movl %esi, %eax

    // ABI arg reg 2 (rdx) <- boxed arg0 from user's code
    movq (%rbx), %rdx
    addq $FRAME_VREGISTER_SIZE, %rbx
    subl $1, %eax
    je .Linvoke

    // ABI arg reg 3 (rcx) <- boxed arg1 from user's code
    movq (%rbx), %rcx
    addq $FRAME_VREGISTER_SIZE, %rbx
    subl $1, %eax
    je .Linvoke

    // ABI arg reg 4 (r8) <- boxed arg2 from user's code
    movq (%rbx), %r8
    addq $FRAME_VREGISTER_SIZE, %rbx
    subl $1, %eax
    je .Linvoke

    // ABI arg reg 5 (r9) <- boxed arg3 from user's code
    movq (%rbx), %r9
    addq $FRAME_VREGISTER_SIZE, %rbx
    subl $1, %eax
    je .Linvoke

    // Reserve stack space for the arguments, starting from r13:
    movl %eax, %r13d
    shll $3, %r13d
    subq %r13, %rsp
    andq $-16, %rsp
    movq %rsp, %r13

    // Push the rest arguments to the stack (r14 is temp):
1:  movq (%rbx), %r14
    movq %r14, (%r13)
    addq $FRAME_VREGISTER_SIZE, %r13
    addq $FRAME_VREGISTER_SIZE, %rbx
    subl $1, %eax
    ja 1b

    jmp .Linvoke
