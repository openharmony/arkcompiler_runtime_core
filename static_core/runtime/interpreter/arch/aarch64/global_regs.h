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
#ifndef PANDA_INTERPRETER_ARCH_AARCH64_GLOBAL_REGS_H
#define PANDA_INTERPRETER_ARCH_AARCH64_GLOBAL_REGS_H

#include <cstdint>

namespace panda {
class Frame;
class ManagedThread;
}  //  namespace panda

namespace panda::interpreter::arch::regs {

#ifndef FFIXED_REGISTERS
#error "Need to compile source with -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x28"
#endif

// NOLINTBEGIN(hicpp-no-assembler, misc-definitions-in-headers)
register const uint8_t *G_PC asm("x20");
register int64_t G_ACC_VALUE asm("x21");
register int64_t G_ACC_TAG asm("x22");
register void *G_FP asm("x23");
register const void *const *G_DISPATCH_TABLE asm("x24");
register void *G_M_FP asm("x25");
register ManagedThread *G_THREAD asm("x28");
// NOLINTEND(hicpp-no-assembler, misc-definitions-in-headers)

ALWAYS_INLINE inline const uint8_t *GetPc()
{
    return G_PC;
}

ALWAYS_INLINE inline void SetPc(const uint8_t *pc)
{
    G_PC = pc;
}

ALWAYS_INLINE inline int64_t GetAccValue()
{
    return G_ACC_VALUE;
}

ALWAYS_INLINE inline void SetAccValue(int64_t value)
{
    G_ACC_VALUE = value;
}

ALWAYS_INLINE inline int64_t GetAccTag()
{
    return G_ACC_TAG;
}

ALWAYS_INLINE inline void SetAccTag(int64_t tag)
{
    G_ACC_TAG = tag;
}

ALWAYS_INLINE inline Frame *GetFrame()
{
    return reinterpret_cast<Frame *>(reinterpret_cast<uintptr_t>(G_FP) - Frame::GetVregsOffset());
}

ALWAYS_INLINE inline void SetFrame(Frame *frame)
{
    G_FP = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(frame) + Frame::GetVregsOffset());
}

ALWAYS_INLINE inline void *GetFp()
{
    return G_FP;
}

ALWAYS_INLINE inline void SetFp(void *fp)
{
    G_FP = fp;
}

ALWAYS_INLINE inline void *GetMirrorFp()
{
    return G_M_FP;
}

ALWAYS_INLINE inline void SetMirrorFp(void *m_fp)
{
    G_M_FP = m_fp;
}

ALWAYS_INLINE inline const void *const *GetDispatchTable()
{
    return G_DISPATCH_TABLE;
}

ALWAYS_INLINE inline void SetDispatchTable(const void *const *dispatch_table)
{
    G_DISPATCH_TABLE = dispatch_table;
}

ALWAYS_INLINE inline ManagedThread *GetThread()
{
    return G_THREAD;
}

ALWAYS_INLINE inline void SetThread(ManagedThread *thread)
{
    G_THREAD = thread;
}

}  // namespace panda::interpreter::arch::regs

#endif  // PANDA_INTERPRETER_ARCH_AARCH64_GLOBAL_REG_VARS_H
