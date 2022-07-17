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

#ifndef PANDA_TOOLING_PT_THREAD_INFO_H
#define PANDA_TOOLING_PT_THREAD_INFO_H

#include <cstdint>

#include "runtime/include/panda_vm.h"
#include "runtime/include/tooling/pt_object.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "pt_hook_type_info.h"

namespace panda::tooling {
class PtThreadInfo {
public:
    PtThreadInfo() = default;

    ~PtThreadInfo()
    {
        ASSERT(managed_thread_ref_ == nullptr);
    }

    PtHookTypeInfo &GetHookTypeInfo()
    {
        return hook_type_info_;
    }

    void SetThreadObjectHeader(ObjectHeader *threadObjectHeader)
    {
        ASSERT(managed_thread_ref_ == nullptr);
        managed_thread_ref_ = PandaVM::GetCurrent()->GetGlobalObjectStorage()->Add(threadObjectHeader,
                                                                                   mem::Reference::ObjectType::GLOBAL);
    }

    void Destroy()
    {
        if (managed_thread_ref_ != nullptr) {
            PandaVM::GetCurrent()->GetGlobalObjectStorage()->Remove(managed_thread_ref_);
            managed_thread_ref_ = nullptr;
        }
    }

    ObjectHeader *GetThreadObjectHeader() const
    {
        ASSERT(managed_thread_ref_ != nullptr);
        return PandaVM::GetCurrent()->GetGlobalObjectStorage()->Get(managed_thread_ref_);
    }

private:
    PtHookTypeInfo hook_type_info_ {false};
    mem::Reference *managed_thread_ref_ {nullptr};

    NO_COPY_SEMANTIC(PtThreadInfo);
    NO_MOVE_SEMANTIC(PtThreadInfo);
};
}  // namespace panda::tooling

#endif  // PANDA_TOOLING_PT_THREAD_INFO_H
