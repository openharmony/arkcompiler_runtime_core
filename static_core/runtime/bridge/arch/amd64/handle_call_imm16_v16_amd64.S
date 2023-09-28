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

    test %esi, %esi
    je .Linvoke

    // rbx <- range in frame.vregs
    movzwl 2(%rax), %r14d
    leaq (%rbx, %r14, FRAME_VREGISTER_SIZE), %rbx

    // Reserve stack args
    leal (, %esi, 8), %r14d
    subq %r14, %rsp
    andq $-16, %rsp

    leal -FRAME_VREGISTER_SIZE(, %esi, FRAME_VREGISTER_SIZE), %r14d
1:
    movq (%rbx, %r14), %r13
    movq %r13, (%rsp, %r14)
    subl $FRAME_VREGISTER_SIZE, %r14d
    jae 1b

    jmp .Linvoke
