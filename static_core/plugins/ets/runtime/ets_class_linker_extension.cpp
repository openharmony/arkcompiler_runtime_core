/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_class_linker_extension.h"

#include "include/method.h"
#include "libpandabase/macros.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/napi/ets_napi_helpers.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/language_context.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/heap_manager.h"

namespace ark::ets {
namespace {
enum class EtsNapiType {
    GENERIC,  // - Switches the coroutine to native mode (GC is allowed)
              // - Prepends the argument list with two additional arguments (NAPI environment and this / class object)

    FAST,  // - Leaves the coroutine in managed mode (GC is not allowed)
           // - Prepends the argument list with two additional arguments (NAPI environment and this / class object)
           // - !!! The native function should not make any allocations (GC may be triggered during an allocation)

    CRITICAL  // - Leaves the coroutine in managed mode (GC is not allowed)
              // - Passes the arguments as is (the callee method should be static)
              // - !!! The native function should not make any allocations (GC may be triggered during an allocation)
};
}  // namespace

extern "C" void EtsAsyncEntryPoint();

static EtsNapiType GetEtsNapiType([[maybe_unused]] Method *method)
{
    // NOTE(#18101): support other NAPI types after annotations have been implemented.
    return EtsNapiType::GENERIC;
}

static std::string_view GetClassLinkerErrorDescriptor(ClassLinker::Error error)
{
    switch (error) {
        case ClassLinker::Error::CLASS_NOT_FOUND:
            return panda_file_items::class_descriptors::CLASS_NOT_FOUND_EXCEPTION;
        case ClassLinker::Error::FIELD_NOT_FOUND:
            return panda_file_items::class_descriptors::NO_SUCH_FIELD_ERROR;
        case ClassLinker::Error::METHOD_NOT_FOUND:
            return panda_file_items::class_descriptors::NO_SUCH_METHOD_ERROR;
        case ClassLinker::Error::NO_CLASS_DEF:
            return panda_file_items::class_descriptors::NO_CLASS_DEF_FOUND_ERROR;
        case ClassLinker::Error::CLASS_CIRCULARITY:
            return panda_file_items::class_descriptors::CLASS_CIRCULARITY_ERROR;
        case ClassLinker::Error::OVERRIDES_FINAL:
        case ClassLinker::Error::MULTIPLE_OVERRIDE:
        case ClassLinker::Error::MULTIPLE_IMPLEMENT:
            return panda_file_items::class_descriptors::LINKAGE_ERROR;
        default:
            LOG(FATAL, CLASS_LINKER) << "Unhandled class linker error (" << helpers::ToUnderlying(error) << "): ";
            UNREACHABLE();
    }
}

void EtsClassLinkerExtension::ErrorHandler::OnError(ClassLinker::Error error, const PandaString &message)
{
    ThrowEtsException(EtsCoroutine::GetCurrent(), GetClassLinkerErrorDescriptor(error), message);
}

void EtsClassLinkerExtension::InitializeClassRoots()
{
    InitializeArrayClassRoot(ClassRoot::ARRAY_CLASS, ClassRoot::CLASS,
                             utf::Mutf8AsCString(langCtx_.GetClassArrayClassDescriptor()));

    InitializePrimitiveClassRoot(ClassRoot::V, panda_file::Type::TypeId::VOID, "V");
    InitializePrimitiveClassRoot(ClassRoot::U1, panda_file::Type::TypeId::U1, "Z");
    InitializePrimitiveClassRoot(ClassRoot::I8, panda_file::Type::TypeId::I8, "B");
    InitializePrimitiveClassRoot(ClassRoot::U8, panda_file::Type::TypeId::U8, "H");
    InitializePrimitiveClassRoot(ClassRoot::I16, panda_file::Type::TypeId::I16, "S");
    InitializePrimitiveClassRoot(ClassRoot::U16, panda_file::Type::TypeId::U16, "C");
    InitializePrimitiveClassRoot(ClassRoot::I32, panda_file::Type::TypeId::I32, "I");
    InitializePrimitiveClassRoot(ClassRoot::U32, panda_file::Type::TypeId::U32, "U");
    InitializePrimitiveClassRoot(ClassRoot::I64, panda_file::Type::TypeId::I64, "J");
    InitializePrimitiveClassRoot(ClassRoot::U64, panda_file::Type::TypeId::U64, "Q");
    InitializePrimitiveClassRoot(ClassRoot::F32, panda_file::Type::TypeId::F32, "F");
    InitializePrimitiveClassRoot(ClassRoot::F64, panda_file::Type::TypeId::F64, "D");
    InitializePrimitiveClassRoot(ClassRoot::TAGGED, panda_file::Type::TypeId::TAGGED, "A");

    InitializeArrayClassRoot(ClassRoot::ARRAY_U1, ClassRoot::U1, "[Z");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I8, ClassRoot::I8, "[B");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U8, ClassRoot::U8, "[H");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I16, ClassRoot::I16, "[S");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U16, ClassRoot::U16, "[C");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I32, ClassRoot::I32, "[I");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U32, ClassRoot::U32, "[U");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I64, ClassRoot::I64, "[J");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U64, ClassRoot::U64, "[Q");
    InitializeArrayClassRoot(ClassRoot::ARRAY_F32, ClassRoot::F32, "[F");
    InitializeArrayClassRoot(ClassRoot::ARRAY_F64, ClassRoot::F64, "[D");
    InitializeArrayClassRoot(ClassRoot::ARRAY_TAGGED, ClassRoot::TAGGED, "[A");
    InitializeArrayClassRoot(ClassRoot::ARRAY_STRING, ClassRoot::STRING,
                             utf::Mutf8AsCString(langCtx_.GetStringArrayClassDescriptor()));
}

bool EtsClassLinkerExtension::InitializeImpl(bool compressedStringEnabled)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    auto *coroutine = ets::EtsCoroutine::GetCurrent();
    langCtx_ = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    heapManager_ = coroutine->GetVM()->GetHeapManager();

    auto *objectClass = GetClassLinker()->GetClass(langCtx_.GetObjectClassDescriptor(), false, GetBootContext());
    if (objectClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << langCtx_.GetObjectClassDescriptor() << "'";
        return false;
    }
    SetClassRoot(ClassRoot::OBJECT, objectClass);

    auto *classClass = GetClassLinker()->GetClass(langCtx_.GetClassClassDescriptor(), false, GetBootContext());
    if (classClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << langCtx_.GetClassClassDescriptor() << "'";
        return false;
    }
    SetClassRoot(ClassRoot::CLASS, classClass);

    EtsClass::FromRuntimeClass(classClass)->AsObject()->GetCoreType()->SetClass(classClass);
    EtsClass::FromRuntimeClass(objectClass)->AsObject()->GetCoreType()->SetClass(classClass);

    coretypes::String::SetCompressedStringsEnabled(compressedStringEnabled);

    auto *stringClass = GetClassLinker()->GetClass(langCtx_.GetStringClassDescriptor(), false, GetBootContext());
    if (stringClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << langCtx_.GetStringClassDescriptor() << "'";
        return false;
    }
    SetClassRoot(ClassRoot::STRING, stringClass);
    stringClass->SetStringClass();

    InitializeClassRoots();

    return true;
}

bool EtsClassLinkerExtension::InitializeArrayClass(Class *arrayClass, Class *componentClass)
{
    ASSERT(IsInitialized());

    ASSERT(!arrayClass->IsInitialized());
    ASSERT(arrayClass->GetComponentType() == nullptr);

    auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
    arrayClass->SetBase(objectClass);
    arrayClass->SetComponentType(componentClass);

    auto accessFlags = componentClass->GetAccessFlags() & ACC_FILE_MASK;
    accessFlags &= ~ACC_INTERFACE;
    accessFlags |= ACC_FINAL | ACC_ABSTRACT;

    arrayClass->SetAccessFlags(accessFlags);

    auto objectClassVtable = objectClass->GetVTable();
    auto arrayClassVtable = arrayClass->GetVTable();
    for (size_t i = 0; i < objectClassVtable.size(); i++) {
        arrayClassVtable[i] = objectClassVtable[i];
    }

    arrayClass->SetState(Class::State::INITIALIZED);

    ASSERT(arrayClass->IsArrayClass());  // After init, we give out a well-formed array class.
    return true;
}

bool EtsClassLinkerExtension::InitializeClass(Class *klass)
{
    ASSERT(IsInitialized());
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    constexpr uint32_t ETS_ACCESS_FLAGS_MASK = 0xFFFFU;

    EtsClass::FromRuntimeClass(klass)->Initialize(
        klass->GetBase() != nullptr ? EtsClass::FromRuntimeClass(klass->GetBase()) : nullptr,
        klass->GetAccessFlags() & ETS_ACCESS_FLAGS_MASK, klass->IsPrimitive());

    return true;
}

void EtsClassLinkerExtension::InitializePrimitiveClass(Class *primitiveClass)
{
    ASSERT(IsInitialized());

    ASSERT(!primitiveClass->IsInitialized());
    ASSERT(primitiveClass->IsPrimitive());

    primitiveClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    primitiveClass->SetState(Class::State::INITIALIZED);
}

size_t EtsClassLinkerExtension::GetClassVTableSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::TAGGED:
            return 0;
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassVTableSize();
        case ClassRoot::OBJECT:
        case ClassRoot::STRING:
            return GetClassRoot(root)->GetVTableSize();
        case ClassRoot::CLASS:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetClassIMTSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::TAGGED:
            return 0;
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassIMTSize();
        case ClassRoot::OBJECT:
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetClassSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::TAGGED:
            return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassSize();
        case ClassRoot::OBJECT:
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
            return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetArrayClassVTableSize()
{
    ASSERT(IsInitialized());

    return GetClassVTableSize(ClassRoot::OBJECT);
}

size_t EtsClassLinkerExtension::GetArrayClassIMTSize()
{
    ASSERT(IsInitialized());

    return GetClassIMTSize(ClassRoot::OBJECT);
}

size_t EtsClassLinkerExtension::GetArrayClassSize()
{
    ASSERT(IsInitialized());

    return GetClassSize(ClassRoot::OBJECT);
}

Class *EtsClassLinkerExtension::InitializeClass(ObjectHeader *objectHeader, const uint8_t *descriptor,
                                                size_t vtableSize, size_t imtSize, size_t size)
{
    auto managedClass = reinterpret_cast<EtsClass *>(objectHeader);
    managedClass->InitClass(descriptor, vtableSize, imtSize, size);
    auto klass = managedClass->GetRuntimeClass();
    klass->SetManagedObject(objectHeader);
    klass->SetSourceLang(GetLanguage());

    AddCreatedClass(klass);

    return klass;
}

Class *EtsClassLinkerExtension::CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size)
{
    ASSERT(IsInitialized());

    Class *classClassRoot = GetClassRoot(ClassRoot::CLASS);
    ObjectHeader *classObject;
    if (UNLIKELY(classClassRoot == nullptr)) {
        ASSERT(utf::IsEqual(descriptor, langCtx_.GetObjectClassDescriptor()) ||
               utf::IsEqual(descriptor, langCtx_.GetClassClassDescriptor()));
        classObject = heapManager_->AllocateNonMovableObject<true>(classClassRoot, EtsClass::GetSize(size));
    } else {
        classObject = heapManager_->AllocateNonMovableObject<false>(classClassRoot, EtsClass::GetSize(size));
    }
    if (UNLIKELY(classObject == nullptr)) {
        return nullptr;
    }

    return InitializeClass(classObject, descriptor, vtableSize, imtSize, size);
}

Class *EtsClassLinkerExtension::CreateClassRoot(const uint8_t *descriptor, ClassRoot root)
{
    auto vtableSize = GetClassVTableSize(root);
    auto imtSize = GetClassIMTSize(root);
    auto size = GetClassSize(root);

    Class *klass;
    if (root == ClassRoot::CLASS) {
        ASSERT(GetClassRoot(ClassRoot::CLASS) == nullptr);
        auto objectHeader = heapManager_->AllocateNonMovableObject<true>(nullptr, EtsClass::GetSize(size));
        ASSERT(objectHeader != nullptr);

        klass = InitializeClass(objectHeader, descriptor, vtableSize, imtSize, size);
        klass->SetObjectSize(EtsClass::GetSize(size));
        EtsClass::FromRuntimeClass(klass)->AsObject()->SetClass(EtsClass::FromRuntimeClass(klass));
    } else {
        klass = CreateClass(descriptor, vtableSize, imtSize, size);
    }

    ASSERT(klass != nullptr);
    klass->SetBase(GetClassRoot(ClassRoot::OBJECT));
    klass->SetState(Class::State::LOADED);
    klass->SetLoadContext(GetBootContext());
    GetClassLinker()->AddClassRoot(root, klass);
    return klass;
}

void EtsClassLinkerExtension::FreeClass(Class *klass)
{
    ASSERT(IsInitialized());

    RemoveCreatedClass(klass);
}

EtsClassLinkerExtension::~EtsClassLinkerExtension()
{
    if (!IsInitialized()) {
        return;
    }

    FreeLoadedClasses();
}

const void *EtsClassLinkerExtension::GetNativeEntryPointFor(Method *method) const
{
    panda_file::File::EntityId asyncAnnId = EtsAnnotation::FindAsyncAnnotation(method);
    if (asyncAnnId.IsValid()) {
        return reinterpret_cast<const void *>(EtsAsyncEntryPoint);
    }
    switch (GetEtsNapiType(method)) {
        case EtsNapiType::GENERIC: {
            return napi::GetEtsNapiEntryPoint();
        }
        case EtsNapiType::FAST: {
            auto flags = method->GetAccessFlags();
            flags |= ACC_FAST_NATIVE;
            method->SetAccessFlags(flags);

            return napi::GetEtsNapiEntryPoint();
        }
        case EtsNapiType::CRITICAL: {
            auto flags = method->GetAccessFlags();
            flags |= ACC_CRITICAL_NATIVE;
            method->SetAccessFlags(flags);

            return napi::GetEtsNapiCriticalEntryPoint();
        }
    }

    UNREACHABLE();
}

Class *EtsClassLinkerExtension::FromClassObject(ark::ObjectHeader *obj)
{
    return obj != nullptr ? reinterpret_cast<EtsClass *>(obj)->GetRuntimeClass() : nullptr;
}

size_t EtsClassLinkerExtension::GetClassObjectSizeFromClassSize(uint32_t size)
{
    return EtsClass::GetSize(size);
}

Class *EtsClassLinkerExtension::CacheClass(std::string_view descriptor, bool forceInit)
{
    Class *cls = GetClassLinker()->GetClass(utf::CStringAsMutf8(descriptor.data()), false, GetBootContext());
    if (cls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class " << descriptor;
        return nullptr;
    }
    if (forceInit && !GetClassLinker()->InitializeClass(EtsCoroutine::GetCurrent(), cls)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot initialize class " << descriptor;
        return nullptr;
    }
    return cls;
}

template <typename F>
Class *EtsClassLinkerExtension::CacheClass(std::string_view descriptor, F const &setup, bool forceInit)
{
    Class *cls = CacheClass(descriptor, forceInit);
    if (cls != nullptr) {
        setup(EtsClass::FromRuntimeClass(cls));
    }
    return cls;
}

void EtsClassLinkerExtension::InitializeBuiltinSpecialClasses()
{
    // CC-OFFNXT(WordsTool.95) false positive
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    CacheClass(STRING, [](auto *c) { c->SetValueTyped(); });
    undefinedClass_ = CacheClass(INTERNAL_UNDEFINED, [](auto *c) {
        c->SetUndefined();
        c->SetValueTyped();
    });
    auto const setupBoxedFlags = [](EtsClass *c) {
        c->SetBoxed();
        c->SetValueTyped();
    };
    boxBooleanClass_ = CacheClass(BOX_BOOLEAN, setupBoxedFlags);
    boxByteClass_ = CacheClass(BOX_BYTE, setupBoxedFlags);
    boxCharClass_ = CacheClass(BOX_CHAR, setupBoxedFlags);
    boxShortClass_ = CacheClass(BOX_SHORT, setupBoxedFlags);
    boxIntClass_ = CacheClass(BOX_INT, setupBoxedFlags);
    boxLongClass_ = CacheClass(BOX_LONG, setupBoxedFlags);
    boxFloatClass_ = CacheClass(BOX_FLOAT, setupBoxedFlags);
    boxDoubleClass_ = CacheClass(BOX_DOUBLE, setupBoxedFlags);
    bigintClass_ = CacheClass(BIG_INT, [](auto *c) {
        c->SetBigInt();
        c->SetValueTyped();
    });
    functionClass_ = CacheClass(FUNCTION, [](auto *c) { c->SetFunction(); });
}

void EtsClassLinkerExtension::InitializeBuiltinClasses()
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    InitializeBuiltinSpecialClasses();

    promiseClass_ = CacheClass(PROMISE);
    if (promiseClass_ != nullptr) {
        subscribeOnAnotherPromiseMethod_ = EtsMethod::ToRuntimeMethod(
            EtsClass::FromRuntimeClass(promiseClass_)->GetMethod("subscribeOnAnotherPromise"));
        ASSERT(subscribeOnAnotherPromiseMethod_ != nullptr);
    }
    promiseRefClass_ = CacheClass(PROMISE_REF);
    waiterListClass_ = CacheClass(WAITERS_LIST);
    mutexClass_ = CacheClass(MUTEX);
    eventClass_ = CacheClass(EVENT);
    condVarClass_ = CacheClass(COND_VAR);
    exceptionClass_ = CacheClass(EXCEPTION);
    errorClass_ = CacheClass(ERROR);
    arraybufClass_ = CacheClass(ARRAY_BUFFER);
    stringBuilderClass_ = CacheClass(STRING_BUILDER);
    arrayAsListIntClass_ = CacheClass(ARRAY_AS_LIST_INT);
    arrayClass_ = CacheClass(ARRAY);
    typeapiFieldClass_ = CacheClass(FIELD);
    typeapiMethodClass_ = CacheClass(METHOD);
    typeapiParameterClass_ = CacheClass(PARAMETER);
    sharedMemoryClass_ = CacheClass(SHARED_MEMORY);
    jsvalueClass_ = CacheClass(JS_VALUE);
    finalizableWeakClass_ = CacheClass(FINALIZABLE_WEAK_REF, [](auto *c) {
        c->SetFinalizeReference();
        c->SetWeakReference();
    });
    CacheClass(WEAK_REF, [](auto *c) { c->SetWeakReference(); });

    auto coro = EtsCoroutine::GetCurrent();
    coro->SetPromiseClass(promiseClass_);
    // NOTE (electronick, #15938): Refactor the managed class-related pseudo TLS fields
    // initialization in MT ManagedThread ctor and EtsCoroutine::Initialize
    coro->SetStringClassPtr(GetClassRoot(ClassRoot::STRING));
    coro->SetArrayU16ClassPtr(GetClassRoot(ClassRoot::ARRAY_U16));
    coro->SetArrayU8ClassPtr(GetClassRoot(ClassRoot::ARRAY_U8));
}

}  // namespace ark::ets
