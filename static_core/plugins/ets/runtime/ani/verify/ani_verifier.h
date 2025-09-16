/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_VERIFIER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_VERIFIER_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vmethod.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include <string_view>

namespace ark::ets::ani::verify {

class ANIVerifier {
public:
    ANIVerifier() = default;
    ~ANIVerifier() = default;

    void Abort(const std::string_view message);
    void SetAbortHook(void (*hook)(void *data, const std::string_view message), void *data)
    {
        abortHook_ = hook;
        abortHookData_ = data;
    }

    VRef *AddGloablVerifiedRef(ani_ref gref);
    void DeleteDeleteGlobalRef(VRef *vgref);
    bool IsValidGlobalVerifiedRef(VRef *vgref);

    impl::VMethod *AddMethod(EtsMethod *method);
    void DeleteMethod(impl::VMethod *vmethod);
    bool IsValidVerifiedMethod(impl::VMethod *vmethod);

private:
    void (*abortHook_)(void *data, const std::string_view message) {};
    void *abortHookData_ {};

    PandaMap<VRef *, PandaUniquePtr<VRef>> grefsMap_ GUARDED_BY(grefsMapMutex_);
    os::memory::Mutex grefsMapMutex_;
    PandaMap<impl::VMethod *, PandaUniquePtr<impl::VMethod>> methodsSet_ GUARDED_BY(methodsSetLock_);
    mutable os::memory::RWLock methodsSetLock_;

    NO_COPY_SEMANTIC(ANIVerifier);
    NO_MOVE_SEMANTIC(ANIVerifier);
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_VERIFIER_H
