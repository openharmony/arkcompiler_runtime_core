/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H

#include <cstddef>
#include <memory>
#include <vector>

#include "libarkbase/macros.h"

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"
#else
// Keep this layout compatible with the JS N-API stack information structure.
// NOLINTNEXTLINE(readability-identifier-naming)
struct NapiStackInfo {
    size_t stackStart;
    size_t stackSize;
};
#endif

namespace ark::ets {
class EtsExecutionContext;
}  // namespace ark::ets

namespace ark::ets::interop::js {

class InteropCtx;

#if defined(PANDA_ETS_INTEROP_JS)
PANDA_PUBLIC_API void DeactivateInteropStackInfoForExecutionContext(EtsExecutionContext *executionCtx);
PANDA_PUBLIC_API void ActivateInteropStackInfoForExecutionContext(EtsExecutionContext *executionCtx);
#else
inline void DeactivateInteropStackInfoForExecutionContext([[maybe_unused]] EtsExecutionContext *executionCtx) {}
inline void ActivateInteropStackInfoForExecutionContext([[maybe_unused]] EtsExecutionContext *executionCtx) {}
#endif

// Allocated in an execution context only after it enters an ETS-to-JS scope.
struct InteropStackInfoState {
    std::vector<NapiStackInfo> savedStackInfos {};
};

class StackInfoManagerBase {
public:
    StackInfoManagerBase([[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] EtsExecutionContext *executionCtx)
        : ctx_(ctx), mainExecCtx_(executionCtx)
    {
        UNUSED_VAR(ctx_);
        UNUSED_VAR(mainExecCtx_);
    }
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void InitStackInfoIfNeeded() {};
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void UpdateStackInfoIfNeeded() {};
    PANDA_PUBLIC_API bool PushAndUpdateStackInfoIfNeeded([[maybe_unused]] EtsExecutionContext *executionCtx)
    {
        return true;
    }
    PANDA_PUBLIC_API bool RestoreStackInfoIfNeeded([[maybe_unused]] EtsExecutionContext *executionCtx)
    {
        return true;
    }
    PANDA_PUBLIC_API void DeactivateStackInfoIfNeeded([[maybe_unused]] EtsExecutionContext *executionCtx) {}
    PANDA_PUBLIC_API void ActivateStackInfoIfNeeded([[maybe_unused]] EtsExecutionContext *executionCtx) {}
    ~StackInfoManagerBase() = default;

    NO_MOVE_SEMANTIC(StackInfoManagerBase);
    NO_COPY_SEMANTIC(StackInfoManagerBase);

protected:
    InteropCtx *ctx_;                   // NOLINT(misc-non-private-member-variables-in-classes)
    EtsExecutionContext *mainExecCtx_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class StackInfoManagerOhos : public StackInfoManagerBase {
public:
    StackInfoManagerOhos(InteropCtx *ctx, EtsExecutionContext *executionCtx);
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void InitStackInfoIfNeeded();
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void UpdateStackInfoIfNeeded();
    PANDA_PUBLIC_API bool PushAndUpdateStackInfoIfNeeded(EtsExecutionContext *executionCtx);
    PANDA_PUBLIC_API bool RestoreStackInfoIfNeeded(EtsExecutionContext *executionCtx);
    PANDA_PUBLIC_API void DeactivateStackInfoIfNeeded(EtsExecutionContext *executionCtx);
    PANDA_PUBLIC_API void ActivateStackInfoIfNeeded(EtsExecutionContext *executionCtx);
    ~StackInfoManagerOhos();

    NO_MOVE_SEMANTIC(StackInfoManagerOhos);
    NO_COPY_SEMANTIC(StackInfoManagerOhos);

private:
    bool SetCurrentStackInfo(EtsExecutionContext *executionCtx);
    InteropStackInfoState *GetStackInfoState(EtsExecutionContext *executionCtx) const;
    InteropStackInfoState *GetOrCreateStackInfoState(EtsExecutionContext *executionCtx) const;
    bool SetStackInfo(NapiStackInfo &stackInfo) const;

    std::unique_ptr<NapiStackInfo> mainStackInfo_ {};
};

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
using StackInfoManager = StackInfoManagerOhos;
#else
using StackInfoManager = StackInfoManagerBase;
#endif

}  // namespace ark::ets::interop::js
#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H
