/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "ets_plugin.h"
#include "abs_int_inl_compat_checks.h"
#include "source_lang_enum.h"
#include "verifier_messages.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::verifier::plugin {

ManagedThread *EtsPlugin::CreateManagedThread() const
{
    os::memory::LockHolder l(mutex_);
    auto rt = Runtime::GetCurrent();
    auto vm = rt->GetPandaVM();
    auto coroman = static_cast<CoroutineManager *>(vm->GetThreadManager());
    return coroman->CreateEntrypointlessCoroutine(rt, vm, true, "_coro_", Coroutine::Type::MUTATOR,
                                                  CoroutinePriority::DEFAULT_PRIORITY);
}

void EtsPlugin::DestroyManagedThread(ManagedThread *thr) const
{
    os::memory::LockHolder l(mutex_);
    auto rt = Runtime::GetCurrent();
    auto vm = rt->GetPandaVM();
    auto coroman = static_cast<CoroutineManager *>(vm->GetThreadManager());
    coroman->DestroyEntrypointlessCoroutine(Coroutine::CastFromThread(thr));
}

void EtsPlugin::TypeSystemSetup(TypeSystem *types) const
{
    types->SetSupertypeOfArray(types->Object());
}

Type EtsPlugin::NormalizeType(Type type, TypeSystem *types) const
{
    auto integral32 = Type {Type::Builtin::INTEGRAL32};
    auto integral64 = Type {Type::Builtin::INTEGRAL64};
    auto float32 = Type {Type::Builtin::FLOAT32};
    auto float64 = Type {Type::Builtin::FLOAT64};
    Type result = IsSubtype(type, integral32, types)   ? integral32
                  : IsSubtype(type, integral64, types) ? integral64
                  : IsSubtype(type, float32, types)    ? float32
                  : IsSubtype(type, float64, types)    ? float64
                                                       : type;
    return result;
}

}  // namespace ark::verifier::plugin
