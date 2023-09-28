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
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/language_context.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/heap_manager.h"

namespace panda::ets {
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
    // TODO(a.urakov): support other NAPI types
#ifdef USE_ETS_NAPI_CRITICAL_BY_DEFAULT
    return EtsNapiType::CRITICAL;
#else
    return EtsNapiType::GENERIC;
#endif
}

void EtsClassLinkerExtension::ErrorHandler::OnError(ClassLinker::Error error, const PandaString &message)
{
    std::string_view class_descriptor {};
    switch (error) {
        case ClassLinker::Error::CLASS_NOT_FOUND: {
            class_descriptor = panda_file_items::class_descriptors::CLASS_NOT_FOUND_EXCEPTION;
            break;
        }
        case ClassLinker::Error::FIELD_NOT_FOUND: {
            class_descriptor = panda_file_items::class_descriptors::NO_SUCH_FIELD_ERROR;
            break;
        }
        case ClassLinker::Error::METHOD_NOT_FOUND: {
            class_descriptor = panda_file_items::class_descriptors::NO_SUCH_METHOD_ERROR;
            break;
        }
        case ClassLinker::Error::NO_CLASS_DEF: {
            class_descriptor = panda_file_items::class_descriptors::NO_CLASS_DEF_FOUND_ERROR;
            break;
        }
        case ClassLinker::Error::CLASS_CIRCULARITY: {
            class_descriptor = panda_file_items::class_descriptors::CLASS_CIRCULARITY_ERROR;
            break;
        }
        default: {
            LOG(FATAL, CLASS_LINKER) << "Unhandled error (" << static_cast<size_t>(error) << "): " << message;
            break;
        }
    }

    ThrowEtsException(EtsCoroutine::GetCurrent(), class_descriptor, message);
}

bool EtsClassLinkerExtension::CacheClass(Class **class_for_cache, const char *descriptor)
{
    *class_for_cache =
        GetClassLinker()->GetClass(reinterpret_cast<const uint8_t *>(descriptor), false, GetBootContext());
    if (*class_for_cache == nullptr || !InitializeClass(*class_for_cache)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class " << descriptor;
        return false;
    }
    return true;
}

bool EtsClassLinkerExtension::InitializeImpl(bool compressed_string_enabled)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    lang_ctx_ = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    heap_manager_ = EtsCoroutine::GetCurrent()->GetVM()->GetHeapManager();

    // NB! By convention, class_class should be allocated first, so that all
    // other class objects receive a pointer to it in their klass words.
    // At the same time, std.core.Class is derived from std.core.Object, so we
    // allocate object_class immediately after std.core.Class and manually adjust
    // inheritance chain. After this, initialization order is not fixed.
    auto class_class = CreateClassRoot(lang_ctx_.GetClassClassDescriptor(), ClassRoot::CLASS);

    // EtsClass has three reference fields, if they are not traversed in gc, then
    // they can be either deallocated or moved
    // also this layout is hardcoded here because it was forbidden to add this class into ets code (stdlib)
    constexpr size_t CLASS_CLASS_REF_FIELDS_NUM = 3;
    class_class->SetRefFieldsOffset(EtsClass::GetIfTableOffset(), false);
    class_class->SetRefFieldsNum(CLASS_CLASS_REF_FIELDS_NUM, false);
    class_class->SetVolatileRefFieldsNum(0, false);

    auto *object_class = GetClassLinker()->GetClass(lang_ctx_.GetObjectClassDescriptor(), false, GetBootContext());
    if (object_class == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << lang_ctx_.GetObjectClassDescriptor() << "'";
        return false;
    }

    SetClassRoot(ClassRoot::OBJECT, object_class);

    if (!CacheClass(&object_class_, OBJECT.data())) {
        return false;
    }

    ASSERT(class_class->GetBase() == nullptr);
    class_class->SetBase(object_class);

    coretypes::String::SetCompressedStringsEnabled(compressed_string_enabled);

    auto *string_class = GetClassLinker()->GetClass(lang_ctx_.GetStringClassDescriptor(), false, GetBootContext());
    if (string_class == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << lang_ctx_.GetStringClassDescriptor() << "'";
        return false;
    }

    SetClassRoot(ClassRoot::STRING, string_class);
    string_class->SetStringClass();

    InitializeArrayClassRoot(ClassRoot::ARRAY_CLASS, ClassRoot::CLASS,
                             utf::Mutf8AsCString(lang_ctx_.GetClassArrayClassDescriptor()));

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
                             utf::Mutf8AsCString(lang_ctx_.GetStringArrayClassDescriptor()));

    if (!CacheClass(&box_boolean_class_, BOX_BOOLEAN.data())) {
        return false;
    }
    if (!CacheClass(&box_byte_class_, BOX_BYTE.data())) {
        return false;
    }
    if (!CacheClass(&box_char_class_, BOX_CHAR.data())) {
        return false;
    }
    if (!CacheClass(&box_short_class_, BOX_SHORT.data())) {
        return false;
    }
    if (!CacheClass(&box_int_class_, BOX_INT.data())) {
        return false;
    }
    if (!CacheClass(&box_long_class_, BOX_LONG.data())) {
        return false;
    }
    if (!CacheClass(&box_float_class_, BOX_FLOAT.data())) {
        return false;
    }
    if (!CacheClass(&box_double_class_, BOX_DOUBLE.data())) {
        return false;
    }
    if (!CacheClass(&promise_class_, PROMISE.data())) {
        return false;
    }
    if (!CacheClass(&arraybuf_class_, ARRAY_BUFFER.data())) {
        return false;
    }
    if (!CacheClass(&shared_memory_class_, SHARED_MEMORY.data())) {
        return false;
    }

    if (!CacheClass(&typeapi_field_class_, FIELD.data())) {
        return false;
    }
    if (!CacheClass(&typeapi_method_class_, METHOD.data())) {
        return false;
    }
    if (!CacheClass(&typeapi_parameter_class_, PARAMETER.data())) {
        return false;
    }

    ets::EtsCoroutine::GetCurrent()->SetPromiseClass(promise_class_);
    Class *weak_ref_class;
    // Cache into local variable, no need to cache to this class
    if (!CacheClass(&weak_ref_class, WEAK_REF.data())) {
        return false;
    }
    // Mark std.core.WeakRef class as weak reference (flag)
    auto *managed_weak_ref_ets_class = reinterpret_cast<EtsClass *>(weak_ref_class->GetManagedObject());
    managed_weak_ref_ets_class->SetWeakReference();

    return true;
}

bool EtsClassLinkerExtension::InitializeArrayClass(Class *array_class, Class *component_class)
{
    ASSERT(IsInitialized());

    ASSERT(!array_class->IsInitialized());
    ASSERT(array_class->GetComponentType() == nullptr);

    auto *object_class = GetClassRoot(ClassRoot::OBJECT);
    array_class->SetBase(object_class);
    array_class->SetComponentType(component_class);

    auto access_flags = component_class->GetAccessFlags() & ACC_FILE_MASK;
    access_flags &= ~ACC_INTERFACE;
    access_flags |= ACC_FINAL | ACC_ABSTRACT;

    array_class->SetAccessFlags(access_flags);

    auto object_class_vtable = object_class->GetVTable();
    auto array_class_vtable = array_class->GetVTable();
    for (size_t i = 0; i < object_class_vtable.size(); i++) {
        array_class_vtable[i] = object_class_vtable[i];
    }

    array_class->SetState(Class::State::INITIALIZED);

    ASSERT(array_class->IsArrayClass());  // After init, we give out a well-formed array class.
    return true;
}

bool EtsClassLinkerExtension::InitializeClass([[maybe_unused]] Class *klass)
{
    return true;
}

void EtsClassLinkerExtension::InitializePrimitiveClass(Class *primitive_class)
{
    ASSERT(IsInitialized());

    ASSERT(!primitive_class->IsInitialized());
    ASSERT(primitive_class->IsPrimitive());

    primitive_class->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    primitive_class->SetState(Class::State::INITIALIZED);
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

Class *EtsClassLinkerExtension::InitializeClass(ObjectHeader *object_header, const uint8_t *descriptor,
                                                size_t vtable_size, size_t imt_size, size_t size)
{
    auto managed_class = reinterpret_cast<EtsClass *>(object_header);
    managed_class->InitClass(descriptor, vtable_size, imt_size, size);
    auto klass = managed_class->GetRuntimeClass();
    klass->SetManagedObject(object_header);
    klass->SetSourceLang(GetLanguage());

    AddCreatedClass(klass);

    return klass;
}

Class *EtsClassLinkerExtension::CreateClass(const uint8_t *descriptor, size_t vtable_size, size_t imt_size, size_t size)
{
    ASSERT(IsInitialized());

    auto class_root = GetClassRoot(ClassRoot::CLASS);
    ASSERT(class_root != nullptr);

    auto object_header = heap_manager_->AllocateNonMovableObject<false>(class_root, EtsClass::GetSize(size));
    if (UNLIKELY(object_header == nullptr)) {
        return nullptr;
    }

    return InitializeClass(object_header, descriptor, vtable_size, imt_size, size);
}

Class *EtsClassLinkerExtension::CreateClassRoot(const uint8_t *descriptor, ClassRoot root)
{
    auto vtable_size = GetClassVTableSize(root);
    auto imt_size = GetClassIMTSize(root);
    auto size = GetClassSize(root);

    Class *klass;
    if (root == ClassRoot::CLASS) {
        ASSERT(GetClassRoot(ClassRoot::CLASS) == nullptr);
        auto object_header = heap_manager_->AllocateNonMovableObject<true>(nullptr, EtsClass::GetSize(size));
        ASSERT(object_header != nullptr);

        klass = InitializeClass(object_header, descriptor, vtable_size, imt_size, size);
        EtsClass::FromRuntimeClass(klass)->AsObject()->SetClass(EtsClass::FromRuntimeClass(klass));
    } else {
        klass = CreateClass(descriptor, vtable_size, imt_size, size);
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
    panda_file::File::EntityId async_ann_id = EtsAnnotation::FindAsyncAnnotation(method);
    if (async_ann_id.IsValid()) {
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

Class *EtsClassLinkerExtension::FromClassObject(panda::ObjectHeader *obj)
{
    return obj != nullptr ? reinterpret_cast<EtsClass *>(obj)->GetRuntimeClass() : nullptr;
}

size_t EtsClassLinkerExtension::GetClassObjectSizeFromClassSize(uint32_t size)
{
    return EtsClass::GetSize(size);
}

}  // namespace panda::ets
