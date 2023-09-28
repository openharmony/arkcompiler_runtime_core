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

    // load imm4,v4,v4,v4,v4,v4 into w13
    ldr w13, [x10]

    // ABI arg reg 1 (x1/w1) <- num_args
    and w1, w13, 0x000F

    cbz w1, .Linvoke

    // Reserve stack args
    sub x14, sp, x1, lsl 3
    and sp, x14, -16

.macro load_calli_dyn_short_arg i
    lsr w14, w13, (4 * (\i + 1) - 3)
    and w14, w14, (0xF << 3)
    ldr x14, [x9, x14]
    str x14, [sp, 8 * \i]
.endm

    load_calli_dyn_short_arg 0
    cmp w1, 1
    beq .Linvoke

    load_calli_dyn_short_arg 1
    cmp w1, 2
    beq .Linvoke

    load_calli_dyn_short_arg 2
    cmp w1, 3
    beq .Linvoke

    load_calli_dyn_short_arg 3
    cmp w1, 4
    beq .Linvoke

    load_calli_dyn_short_arg 4
    b .Linvoke
