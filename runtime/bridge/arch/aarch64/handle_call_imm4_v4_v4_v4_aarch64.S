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
// x0-x7 - arguments, x9 - frame.vregs, x10 - insn_ptr, x12 - method, x19 - frame
// x13, x14, x15 - temp

    // ABI arg reg 0 (x0) <- panda::Method*
    mov x0, x12

    // ABI arg reg 1 (x1/w1) <- num_args
    ldrb w13, [x10], 1
    and w1, w13, 0xF

    cbz w1, .Linvoke

    // ABI arg reg 2 (x2) <- boxed arg0 from user's code
    and w2, w13, -16
    lsr w2, w2, #1
    add x2, x9, x2
    ldr x2, [x2]
    cmp w1, 1
    beq .Linvoke // No more args from user's code?

    // Read next insn byte for user's arg1 and arg2:
    ldrb w13, [x10], 1

    // ABI arg reg 3 (x3) <- boxed arg1 from user's code
    and w3, w13, 0xF
    add x3, x9, x3, lsl 3
    ldr x3, [x3]
    cmp w1, 2
    beq .Linvoke // No more args from user's code?

    // ABI arg reg 4 (x4) <- boxed arg2 from user's code
    and w4, w13, -16
    lsr w4, w4, #1
    add x4, x9, x4
    ldr x4, [x4]
    cmp w1, 3
    beq .Linvoke // No more args from user's code?

    // Read next insn byte for user's arg3 and arg4:
    ldrb w13, [x10], 1

    // ABI arg reg 5 (x5) <- boxed arg3 from user's code
    and w5, w13, 0xF
    add x5, x9, x5, lsl 3
    ldr x5, [x5]
    cmp w1, 4
    beq .Linvoke // No more args from user's code?

    // ABI arg reg 6 (x6) <- boxed arg4 from user's code
    and w6, w13, -16
    lsr w6, w6, #1
    add x6, x9, x6
    ldr x6, [x6]

    b .Linvoke
