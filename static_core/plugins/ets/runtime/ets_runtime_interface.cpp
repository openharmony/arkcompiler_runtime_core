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

#include <string>

#include "ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

namespace panda::ets {
compiler::RuntimeInterface::ClassPtr EtsRuntimeInterface::GetClass(MethodPtr method, IdType id) const
{
    if (id == RuntimeInterface::MEM_PROMISE_CLASS_ID) {
        ScopedMutatorLock lock;
        auto *caller = MethodCast(method);
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
        return static_cast<EtsClassLinkerExtension *>(Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx))
            ->GetPromiseClass();
    }
    return PandaRuntimeInterface::GetClass(method, id);
}

compiler::RuntimeInterface::InteropCallKind EtsRuntimeInterface::GetInteropCallKind(MethodPtr method_ptr) const
{
    auto class_name = GetClassNameFromMethod(method_ptr);
    auto class_name_suffix = class_name.substr(class_name.find_last_of('.') + 1);
    if (class_name_suffix == "$jsnew") {
        return InteropCallKind::NEW_INSTANCE;
    }
    if (class_name_suffix != "$jscall") {
        return InteropCallKind::UNKNOWN;
    }

    auto method = MethodCast(method_ptr);
    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*method);
    auto linker_ctx = static_cast<EtsClassLinkerExtension *>(class_linker->GetExtension(ctx))->GetBootContext();

    ASSERT(method->GetArgType(0).IsReference());  // arg0 is always a reference
    ASSERT(method->GetArgType(1).IsReference());  // arg1 is always a reference
    uint32_t const arg_reftype_shift = method->GetReturnType().IsReference() ? 1 : 0;
    auto cls = class_linker->GetClass(*pf, pda.GetReferenceType(1 + arg_reftype_shift), linker_ctx);
    if (cls->IsStringClass()) {
        return InteropCallKind::CALL;
    }
    return InteropCallKind::CALL_BY_VALUE;
}

char *EtsRuntimeInterface::GetFuncPropName(MethodPtr method_ptr, uint32_t str_id) const
{
    auto method = MethodCast(method_ptr);
    auto pf = method->GetPandaFile();
    auto str = reinterpret_cast<const char *>(pf->GetStringData(panda::panda_file::File::EntityId(str_id)).data);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return const_cast<char *>(std::strrchr(str, '.') + 1);
}

uint64_t EtsRuntimeInterface::GetFuncPropNameOffset(MethodPtr method_ptr, uint32_t str_id) const
{
    auto pf = MethodCast(method_ptr)->GetPandaFile();
    auto str = GetFuncPropName(method_ptr, str_id);
    return reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(pf->GetBase());
}

}  // namespace panda::ets
