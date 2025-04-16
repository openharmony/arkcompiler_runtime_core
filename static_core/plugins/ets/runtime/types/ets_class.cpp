/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "include/language_context.h"
#include "include/mem/panda_containers.h"
#include "libpandabase/utils/utf.h"
#include "macros.h"
#include "napi/ets_napi.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_value.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets {

uint32_t EtsClass::GetFieldsNumber()
{
    uint32_t fnumber = 0;
    EnumerateBaseClasses([&](EtsClass *c) {
        fnumber += c->GetRuntimeClass()->GetFields().Size();
        return false;
    });
    return fnumber;
}

// Without inherited fields
uint32_t EtsClass::GetOwnFieldsNumber()
{
    return GetRuntimeClass()->GetFields().Size();
}

PandaVector<EtsField *> EtsClass::GetFields()
{
    auto etsFields = PandaVector<EtsField *>(Runtime::GetCurrent()->GetInternalAllocator()->Adapter());
    EnumerateBaseClasses([&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetFields();
        auto fnum = fields.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            etsFields.push_back(EtsField::FromRuntimeField(&fields[i]));
        }
        return false;
    });
    return etsFields;
}

EtsField *EtsClass::GetFieldByIndex(uint32_t i)
{
    EtsField *res = nullptr;
    EnumerateBaseClasses([&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetFields();
        auto fnum = fields.Size();
        if (i >= fnum) {
            i -= fnum;
            return false;
        }
        res = EtsField::FromRuntimeField(&fields[i]);
        return true;
    });
    return res;
}

EtsField *EtsClass::GetOwnFieldByIndex(uint32_t i)
{
    return EtsField::FromRuntimeField(&GetRuntimeClass()->GetFields()[i]);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name, const char *signature)
{
    auto coreName = reinterpret_cast<const uint8_t *>(name);
    return GetDirectMethod(coreName, signature);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name)
{
    const uint8_t *mutf8Name = utf::CStringAsMutf8(name);
    Method *rtMethod = GetRuntimeClass()->GetDirectMethod(mutf8Name);
    return EtsMethod::FromRuntimeMethod(rtMethod);
}

EtsMethod *EtsClass::GetDirectMethod(const uint8_t *name, const char *signature)
{
    EtsMethodSignature methodSignature(signature);
    if (!methodSignature.IsValid()) {
        LOG(ERROR, ETS_NAPI) << "Wrong method signature: " << signature;
        return nullptr;
    }

    auto coreMethod = GetRuntimeClass()->GetDirectMethod(name, methodSignature.GetProto());
    return reinterpret_cast<EtsMethod *>(coreMethod);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name, const Method::Proto &proto) const
{
    Method *method = klass_.GetDirectMethod(utf::CStringAsMutf8(name), proto);
    return EtsMethod::FromRuntimeMethod(method);
}

uint32_t EtsClass::GetMethodsNum()
{
    return GetMethods().size();
}

EtsMethod *EtsClass::GetMethodByIndex(uint32_t ind)
{
    EtsMethod *res = nullptr;
    auto methods = GetMethods();
    ASSERT(ind < methods.size());
    res = methods[ind];
    return res;
}

// NOTE(kirill-mitkin): Cache in EtsClass field later
PandaVector<EtsMethod *> EtsClass::GetMethods()
{
    PandaUnorderedMap<PandaString, EtsMethod *> uniqueMethods;

    auto addDirectMethods = [&](const EtsClass *c) {
        auto directMethods = c->GetRuntimeClass()->GetMethods();
        for (auto &method : directMethods) {
            PandaString methodFullName = utf::Mutf8AsCString(method.GetName().data);
            methodFullName += method.GetProto().GetSignature();
            if (uniqueMethods.find(methodFullName) == uniqueMethods.end()) {
                uniqueMethods[methodFullName] = EtsMethod::FromRuntimeMethod(&method);
            }
        }
    };

    auto addDirectMethodsForBaseClass = [&uniqueMethods](EtsClass *c) {
        auto directMethods = c->GetRuntimeClass()->GetMethods();
        auto fnum = directMethods.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            Method *method = &directMethods[i];
            // Skip constructors
            if (method->IsConstructor()) {
                continue;
            }

            PandaString methodFullName = utf::Mutf8AsCString((method->GetName().data));
            methodFullName += method->GetProto().GetSignature();

            uniqueMethods[methodFullName] = EtsMethod::FromRuntimeMethod(method);
        }
    };

    if (IsInterface()) {
        addDirectMethods(this);
        EnumerateInterfaces([&](const EtsClass *c) {
            addDirectMethods(c);
            return false;
        });
    } else {
        EnumerateBaseClasses([&](EtsClass *c) {
            addDirectMethodsForBaseClass(c);
            return false;
        });
    }
    auto etsMethods = PandaVector<EtsMethod *>();
    for (auto &iter : uniqueMethods) {
        etsMethods.push_back(iter.second);
    }
    return etsMethods;
}

PandaVector<EtsMethod *> EtsClass::GetConstructors()
{
    auto constructors = PandaVector<EtsMethod *>();
    auto methods = GetRuntimeClass()->GetMethods();
    // NOTE(kirill-mitkin): cache in ets_class field
    for (auto &method : methods) {
        // Skip constructors
        if (method.IsInstanceConstructor()) {
            constructors.emplace_back(EtsMethod::FromRuntimeMethod(&method));
        }
    }
    return constructors;
}

EtsMethod *EtsClass::ResolveVirtualMethod(const EtsMethod *method) const
{
    return reinterpret_cast<EtsMethod *>(GetRuntimeClass()->ResolveVirtualMethod(method->GetPandaMethod()));
}

static std::pair<char const *, EtsClassRoot> GetNameAndClassRoot(char hash)
{
    const char *primitiveName = nullptr;
    EtsClassRoot classRoot;

    switch (hash) {
        case 'v':
            primitiveName = "void";
            classRoot = EtsClassRoot::VOID;
            break;
        case 'b':
            primitiveName = "boolean";
            classRoot = EtsClassRoot::BOOLEAN;
            break;
        case 'B':
            primitiveName = "byte";
            classRoot = EtsClassRoot::BYTE;
            break;
        case 'c':
            primitiveName = "char";
            classRoot = EtsClassRoot::CHAR;
            break;
        case 's':
            primitiveName = "short";
            classRoot = EtsClassRoot::SHORT;
            break;
        case 'i':
            primitiveName = "int";
            classRoot = EtsClassRoot::INT;
            break;
        case 'l':
            primitiveName = "long";
            classRoot = EtsClassRoot::LONG;
            break;
        case 'f':
            primitiveName = "float";
            classRoot = EtsClassRoot::FLOAT;
            break;
        case 'd':
            primitiveName = "double";
            classRoot = EtsClassRoot::DOUBLE;
            break;
        default:
            break;
    }

    return {primitiveName, classRoot};
}

/* static */
EtsClass *EtsClass::GetPrimitiveClass(EtsString *name)
{
    if (name == nullptr || name->GetMUtf8Length() < 2) {  // MUtf8Length must be >= 2
        return nullptr;
    }
    // StringIndexOutOfBoundsException is not thrown by At method, because index (0, 1) < length (>= 2)
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    char hash = name->At(0) ^ ((name->At(1) & 0x10) << 1);  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
    auto [primitiveName, classRoot] = GetNameAndClassRoot(hash);

    if (primitiveName != nullptr && name->IsEqual(primitiveName)) {  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
        return PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(classRoot);
    }

    return nullptr;
}

EtsString *EtsClass::CreateEtsClassName([[maybe_unused]] const char *descriptor)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    if (*descriptor == 'L') {
        std::string_view tmpName(descriptor);
        tmpName.remove_prefix(1);
        tmpName.remove_suffix(1);
        PandaString etsName(tmpName);
        std::replace(etsName.begin(), etsName.end(), '/', '.');
        return EtsString::CreateFromMUtf8(etsName.data(), etsName.length());
    }
    if (*descriptor == '[') {
        PandaString etsName(descriptor);
        std::replace(etsName.begin(), etsName.end(), '/', '.');
        return EtsString::CreateFromMUtf8(etsName.data(), etsName.length());
    }

    switch (*descriptor) {
        case 'Z':
            return EtsString::CreateFromMUtf8("boolean");
        case 'B':
            return EtsString::CreateFromMUtf8("byte");
        case 'C':
            return EtsString::CreateFromMUtf8("char");
        case 'S':
            return EtsString::CreateFromMUtf8("short");
        case 'I':
            return EtsString::CreateFromMUtf8("int");
        case 'J':
            return EtsString::CreateFromMUtf8("long");
        case 'F':
            return EtsString::CreateFromMUtf8("float");
        case 'D':
            return EtsString::CreateFromMUtf8("double");
        case 'V':
            return EtsString::CreateFromMUtf8("void");
        default:
            LOG(FATAL, RUNTIME) << "Incorrect primitive name" << descriptor;
            UNREACHABLE();
    }
}

EtsString *EtsClass::GetName()
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    EtsString *name = nullptr;
    bool success = false;

    do {
        name = reinterpret_cast<EtsString *>(GetObjectHeader()->GetFieldObject(GetNameOffset()));
        if (name != nullptr) {
            return name;
        }

        name = CreateEtsClassName(GetDescriptor());
        if (name == nullptr) {
            return nullptr;
        }
        success = CompareAndSetName(nullptr, name);
    } while (!success);
    return name;
}

bool EtsClass::IsInSamePackage(std::string_view className1, std::string_view className2)
{
    size_t i = 0;
    size_t minLength = std::min(className1.size(), className2.size());
    while (i < minLength && className1[i] == className2[i]) {
        ++i;
    }
    return className1.find('/', i) == std::string::npos && className2.find('/', i) == std::string::npos;
}

bool EtsClass::IsInSamePackage(EtsClass *that)
{
    if (this == that) {
        return true;
    }

    EtsClass *klass1 = this;
    EtsClass *klass2 = that;
    while (klass1->IsArrayClass()) {
        klass1 = klass1->GetComponentType();
    }
    while (klass2->IsArrayClass()) {
        klass2 = klass2->GetComponentType();
    }
    if (klass1 == klass2) {
        return true;
    }

    // Compare the package part of the descriptor string.
    return IsInSamePackage(klass1->GetDescriptor(), klass2->GetDescriptor());
}

void EtsClass::SetWeakReference()
{
    flags_ = flags_ | IS_WEAK_REFERENCE;
    ASSERT(IsWeakReference() && IsReference());
}
void EtsClass::SetFinalizeReference()
{
    flags_ = flags_ | IS_FINALIZE_REFERENCE;
    ASSERT(IsFinalizerReference() && IsReference());
}

void EtsClass::SetValueTyped()
{
    flags_ = flags_ | IS_VALUE_TYPED;
    ASSERT(IsValueTyped());
}
void EtsClass::SetNullValue()
{
    flags_ = flags_ | IS_NULLVALUE;
    ASSERT(IsNullValue());
}
void EtsClass::SetBoxed()
{
    flags_ = flags_ | IS_BOXED;
    ASSERT(IsBoxed());
}
void EtsClass::SetFunction()
{
    flags_ = flags_ | IS_FUNCTION;
    ASSERT(IsFunction());
}
void EtsClass::SetEtsEnum()
{
    flags_ = flags_ | IS_ETS_ENUM;
    ASSERT(IsEtsEnum());
}
void EtsClass::SetBigInt()
{
    flags_ = flags_ | IS_BIGINT;
    ASSERT(IsBigInt());
}

static bool HasFunctionTypeInSuperClasses(EtsClass *cls)
{
    if (EtsClass *base = cls->GetBase(); base != nullptr) {
        if (UNLIKELY(base->IsFunction())) {
            return true;
        }
    }
    for (Class *iface : cls->GetRuntimeClass()->GetInterfaces()) {
        if (UNLIKELY(EtsClass::FromRuntimeClass(iface)->IsFunction())) {
            return true;
        }
    }
    return false;
}

void EtsClass::Initialize(EtsClass *superClass, uint16_t accessFlags, bool isPrimitiveType)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    SetName(nullptr);
    SetSuperClass(superClass);

    uint32_t flags = accessFlags;
    if (isPrimitiveType) {
        flags |= ETS_ACC_PRIMITIVE;
    }

    if (superClass != nullptr) {
        static constexpr uint32_t COPIED_MASK = IS_WEAK_REFERENCE | IS_FINALIZE_REFERENCE;
        flags |= superClass->GetFlags() & COPIED_MASK;
        ASSERT(!superClass->IsValueTyped() || superClass->IsEtsEnum());
    }
    if (UNLIKELY(HasFunctionTypeInSuperClasses(this))) {
        flags |= IS_FUNCTION;
    }
    if (UNLIKELY(GetBase() != nullptr && GetBase()->IsEtsEnum())) {
        flags |= (IS_ETS_ENUM | IS_VALUE_TYPED);
    }

    auto *runtimeClass = GetRuntimeClass();
    auto *pfile = runtimeClass->GetPandaFile();
    if (pfile != nullptr) {
        panda_file::ClassDataAccessor cda(*pfile, runtimeClass->GetFileId());

        cda.EnumerateAnnotation(panda_file_items::class_descriptors::ANNOTATION_MODULE.data(),
                                [&flags](panda_file::AnnotationDataAccessor &) {
                                    flags |= IS_MODULE;
                                    return true;
                                });
    }

    SetFlags(flags);
}

void EtsClass::SetComponentType(EtsClass *componentType)
{
    if (componentType == nullptr) {
        GetRuntimeClass()->SetComponentType(nullptr);
        return;
    }
    GetRuntimeClass()->SetComponentType(componentType->GetRuntimeClass());
}

EtsClass *EtsClass::GetComponentType() const
{
    ark::Class *componentType = GetRuntimeClass()->GetComponentType();
    if (componentType == nullptr) {
        return nullptr;
    }
    return FromRuntimeClass(componentType);
}

void EtsClass::SetName(EtsString *name)
{
    GetObjectHeader()->SetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(name));
}

bool EtsClass::CompareAndSetName(EtsString *oldName, EtsString *newName)
{
    return GetObjectHeader()->CompareAndSetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(oldName),
                                                       reinterpret_cast<ObjectHeader *>(newName),
                                                       std::memory_order::memory_order_seq_cst, true);
}

EtsField *EtsClass::GetFieldIDByName(const char *name, const char *sig)
{
    auto u8name = reinterpret_cast<const uint8_t *>(name);
    auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetInstanceFieldByName(u8name));

    if (sig != nullptr && field != nullptr) {
        if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
            return nullptr;
        }
    }

    return field;
}

uint32_t EtsClass::GetFieldIndexByName(const char *name)
{
    auto u8name = reinterpret_cast<const uint8_t *>(name);
    auto fields = GetRuntimeClass()->GetFields();
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(u8name)), u8name};
    for (uint32_t i = 0; i < GetFieldsNumber(); i++) {
        if (fields[i].GetName() == sd) {
            return i;
        }
    }
    return -1;
}

EtsField *EtsClass::GetStaticFieldIDByName(const char *name, const char *sig)
{
    auto u8name = reinterpret_cast<const uint8_t *>(name);
    auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetStaticFieldByName(u8name));

    if (sig != nullptr && field != nullptr) {
        if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
            return nullptr;
        }
    }

    return field;
}

EtsField *EtsClass::GetDeclaredFieldIDByName(const char *name)
{
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindDeclaredField([name](const ark::Field &field) -> bool {
        auto *efield = EtsField::FromRuntimeField(&field);
        return ::strcmp(efield->GetName(), name) == 0;
    }));
}

EtsField *EtsClass::GetFieldIDByOffset(uint32_t fieldOffset)
{
    auto pred = [fieldOffset](const ark::Field &f) { return f.GetOffset() == fieldOffset; };
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindInstanceField(pred));
}

EtsField *EtsClass::GetStaticFieldIDByOffset(uint32_t fieldOffset)
{
    auto pred = [fieldOffset](const ark::Field &f) { return f.GetOffset() == fieldOffset; };
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindStaticField(pred));
}

EtsClass *EtsClass::GetBase()
{
    if (IsInterface()) {
        return nullptr;
    }
    auto *base = GetRuntimeClass()->GetBase();
    if (base == nullptr) {
        return nullptr;
    }
    return FromRuntimeClass(base);
}

void EtsClass::GetInterfaces(PandaUnorderedSet<EtsClass *> &ifaces, EtsClass *iface)
{
    ifaces.insert(iface);
    EnumerateDirectInterfaces([&](EtsClass *runtimeInterface) {
        if (ifaces.find(runtimeInterface) == ifaces.end()) {
            runtimeInterface->GetInterfaces(ifaces, runtimeInterface);
        }
        return false;
    });
}

EtsObject *EtsClass::GetStaticFieldObject(EtsField *field)
{
    return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject(*field->GetRuntimeField()));
}

EtsObject *EtsClass::GetStaticFieldObject(int32_t fieldOffset, bool isVolatile)
{
    if (isVolatile) {
        return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<true>(fieldOffset));
    }
    return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<false>(fieldOffset));
}

void EtsClass::SetStaticFieldObject(EtsField *field, EtsObject *value)
{
    GetRuntimeClass()->SetFieldObject(*field->GetRuntimeField(), reinterpret_cast<ObjectHeader *>(value));
}

void EtsClass::SetStaticFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value)
{
    if (isVolatile) {
        GetRuntimeClass()->SetFieldObject<true>(fieldOffset, reinterpret_cast<ObjectHeader *>(value));
    }
    GetRuntimeClass()->SetFieldObject<false>(fieldOffset, reinterpret_cast<ObjectHeader *>(value));
}

EtsObject *EtsClass::CreateInstance()
{
    auto coro = EtsCoroutine::GetCurrent();
    const auto throwCreateInstanceErr = [coro, this](std::string_view msg) {
        ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR,
                               PandaString(msg) + " " + GetDescriptor());
    };

    if (UNLIKELY(!GetRuntimeClass()->IsInstantiable() || IsArrayClass())) {
        throwCreateInstanceErr("Cannot instantiate");
        return nullptr;
    }

    if (IsStringClass()) {
        return EtsString::CreateNewEmptyString()->AsObject();
    }

    EtsMethod *ctor = GetDirectMethod(panda_file_items::CTOR.data(), ":V");
    if (UNLIKELY(ctor == nullptr)) {
        throwCreateInstanceErr("No default constructor in");
        return nullptr;
    }

    EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
    if (UNLIKELY(!IsInitialized() && !linker->InitializeClass(coro, this))) {
        return nullptr;
    }
    EtsObject *obj = EtsObject::Create(this);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }

    LocalObjectHandle objHandle(coro, obj);
    std::array<Value, 1> args {Value(obj->GetCoreType())};
    ctor->GetPandaMethod()->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return objHandle.GetPtr();
}

}  // namespace ark::ets
