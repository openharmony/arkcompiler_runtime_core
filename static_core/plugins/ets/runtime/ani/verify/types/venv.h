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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VENV_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VENV_H

#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vmethod.h"
#include "plugins/ets/runtime/ani/verify/types/vfield.h"
#include "plugins/ets/runtime/ani/verify/types/vresolver.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_interaction_api.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark::ets::ani::verify {

class EnvANIVerifier;

class VEnv final : public ani_env {
public:
    static PandaUniquePtr<VEnv> Create();
    ani_env *GetEnv();
    static VEnv *GetCurrent();

    // Scope
    void CreateLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyLocalScope();
    void CreateEscapeLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyEscapeLocalScope(VRef *vref);

    // Local refs
    template <class RefType>
    auto AddLocalVerifiedRef(RefType ref);

    void DeleteLocalVerifiedRef(VRef *vref);

    VMethod *GetVerifiedMethod(ani_method method);

    VStaticMethod *GetVerifiedStaticMethod(ani_static_method staticMethod);

    VFunction *GetVerifiedFunction(ani_function function);

    VField *GetVerifiedField(ani_field field);

    VStaticField *GetVerifiedStaticField(ani_static_field staticField);

    // Global refs
    VRef *AddGlobalVerifiedRef(ani_ref gref);
    void DeleteGlobalVerifiedRef(VRef *vgref);
    bool IsValidGlobalVerifiedRef(VRef *vgref);

    // Global resolvers
    VResolver *AddGlobalVerifiedResolver(ani_resolver resolver);
    void DeleteGlobalVerifiedResolver(VResolver *vresolver);
    bool IsValidGlobalVerifiedResolver(VResolver *vresolver);

private:
    VEnv() : ani_env()
    {
        c_api = ani::verify::GetVerifyInteractionAPI();
    }
    ~VEnv() = default;
    EnvANIVerifier *GetEnvANIVerifier();
    NO_COPY_SEMANTIC(VEnv);
    NO_MOVE_SEMANTIC(VEnv);
    friend class ark::mem::Allocator;
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VENV_H
