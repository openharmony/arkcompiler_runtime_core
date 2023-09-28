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

#include "plugins/ets/runtime/ets_itable_builder.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"

#include <array>

namespace panda::ets {

void EtsLanguageContext::ThrowException(ManagedThread *thread, const uint8_t *mutf8_name,
                                        const uint8_t *mutf8_msg) const
{
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(thread);
    ThrowEtsException(coroutine, utf::Mutf8AsCString(mutf8_name), utf::Mutf8AsCString(mutf8_msg));
}

PandaUniquePtr<ITableBuilder> EtsLanguageContext::CreateITableBuilder() const
{
    return MakePandaUnique<EtsITableBuilder>();
}

PandaUniquePtr<VTableBuilder> EtsLanguageContext::CreateVTableBuilder() const
{
    return MakePandaUnique<EtsVTableBuilder>();
}

ets::PandaEtsVM *EtsLanguageContext::CreateVM(Runtime *runtime, const RuntimeOptions &options) const
{
    ASSERT(runtime != nullptr);

    auto vm = ets::PandaEtsVM::Create(runtime, options);
    if (!vm) {
        LOG(ERROR, RUNTIME) << vm.Error();
        return nullptr;
    }

    return vm.Value();
}

mem::GC *EtsLanguageContext::CreateGC(mem::GCType gc_type, mem::ObjectAllocatorBase *object_allocator,
                                      const mem::GCSettings &settings) const
{
    return mem::CreateGC<EtsLanguageConfig>(gc_type, object_allocator, settings);
}

void EtsLanguageContext::ThrowStackOverflowException(ManagedThread *thread) const
{
    ASSERT(thread != nullptr);
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(thread);
    EtsClassLinker *class_linker = coroutine->GetPandaVM()->GetClassLinker();
    const char *class_descriptor = utf::Mutf8AsCString(GetStackOverflowErrorClassDescriptor());
    EtsClass *cls = class_linker->GetClass(class_descriptor, true);
    ASSERT(cls != nullptr);

    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> exc(coroutine, EtsObject::Create(cls));
    coroutine->SetException(exc.GetPtr()->GetCoreType());
}

VerificationInitAPI EtsLanguageContext::GetVerificationInitAPI() const
{
    VerificationInitAPI v_api;
    v_api.primitive_roots_for_verification = {
        panda_file::Type::TypeId::TAGGED, panda_file::Type::TypeId::VOID, panda_file::Type::TypeId::U1,
        panda_file::Type::TypeId::U8,     panda_file::Type::TypeId::U16,  panda_file::Type::TypeId::U32,
        panda_file::Type::TypeId::U64,    panda_file::Type::TypeId::I8,   panda_file::Type::TypeId::I16,
        panda_file::Type::TypeId::I32,    panda_file::Type::TypeId::I64,  panda_file::Type::TypeId::F32,
        panda_file::Type::TypeId::F64};

    v_api.array_elements_for_verification = {reinterpret_cast<const uint8_t *>("[Z"),
                                             reinterpret_cast<const uint8_t *>("[B"),
                                             reinterpret_cast<const uint8_t *>("[S"),
                                             reinterpret_cast<const uint8_t *>("[C"),
                                             reinterpret_cast<const uint8_t *>("[I"),
                                             reinterpret_cast<const uint8_t *>("[J"),
                                             reinterpret_cast<const uint8_t *>("[F"),
                                             reinterpret_cast<const uint8_t *>("[D")

    };

    v_api.is_need_class_synthetic_class = true;
    v_api.is_need_object_synthetic_class = false;
    v_api.is_need_string_synthetic_class = false;

    return v_api;
}

}  // namespace panda::ets
