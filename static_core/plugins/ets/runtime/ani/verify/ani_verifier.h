/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ani/ani_options.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vwref.h"
#include "plugins/ets/runtime/ani/verify/types/vmethod.h"
#include "plugins/ets/runtime/ani/verify/types/vfield.h"
#include "plugins/ets/runtime/ani/verify/types/vresolver.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include <string_view>

namespace ark::ets::ani::verify {

class ANIVerifier {
public:
    ANIVerifier() = default;
    ~ANIVerifier() = default;

    struct GlobalData {
        struct MemRange {
            uintptr_t start;
            uintptr_t end;
        };
        using MethodRange = MemRange;
        using FieldRange = MemRange;
        PandaMap<VRef *, PandaUniquePtr<InternalRef>> grefsMap GUARDED_BY(grefsMapMutex);
        os::memory::Mutex grefsMapMutex;

        PandaMap<VWRef *, PandaUniquePtr<InternalRef>> wrefsMap GUARDED_BY(wrefsMapMutex);
        os::memory::Mutex wrefsMapMutex;

        PandaMap<EtsMethod *, impl::VMethod *> etsMethodsMap GUARDED_BY(methodsMapLock);
        PandaMap<impl::VMethod *, PandaUniquePtr<impl::VMethod>> methodsMap GUARDED_BY(methodsMapLock);
        PandaVector<MethodRange> methodRanges GUARDED_BY(methodsMapLock);
        os::memory::RWLock methodsMapLock;

        PandaMap<EtsField *, impl::VField *> etsFieldsMap GUARDED_BY(fieldsMapLock);
        PandaMap<impl::VField *, PandaUniquePtr<impl::VField>> fieldsMap GUARDED_BY(fieldsMapLock);
        PandaVector<FieldRange> fieldRanges GUARDED_BY(fieldsMapLock);
        os::memory::RWLock fieldsMapLock;

        PandaMap<VResolver *, PandaUniquePtr<VResolver>> resolversMap GUARDED_BY(resolverMapMutex);
        os::memory::Mutex resolverMapMutex;
    };

    void Report(const std::string_view message, bool isFatal = true);

    void SetVerifyOptions(bool isWorkaroundNoCrashIfInvalidUsage)
    {
        isWorkaroundNoCrashIfInvalidUsage_ = isWorkaroundNoCrashIfInvalidUsage;
    }

    void SetAbortHook(void (*hook)(void *data, const std::string_view message), void *data)
    {
        abortHook_ = hook;
        abortHookData_ = data;
    }

    void SetErrorHook(void (*hook)(void *data, const std::string_view message), void *data)
    {
        errorHook_ = hook;
        errorHookData_ = data;
    }

    VRef *AddGlobalVerifiedRef(ani_ref gref);
    void DeleteGlobalVerifiedRef(VRef *vgref);
    bool IsValidGlobalVerifiedRef(VRef *vgref);
    bool IsValidRawAniGlobalRef(void *ptr);
    bool IsValidRawAniWeakRef(void *ptr);

    VWRef *AddVerifiedWeakRef(ani_wref wref);
    void DeleteVerifiedWeakRef(VWRef *vwref);
    bool IsValidWeakRef(VWRef *vwref);

    impl::VMethod *AddMethod(EtsMethod *method);
    void AddMethodRanges(PandaVector<GlobalData::MethodRange> &&ranges);
    void DeleteMethod(impl::VMethod *vmethod);
    bool IsValidMethod(impl::VMethod *vmethod);
    bool IsValidRawEtsMethod(void *ptr);

    impl::VField *AddField(EtsField *field);
    void AddFieldRanges(PandaVector<GlobalData::FieldRange> &&ranges);
    void DeleteField(impl::VField *vfield);
    bool IsValidField(impl::VField *vfield);
    bool IsValidRawEtsField(void *ptr);

    bool IsValidStackRef(VRef *vref);

    VResolver *AddGlobalVerifiedResolver(ani_resolver resolver);
    void DeleteGlobalResolver(VResolver *vresolver);
    bool IsValidGlobalResolver(VResolver *vresolver);

    // NOTE: This method must be removed after resolution of #34764
    bool IsWorkaroundNoCrashIfInvalidUsage() const
    {
        return isWorkaroundNoCrashIfInvalidUsage_;
    }

private:
    void Abort(const std::string_view message);
    void Error(const std::string_view message);

    GlobalData &GetGlobalData()
    {
        return verifyObj_;
    }

    void (*abortHook_)(void *data, const std::string_view message) {};
    void *abortHookData_ {};

    void (*errorHook_)(void *data, const std::string_view message) {};
    void *errorHookData_ {};

    GlobalData verifyObj_;

    bool isWorkaroundNoCrashIfInvalidUsage_ {false};

    NO_COPY_SEMANTIC(ANIVerifier);
    NO_MOVE_SEMANTIC(ANIVerifier);
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_VERIFIER_H
