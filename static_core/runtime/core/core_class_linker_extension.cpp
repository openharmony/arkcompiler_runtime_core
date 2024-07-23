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

#include "runtime/core/core_class_linker_extension.h"

#include "runtime/include/coretypes/class.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/panda_vm.h"

namespace ark {

using SourceLang = panda_file::SourceLang;
using Type = panda_file::Type;

void CoreClassLinkerExtension::ErrorHandler::OnError(ClassLinker::Error error, const PandaString &message)
{
    auto *thread = ManagedThread::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);

    switch (error) {
        case ClassLinker::Error::CLASS_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetClassNotFoundExceptionDescriptor(),
                           utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::FIELD_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetNoSuchFieldErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::METHOD_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetNoSuchMethodErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::NO_CLASS_DEF: {
            ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::CLASS_CIRCULARITY: {
            ThrowException(ctx, thread, ctx.GetClassCircularityErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        default:
            LOG(FATAL, CLASS_LINKER) << "Unhandled error (" << static_cast<size_t>(error) << "): " << message;
            break;
    }
}

void CoreClassLinkerExtension::InitializeClassRoots(const LanguageContext &ctx)
{
    InitializePrimitiveClassRoot(ClassRoot::U1, Type::TypeId::U1, "Z");
    InitializePrimitiveClassRoot(ClassRoot::I8, Type::TypeId::I8, "B");
    InitializePrimitiveClassRoot(ClassRoot::U8, Type::TypeId::U8, "H");
    InitializePrimitiveClassRoot(ClassRoot::I16, Type::TypeId::I16, "S");
    InitializePrimitiveClassRoot(ClassRoot::U16, Type::TypeId::U16, "C");
    InitializePrimitiveClassRoot(ClassRoot::I32, Type::TypeId::I32, "I");
    InitializePrimitiveClassRoot(ClassRoot::U32, Type::TypeId::U32, "U");
    InitializePrimitiveClassRoot(ClassRoot::I64, Type::TypeId::I64, "J");
    InitializePrimitiveClassRoot(ClassRoot::U64, Type::TypeId::U64, "Q");
    InitializePrimitiveClassRoot(ClassRoot::F32, Type::TypeId::F32, "F");
    InitializePrimitiveClassRoot(ClassRoot::F64, Type::TypeId::F64, "D");
    InitializePrimitiveClassRoot(ClassRoot::TAGGED, Type::TypeId::TAGGED, "A");

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
                             utf::Mutf8AsCString(ctx.GetStringArrayClassDescriptor()));
}

bool CoreClassLinkerExtension::InitializeImpl(bool compressedStringEnabled)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(GetLanguage());

    auto *classClass = CreateClass(ctx.GetClassClassDescriptor(), GetClassVTableSize(ClassRoot::CLASS),
                                   GetClassIMTSize(ClassRoot::CLASS), GetClassSize(ClassRoot::CLASS));
    coretypes::Class::FromRuntimeClass(classClass)->SetClass(classClass);
    classClass->SetState(Class::State::LOADED);
    classClass->SetLoadContext(GetBootContext());
    GetClassLinker()->AddClassRoot(ClassRoot::CLASS, classClass);

    auto *objClass = GetClassLinker()->GetClass(ctx.GetObjectClassDescriptor(), false, GetBootContext());
    if (objClass == nullptr) {  // Happens when we work without pandastdlib
        objClass = CreateClass(ctx.GetObjectClassDescriptor(), GetClassVTableSize(ClassRoot::OBJECT),
                               GetClassIMTSize(ClassRoot::OBJECT), GetClassSize(ClassRoot::OBJECT));
        objClass->SetObjectSize(ObjectHeader::ObjectHeaderSize());
        objClass->SetState(Class::State::LOADED);
        objClass->SetLoadContext(GetBootContext());
        GetClassLinker()->AddClassRoot(ClassRoot::OBJECT, objClass);
    } else {
        SetClassRoot(ClassRoot::OBJECT, objClass);
    }
    classClass->SetBase(objClass);

    auto *stringClass = CreateClass(ctx.GetStringClassDescriptor(), GetClassVTableSize(ClassRoot::STRING),
                                    GetClassIMTSize(ClassRoot::STRING), GetClassSize(ClassRoot::STRING));
    stringClass->SetBase(objClass);
    stringClass->SetStringClass();
    coretypes::String::SetCompressedStringsEnabled(compressedStringEnabled);
    stringClass->SetState(Class::State::LOADED);
    stringClass->SetLoadContext(GetBootContext());
    GetClassLinker()->AddClassRoot(ClassRoot::STRING, stringClass);

    InitializeArrayClassRoot(ClassRoot::ARRAY_CLASS, ClassRoot::CLASS,
                             utf::Mutf8AsCString(ctx.GetClassArrayClassDescriptor()));

    InitializeClassRoots(ctx);
    return true;
}

bool CoreClassLinkerExtension::InitializeArrayClass(Class *arrayClass, Class *componentClass)
{
    ASSERT(IsInitialized());

    auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
    arrayClass->SetBase(objectClass);
    arrayClass->SetComponentType(componentClass);
    uint32_t accessFlags = componentClass->GetAccessFlags() & ACC_FILE_MASK;
    accessFlags &= ~ACC_INTERFACE;
    accessFlags |= ACC_FINAL | ACC_ABSTRACT;
    arrayClass->SetAccessFlags(accessFlags);
    arrayClass->SetState(Class::State::INITIALIZED);
    return true;
}

void CoreClassLinkerExtension::InitializePrimitiveClass(Class *primitiveClass)
{
    ASSERT(IsInitialized());

    primitiveClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    primitiveClass->SetState(Class::State::INITIALIZED);
}

size_t CoreClassLinkerExtension::GetClassVTableSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
}

size_t CoreClassLinkerExtension::GetClassIMTSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
}

size_t CoreClassLinkerExtension::GetClassSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
}

size_t CoreClassLinkerExtension::GetArrayClassVTableSize()
{
    ASSERT(IsInitialized());

    return GetClassVTableSize(ClassRoot::OBJECT);
}

size_t CoreClassLinkerExtension::GetArrayClassIMTSize()
{
    ASSERT(IsInitialized());

    return GetClassIMTSize(ClassRoot::OBJECT);
}

size_t CoreClassLinkerExtension::GetArrayClassSize()
{
    ASSERT(IsInitialized());

    return GetClassSize(ClassRoot::OBJECT);
}

Class *CoreClassLinkerExtension::CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size)
{
    ASSERT(IsInitialized());

    auto vm = Thread::GetCurrent()->GetVM();
    auto *heapManager = vm->GetHeapManager();

    auto *classRoot = GetClassRoot(ClassRoot::CLASS);
    ObjectHeader *objectHeader;
    if (classRoot == nullptr) {
        objectHeader = heapManager->AllocateNonMovableObject<true>(classRoot, coretypes::Class::GetSize(size));
    } else {
        objectHeader = heapManager->AllocateNonMovableObject<false>(classRoot, coretypes::Class::GetSize(size));
    }

    if (UNLIKELY(objectHeader == nullptr)) {
        return nullptr;
    }

    auto *res = reinterpret_cast<coretypes::Class *>(objectHeader);
    res->InitClass(descriptor, vtableSize, imtSize, size);
    auto *klass = res->GetRuntimeClass();
    klass->SetManagedObject(res);
    AddCreatedClass(klass);
    return klass;
}

void CoreClassLinkerExtension::FreeClass(Class *klass)
{
    ASSERT(IsInitialized());

    RemoveCreatedClass(klass);
}

CoreClassLinkerExtension::~CoreClassLinkerExtension()
{
    if (!IsInitialized()) {
        return;
    }

    FreeLoadedClasses();
}

}  // namespace ark
