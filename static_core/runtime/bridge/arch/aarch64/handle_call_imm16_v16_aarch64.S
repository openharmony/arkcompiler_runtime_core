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

    // ABI arg reg 0 (x0) <- panda::Method*
    mov x0, x12

    // ABI arg reg 1 (x1) <- num_args
    ldrh w1, [x10]

    cbz w1, .Linvoke

    // x9 <- range in frame.vregs
    ldrh w13, [x10, 2]
    add x9, x9, x13, lsl 3

    // Reserve stack args
    sub x13, sp, x1, lsl 3
    and sp, x13, -16

    lsl w13, w1, 3
    sub w13, w13, 8
1:
    ldr x14, [x9, x13]
    str x14, [sp, x13]
    subs w13, w13, 8
    bhs 1b

    b .Linvoke
