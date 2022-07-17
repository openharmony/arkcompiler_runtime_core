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
// x0-x7 - arguments, x9 - frame.vregs, x10 - insn_ptr, x12 - method, x19 - frame
// x13, x14, x15 - temp

    // Prepare to set up ABI regs 0 and 1:
    ldrb w0, [x10], 1
    ldrb w1, [x10], 1

    // ABI arg reg 1 (x1) <- num_args
    lsl w1, w1, 8
    orr w1, w0, w1

    // ABI arg reg 0 (x0) <- panda::Method*
    mov x0, x12

    cbz w1, .Linvoke

    // x9 <- frame.vregs + vreg_idx (copy base)
    ldrb w2, [x10], 1
    ldrb w3, [x10]
    lsl w3, w3, 8
    orr w2, w2, w3
    add x9, x9, x2, lsl 3

    // w10 <- counter of args
    mov w10, w1

    // ABI arg reg 2 (r2) <- boxed arg0 from user's code
    ldr x2, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // ABI arg reg 3 (r3) <- boxed arg1 from user's code
    ldr x3, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // ABI arg reg 4 (r4) <- boxed arg2 from user's code
    ldr x4, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // ABI arg reg 5 (r5) <- boxed arg3 from user's code
    ldr x5, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // ABI arg reg 6 (r6) <- boxed arg4 from user's code
    ldr x6, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // ABI arg reg 7 (r7) <- boxed arg5 from user's code
    ldr x7, [x9], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbz w10, .Linvoke

    // Reserve stack space for the arguments, starting from x13:
    sub sp, sp, x10, lsl 3
    mov x13, sp

    // Push the rest arguments to the stack (r14 is temp):
1:  ldr x14, [x9], FRAME_VREGISTER_SIZE
    str x14, [x13], FRAME_VREGISTER_SIZE
    sub w10, w10, 1
    cbnz w10, 1b
    b .Linvoke
