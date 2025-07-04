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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"

#include <optional>

namespace ark::ets::ani::verify {

class VEnv;
class VRef;
class ANIVerifier;

class EnvANIVerifier final {
public:
    EnvANIVerifier(ANIVerifier *verifier, const __ani_interaction_api *interactionAPI);
    ~EnvANIVerifier() = default;

    VEnv *GetEnv(ani_env *env);
    VEnv *AttachThread(ani_env *env);

    void PushNatveFrame();
    [[nodiscard]] std::optional<PandaString> PopNativeFrame();

    void CreateLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyLocalScope();
    void CreateEscapeLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyEscapeLocalScope(VRef *vref);

    template <class RefType>
    auto AddLocalVerifiedRef(RefType ref)
    {
        if constexpr (std::is_same_v<RefType, ani_ref>) {
            return static_cast<VRef *>(DoAddLocalVerifiedRef(ref, ANIRefType::REFERENCE));
        } else if constexpr (std::is_same_v<RefType, ani_object>) {
            return static_cast<VObject *>(DoAddLocalVerifiedRef(ref, ANIRefType::OBJECT));
        } else if constexpr (std::is_same_v<RefType, ani_class>) {
            return static_cast<VClass *>(DoAddLocalVerifiedRef(ref, ANIRefType::CLASS));
        } else if constexpr (std::is_same_v<RefType, ani_module>) {
            return static_cast<VModule *>(DoAddLocalVerifiedRef(ref, ANIRefType::MODULE));
        } else if constexpr (std::is_same_v<RefType, ani_string>) {
            return static_cast<VString *>(DoAddLocalVerifiedRef(ref, ANIRefType::STRING));
        } else if constexpr (std::is_same_v<RefType, ani_error>) {
            return static_cast<VError *>(DoAddLocalVerifiedRef(ref, ANIRefType::ERROR));
        } else {
            UNREACHABLE_CONSTEXPR();
        }
    }
    VRef *DoAddLocalVerifiedRef(ani_ref ref, ANIRefType type);
    void DeleteLocalVerifiedRef(VRef *vref);

    VRef *AddGloablVerifiedRef(ani_ref gref);
    void DeleteDeleteGlobalRef(VRef *vgref);
    bool IsValidGlobalVerifiedRef(VRef *vgref);

    bool IsValidInCurrentFrame(VRef *vref);
    bool CanBeDeletedFromCurrentScope(VRef *vref);

    const __ani_interaction_api *GetInteractionAPI() const
    {
        return interactionAPI_;
    }

private:
    enum class SubFrameType {
        NATIVE_FRAME,
        LOCAL_SCOPE,
        ESCAPE_LOCAL_SCOPE,
    };

    struct Frame {
        SubFrameType frameType;
        size_t nrUndef;
        PandaSet<VRef *> refs;
        PandaUniquePtr<ArenaAllocator> refsAllocatorHolder;
        ArenaAllocator *refsAllocator;
    };

    void DoPushNatveFrame();
    static bool IsValidVerifiedRef(const Frame &frame, VRef *vref);

    ANIVerifier *verifier_ {};
    const __ani_interaction_api *interactionAPI_ {};
    PandaVector<Frame> frames_;

    NO_COPY_SEMANTIC(EnvANIVerifier);
    NO_MOVE_SEMANTIC(EnvANIVerifier);
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H
