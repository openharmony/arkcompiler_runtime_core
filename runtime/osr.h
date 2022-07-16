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
#ifndef PANDA_OSR_H
#define PANDA_OSR_H

#include "libpandabase/macros.h"
#include "runtime/include/cframe.h"
#include "runtime/interpreter/frame.h"
#include "libpandabase/os/mutex.h"
#include "include/method.h"

namespace panda {

extern "C" void *PrepareOsrEntry(const Frame *iframe, uintptr_t bc_offset, const void *osr_code, void *cframe_ptr,
                                 uintptr_t *reg_buffer, uintptr_t *fp_reg_buffer);

bool OsrEntry(uintptr_t loop_head_bc, const void *osr_code);

extern "C" void OsrEntryAfterCFrame(Frame *frame, uintptr_t loop_head_bc, const void *osr_code, size_t frame_size);
extern "C" void OsrEntryAfterIFrame(Frame *frame, uintptr_t loop_head_bc, const void *osr_code, size_t frame_size);
extern "C" void OsrEntryTopFrame(Frame *frame, uintptr_t loop_head_bc, const void *osr_code, size_t frame_size);

extern "C" void SetOsrResult(Frame *frame, uint64_t uval, double fval);

class OsrCodeMap {
public:
    void *Get(const Method *method)
    {
        os::memory::ReadLockHolder holder(osr_lock_);
        auto iter = code_map_.find(method);
        if (UNLIKELY(iter == code_map_.end())) {
            return nullptr;
        }
        return iter->second;
    }

    void Set(const Method *method, void *ptr)
    {
        if (UNLIKELY(ptr == nullptr)) {
            Remove(method);
            return;
        }
        os::memory::WriteLockHolder holder(osr_lock_);
        code_map_.insert({method, ptr});
    }

    void Remove(const Method *method)
    {
        os::memory::WriteLockHolder holder(osr_lock_);
        code_map_.erase(method);
    }

private:
    os::memory::RWLock osr_lock_;
    PandaMap<const Method *, void *> code_map_;
};

}  // namespace panda

#endif  // PANDA_OSR_H
