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

#include "plugins/ets/runtime/interop_js/code_scopes-inl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/native_api/arkts_interop_js_api_impl.h"

namespace ark::ets::interop::js {

ScopedInteropCallStackRecord::ScopedInteropCallStackRecord(EtsExecutionContext *executionCtx, char const *descr)
    : executionCtx_(executionCtx)
{
    [[maybe_unused]] auto status = OpenInteropCodeScope<false>(executionCtx_, descr);
    ASSERT(status);
}

ScopedInteropCallStackRecord::~ScopedInteropCallStackRecord()
{
    [[maybe_unused]] auto status = CloseInteropCodeScope<false>(executionCtx_);
    ASSERT(status);
}

InteropETSToJSCodeScope::InteropETSToJSCodeScope(EtsExecutionContext *executionCtx, char const *descr)
    : executionCtx_(executionCtx)
{
    [[maybe_unused]] auto status = OpenETSToJSScope(executionCtx_, descr);
    ASSERT(status);
}

InteropETSToJSCodeScope::~InteropETSToJSCodeScope()
{
    [[maybe_unused]] auto status = CloseETSToJSScope(executionCtx_);
    ASSERT(status);
}

InteropJSToETSCodeScope::InteropJSToETSCodeScope(EtsExecutionContext *executionCtx, char const *descr)
    : executionCtx_(executionCtx)
{
    [[maybe_unused]] auto status = OpenJSToETSScope(executionCtx_, descr);
    ASSERT(status);
}

InteropJSToETSCodeScope::~InteropJSToETSCodeScope()
{
    [[maybe_unused]] auto status = CloseJSToETSScope(executionCtx_);
    ASSERT(status);
}

}  // namespace ark::ets::interop::js
