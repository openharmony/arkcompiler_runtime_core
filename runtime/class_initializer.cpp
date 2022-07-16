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

#include "runtime/class_initializer.h"

#include "include/object_header.h"
#include "libpandafile/file_items.h"
#include "macros.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/monitor.h"
#include "runtime/monitor_object_lock.h"
#include "runtime/global_object_lock.h"
#include "verification/util/is_system.h"
#include "verify_app_install.h"

namespace panda {

template <MTModeT mode>
class ObjectLockConfig {
};

template <>
class ObjectLockConfig<MT_MODE_MULTI> {
public:
    using LockT = ObjectLock;
};

template <>
class ObjectLockConfig<MT_MODE_TASK> {
public:
    // TODO(xuliang): it is fast solution which we can reconsider later if we will face some perf issues with it.
    using LockT = GlobalObjectLock;
};

template <>
class ObjectLockConfig<MT_MODE_SINGLE> {
public:
    class DummyObjectLock {
    public:
        explicit DummyObjectLock(ObjectHeader *header [[maybe_unused]]) {}
        ~DummyObjectLock() = default;
        void Wait([[maybe_unused]] bool ignore_interruption = false) {}
        void TimedWait([[maybe_unused]] uint64_t timeout) {}
        void Notify() {}
        void NotifyAll() {}
        NO_COPY_SEMANTIC(DummyObjectLock);
        NO_MOVE_SEMANTIC(DummyObjectLock);
    };

    using LockT = DummyObjectLock;
};

static void WrapException(ClassLinker *class_linker, ManagedThread *thread)
{
    ASSERT(thread->HasPendingException());

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*thread->GetException()->ClassAddr<Class>());

    auto *error_class = class_linker->GetExtension(ctx)->GetClass(ctx.GetErrorClassDescriptor(), false);
    ASSERT(error_class != nullptr);

    auto *cause = thread->GetException();
    if (cause->IsInstanceOf(error_class)) {
        return;
    }

    ThrowException(ctx, thread, ctx.GetExceptionInInitializerErrorDescriptor(), nullptr);
}

static void ThrowNoClassDefFoundError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static void ThrowEarlierInitializationException(ManagedThread *thread, const Class *klass)
{
    ASSERT(klass->IsErroneous());

    ThrowNoClassDefFoundError(thread, klass);
}

static void ThrowIncompatibleClassChangeError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetIncompatibleClassChangeErrorDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static void ThrowVerifyError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static bool isBadSuperClass(const Class *base, ManagedThread *thread, const Class *klass)
{
    if (base->IsInterface()) {
        ThrowIncompatibleClassChangeError(thread, klass);
        return true;
    }

    if (base->IsFinal()) {
        ThrowVerifyError(thread, klass);
        return true;
    }

    return false;
}

template <class ObjectLockT>
static bool WaitInitialization(ObjectLockT *lock, ClassLinker *class_linker, ManagedThread *thread, Class *klass)
{
    while (true) {
        lock->Wait(true);

        if (thread->HasPendingException()) {
            WrapException(class_linker, thread);
            klass->SetState(Class::State::ERRONEOUS);
            return false;
        }

        if (klass->IsInitializing()) {
            continue;
        }

        if (klass->IsErroneous()) {
            ThrowNoClassDefFoundError(thread, klass);
            return false;
        }

        if (klass->IsInitialized()) {
            return true;
        }

        UNREACHABLE();
    }
}

/* static */
template <MTModeT mode>
bool ClassInitializer<mode>::Initialize(ClassLinker *class_linker, ManagedThread *thread, Class *klass)
{
    if (klass->IsInitialized()) {
        return true;
    }

    using ObjectLockT = typename ObjectLockConfig<mode>::LockT;

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> managed_class_obj_handle(thread, klass->GetManagedObject());
    {
        ObjectLockT lock(managed_class_obj_handle.GetPtr());

        if (klass->IsInitialized()) {
            return true;
        }

        if (klass->IsErroneous()) {
            ThrowEarlierInitializationException(thread, klass);
            return false;
        }

        if (!klass->IsVerified()) {
            if (!VerifyClass(klass)) {
                klass->SetState(Class::State::ERRONEOUS);
                panda::ThrowVerificationException(utf::Mutf8AsCString(klass->GetDescriptor()));
                return false;
            }
        }

        if (klass->IsInitializing()) {
            if (klass->GetInitTid() == thread->GetId()) {
                return true;
            }

            if (mode == MT_MODE_MULTI) {
                return WaitInitialization(&lock, class_linker, thread, klass);
            }

            UNREACHABLE();
        }

        klass->SetInitTid(thread->GetId());
        klass->SetState(Class::State::INITIALIZING);
        if (!ClassInitializer::InitializeFields(klass)) {
            LOG(ERROR, CLASS_LINKER) << "Cannot initialize fields of class '" << klass->GetName() << "'";
            return false;
        }
    }

    LOG(DEBUG, CLASS_LINKER) << "Initializing class " << klass->GetName();

    return InitializeClass(class_linker, thread, klass, managed_class_obj_handle);
}

/* static */
template <MTModeT mode>
bool ClassInitializer<mode>::InitializeClass(ClassLinker *class_linker, ManagedThread *thread, Class *klass,
                                             const VMHandle<ObjectHeader> &managed_class_obj_handle)
{
    using ObjectLockT = typename ObjectLockConfig<mode>::LockT;

    if (!klass->IsInterface()) {
        auto *base = klass->GetBase();

        if (base != nullptr) {
            if (isBadSuperClass(base, thread, klass)) {
                return false;
            }

            if (!Initialize(class_linker, thread, base)) {
                ObjectLockT lock(managed_class_obj_handle.GetPtr());
                klass->SetState(Class::State::ERRONEOUS);
                lock.NotifyAll();
                return false;
            }
        }

        for (auto *iface : klass->GetInterfaces()) {
            if (iface->IsInitialized()) {
                continue;
            }

            if (!InitializeInterface(class_linker, thread, iface, klass)) {
                ObjectLockT lock(managed_class_obj_handle.GetPtr());
                klass->SetState(Class::State::ERRONEOUS);
                lock.NotifyAll();
                return false;
            }
        }
    }

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID)},
                        Method::Proto::RefTypeVector {});
    auto *cctor_name = ctx.GetCctorName();
    auto *cctor = klass->GetDirectMethod(cctor_name, proto);
    if (cctor != nullptr) {
        cctor->InvokeVoid(thread, nullptr);
    }

    {
        ObjectLockT lock(managed_class_obj_handle.GetPtr());

        if (thread->HasPendingException()) {
            WrapException(class_linker, thread);
            klass->SetState(Class::State::ERRONEOUS);
            lock.NotifyAll();
            return false;
        }

        klass->SetState(Class::State::INITIALIZED);

        lock.NotifyAll();
    }

    return true;
}

/* static */
template <MTModeT mode>
bool ClassInitializer<mode>::InitializeInterface(ClassLinker *class_linker, ManagedThread *thread, Class *iface,
                                                 Class *klass)
{
    if (!iface->IsInterface()) {
        ThrowIncompatibleClassChangeError(thread, klass);
        return false;
    }

    for (auto *base_iface : iface->GetInterfaces()) {
        if (base_iface->IsInitialized()) {
            continue;
        }

        if (!InitializeInterface(class_linker, thread, base_iface, klass)) {
            return false;
        }
    }

    if (!iface->HasDefaultMethods()) {
        return true;
    }

    return Initialize(class_linker, thread, iface);
}

/* static */
template <MTModeT mode>
bool ClassInitializer<mode>::VerifyClass(Class *klass)
{
    ASSERT(!klass->IsVerified());

    auto &verif_opts = Runtime::GetCurrent()->GetVerificationOptions();

    if (!IsVerifySuccInAppInstall(klass->GetPandaFile())) {
        LOG(ERROR, CLASS_LINKER) << "verify fail";
        return false;
    }
    if (!verif_opts.IsEnabled()) {
        klass->SetState(Class::State::VERIFIED);
        return true;
    }

    bool skip_verification = !verif_opts.VerifyRuntimeLibraries && verifier::IsSystemOrSyntheticClass(*klass);
    if (skip_verification) {
        for (auto &method : klass->GetMethods()) {
            method.SetVerified(true);
        }
        klass->SetState(Class::State::VERIFIED);
        return true;
    }

    LOG(INFO, VERIFIER) << "Verification of class '" << klass->GetName() << "'";
    for (auto &method : klass->GetMethods()) {
        method.EnqueueForVerification();
    }

    // sync point
    if (!verif_opts.SyncOnClassInitialization) {
        klass->SetState(Class::State::VERIFIED);
        return true;
    }

    for (auto &method : klass->GetMethods()) {
        if (!method.Verify()) {
            return false;
        }
    }

    klass->SetState(Class::State::VERIFIED);
    return true;
}

template <class T>
static void InitializePrimitiveField(Class *klass, const Field &field)
{
    panda_file::FieldDataAccessor fda(*field.GetPandaFile(), field.GetFileId());
    auto value = fda.GetValue<T>();
    klass->SetFieldPrimitive<T>(field, value ? value.value() : 0);
}

static void InitializeTaggedField(Class *klass, const Field &field)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    klass->SetFieldPrimitive<coretypes::TaggedValue>(field, ctx.GetInitialTaggedValue());
}

static void InitializeStringField(Class *klass, const Field &field)
{
    panda_file::FieldDataAccessor fda(*field.GetPandaFile(), field.GetFileId());
    auto value = fda.GetValue<uint32_t>();
    if (value) {
        panda_file::File::EntityId id(value.value());
        coretypes::String *str = Runtime::GetCurrent()->GetPandaVM()->ResolveString(*klass->GetPandaFile(), id);
        if (LIKELY(str != nullptr)) {
            klass->SetFieldObject(field, str);
            return;
        }
    }
    // Should nullptr be set?
    klass->SetFieldObject(field, nullptr);
}

/* static */
template <MTModeT mode>
bool ClassInitializer<mode>::InitializeFields(Class *klass)
{
    using Type = panda_file::Type;

    for (const auto &field : klass->GetStaticFields()) {
        switch (field.GetTypeId()) {
            case Type::TypeId::U1:
            case Type::TypeId::U8:
                InitializePrimitiveField<uint8_t>(klass, field);
                break;
            case Type::TypeId::I8:
                InitializePrimitiveField<int8_t>(klass, field);
                break;
            case Type::TypeId::I16:
                InitializePrimitiveField<int16_t>(klass, field);
                break;
            case Type::TypeId::U16:
                InitializePrimitiveField<uint16_t>(klass, field);
                break;
            case Type::TypeId::I32:
                InitializePrimitiveField<int32_t>(klass, field);
                break;
            case Type::TypeId::U32:
                InitializePrimitiveField<uint32_t>(klass, field);
                break;
            case Type::TypeId::I64:
                InitializePrimitiveField<int64_t>(klass, field);
                break;
            case Type::TypeId::U64:
                InitializePrimitiveField<uint64_t>(klass, field);
                break;
            case Type::TypeId::F32:
                InitializePrimitiveField<float>(klass, field);
                break;
            case Type::TypeId::F64:
                InitializePrimitiveField<double>(klass, field);
                break;
            case Type::TypeId::TAGGED:
                InitializeTaggedField(klass, field);
                break;
            case Type::TypeId::REFERENCE:
                InitializeStringField(klass, field);
                break;
            default: {
                UNREACHABLE();
                break;
            }
        }
    }

    return true;
}

template class ClassInitializer<MT_MODE_SINGLE>;
template class ClassInitializer<MT_MODE_MULTI>;
template class ClassInitializer<MT_MODE_TASK>;

}  // namespace panda
