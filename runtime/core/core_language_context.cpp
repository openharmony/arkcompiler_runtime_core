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

#include "runtime/core/core_language_context.h"

#include "runtime/core/core_itable_builder.h"
#include "runtime/core/core_vtable_builder.h"
#include "runtime/include/vtable_builder-inl.h"
#include "runtime/handle_scope-inl.h"

namespace panda {

static Class *GetExceptionClass(const uint8_t *mutf8_name, ManagedThread *thread, ClassLinker *class_linker)
{
    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = class_linker->GetExtension(ctx);
    auto *cls = class_linker->GetClass(mutf8_name, true, extension->GetBootContext());
    if (cls == nullptr) {
        LOG(ERROR, CORE) << "Class " << utf::Mutf8AsCString(mutf8_name) << " not found";
        return nullptr;
    }

    if (!class_linker->InitializeClass(thread, cls)) {
        LOG(ERROR, CORE) << "Class " << utf::Mutf8AsCString(mutf8_name) << " cannot be initialized";
        return nullptr;
    }
    return cls;
}

void CoreLanguageContext::ThrowException(ManagedThread *thread, const uint8_t *mutf8_name,
                                         const uint8_t *mutf8_msg) const
{
    ASSERT(thread == ManagedThread::GetCurrent());

    if (thread->IsUsePreAllocObj()) {
        thread->SetUsePreAllocObj(false);
        auto *oom = thread->GetVM()->GetOOMErrorObject();
        thread->SetException(oom);
        return;
    }

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> cause(thread, thread->GetException());
    thread->ClearException();

    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *class_linker = runtime->GetClassLinker();
    auto *cls = GetExceptionClass(mutf8_name, thread, class_linker);
    if (cls == nullptr) {
        return;
    }

    VMHandle<ObjectHeader> exc_handle(thread, ObjectHeader::Create(cls));

    coretypes::String *msg;
    if (mutf8_msg != nullptr) {
        msg = coretypes::String::CreateFromMUtf8(mutf8_msg, ctx, Runtime::GetCurrent()->GetPandaVM());
        if (UNLIKELY(msg == nullptr)) {
            // OOM happened during msg allocation
            ASSERT(thread->HasPendingException());
            return;
        }
    } else {
        msg = nullptr;
    }
    VMHandle<ObjectHeader> msg_handle(thread, msg);

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {utf::Mutf8AsCString(ctx.GetStringClassDescriptor()),
                                                      utf::Mutf8AsCString(ctx.GetObjectClassDescriptor())});
    auto *ctor_name = ctx.GetCtorName();
    auto *ctor = cls->GetDirectMethod(ctor_name, proto);
    if (ctor == nullptr) {
        LOG(ERROR, CORE) << "No method " << utf::Mutf8AsCString(ctor_name) << " in class "
                         << utf::Mutf8AsCString(mutf8_name);
        return;
    }

    constexpr size_t NARGS = 3;
    std::array<Value, NARGS> args {Value(exc_handle.GetPtr()), Value(msg_handle.GetPtr()), Value(cause.GetPtr())};
    ctor->InvokeVoid(thread, args.data());
    if (LIKELY(!thread->HasPendingException())) {
        thread->SetException(exc_handle.GetPtr());
    }
}

PandaUniquePtr<ITableBuilder> CoreLanguageContext::CreateITableBuilder() const
{
    return MakePandaUnique<CoreITableBuilder>();
}

PandaUniquePtr<VTableBuilder> CoreLanguageContext::CreateVTableBuilder() const
{
    return MakePandaUnique<CoreVTableBuilder>();
}

PandaVM *CoreLanguageContext::CreateVM(Runtime *runtime, const RuntimeOptions &options) const
{
    auto core_vm = core::PandaCoreVM::Create(runtime, options);
    if (!core_vm) {
        LOG(ERROR, CORE) << core_vm.Error();
        return nullptr;
    }
    return core_vm.Value();
}

mem::GC *CoreLanguageContext::CreateGC(mem::GCType gc_type, mem::ObjectAllocatorBase *object_allocator,
                                       const mem::GCSettings &settings) const
{
    return mem::CreateGC<PandaAssemblyLanguageConfig>(gc_type, object_allocator, settings);
}

void CoreLanguageContext::ThrowStackOverflowException(ManagedThread *thread) const
{
    auto runtime = Runtime::GetCurrent();
    auto *class_linker = runtime->GetClassLinker();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = class_linker->GetExtension(ctx);
    auto *cls = class_linker->GetClass(ctx.GetStackOverflowErrorClassDescriptor(), true, extension->GetBootContext());

    HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> exc(thread, ObjectHeader::Create(cls));
    thread->SetException(exc.GetPtr());
}

VerificationInitAPI CoreLanguageContext::GetVerificationInitAPI() const
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
    v_api.is_need_object_synthetic_class = true;
    v_api.is_need_string_synthetic_class = true;

    return v_api;
}

}  // namespace panda
