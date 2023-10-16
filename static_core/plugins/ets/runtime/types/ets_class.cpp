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

#include "include/language_context.h"
#include "include/mem/panda_containers.h"
#include "libpandabase/utils/utf.h"
#include "napi/ets_napi.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_value.h"
#include "plugins/ets/runtime/types/ets_class.h"

namespace panda::ets {

uint32_t EtsClass::GetFieldsNumber()
{
    uint32_t fnumber = 0;
    EnumerateBaseClasses([&](EtsClass *c) {
        fnumber += c->GetRuntimeClass()->GetFields().Size();
        return false;
    });
    return fnumber;
}

PandaVector<EtsField *> EtsClass::GetFields()
{
    auto ets_fields = PandaVector<EtsField *>(Runtime::GetCurrent()->GetInternalAllocator()->Adapter());
    EnumerateBaseClasses([&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetFields();
        auto fnum = fields.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            ets_fields.push_back(EtsField::FromRuntimeField(&fields[i]));
        }
        return false;
    });
    return ets_fields;
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

EtsMethod *EtsClass::GetDirectMethod(const char *name, const char *signature)
{
    auto core_name = reinterpret_cast<const uint8_t *>(name);
    return GetDirectMethod(core_name, signature);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name)
{
    const uint8_t *mutf8_name = utf::CStringAsMutf8(name);
    Method *rt_method = GetRuntimeClass()->GetDirectMethod(mutf8_name);
    return EtsMethod::FromRuntimeMethod(rt_method);
}

EtsMethod *EtsClass::GetDirectMethod(const uint8_t *name, const char *signature)
{
    EtsMethodSignature method_signature(signature);
    if (!method_signature.IsValid()) {
        LOG(ERROR, ETS_NAPI) << "Wrong method signature: " << signature;
        return nullptr;
    }

    auto core_method = GetRuntimeClass()->GetDirectMethod(name, method_signature.GetProto());
    return reinterpret_cast<EtsMethod *>(core_method);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name, const Method::Proto &proto) const
{
    Method *method = klass_.GetDirectMethod(utf::CStringAsMutf8(name), proto);
    return EtsMethod::FromRuntimeMethod(method);
}

uint32_t EtsClass::GetMethodsNum()
{
    uint32_t fnumber = 0;
    EnumerateBaseClasses([&](EtsClass *c) {
        fnumber += c->GetRuntimeClass()->GetMethods().Size();
        return false;
    });
    return fnumber;
}

EtsMethod *EtsClass::GetMethodByIndex(uint32_t i)
{
    EtsMethod *res = nullptr;
    EnumerateBaseClasses([&](EtsClass *c) {
        auto methods = c->GetRuntimeClass()->GetMethods();
        auto fnum = methods.Size();
        if (i >= fnum) {
            i -= fnum;
            return false;
        }
        res = EtsMethod::FromRuntimeMethod(&methods[i]);
        return true;
    });
    return res;
}

EtsMethod *EtsClass::GetMethod(const char *name)
{
    auto core_name = reinterpret_cast<const uint8_t *>(name);

    Method *core_method = nullptr;
    auto *runtime_class = GetRuntimeClass();
    if (IsInterface()) {
        core_method = runtime_class->GetInterfaceMethod(core_name);
    } else {
        core_method = runtime_class->GetClassMethod(core_name);
    }
    return reinterpret_cast<EtsMethod *>(core_method);
}

EtsMethod *EtsClass::GetMethod(const char *name, const char *signature)
{
    EtsMethodSignature method_signature(signature);
    if (!method_signature.IsValid()) {
        LOG(ERROR, ETS_NAPI) << "Wrong method signature:" << signature;
        return nullptr;
    }

    auto core_name = reinterpret_cast<const uint8_t *>(name);

    Method *core_method = nullptr;
    auto *runtime_class = GetRuntimeClass();
    if (IsInterface()) {
        core_method = runtime_class->GetInterfaceMethod(core_name, method_signature.GetProto());
    } else {
        core_method = runtime_class->GetClassMethod(core_name, method_signature.GetProto());
    }
    return reinterpret_cast<EtsMethod *>(core_method);
}

PandaVector<EtsMethod *> EtsClass::GetMethods()
{
    auto ets_methods = PandaVector<EtsMethod *>(Runtime::GetCurrent()->GetInternalAllocator()->Adapter());
    EnumerateBaseClasses([&](EtsClass *c) {
        auto methods = c->GetRuntimeClass()->GetMethods();
        auto fnum = methods.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            // Skip constructors of base classes
            if (methods[i].IsConstructor() && c != this) {
                continue;
            }
            ets_methods.push_back(EtsMethod::FromRuntimeMethod(&methods[i]));
        }
        return false;
    });
    return ets_methods;
}

PandaVector<EtsMethod *> EtsClass::GetConstructors()
{
    auto constructors = PandaVector<EtsMethod *>();
    auto methods = GetMethods();
    std::copy_if(methods.begin(), methods.end(), std::back_inserter(constructors),
                 [this](EtsMethod *method) { return method->IsInstanceConstructor() && method->GetClass() == this; });
    return constructors;
}

EtsMethod *EtsClass::ResolveVirtualMethod(const EtsMethod *method) const
{
    return reinterpret_cast<EtsMethod *>(GetRuntimeClass()->ResolveVirtualMethod(method->GetPandaMethod()));
}

PandaVector<EtsClass *> EtsClass::GetInterfaces() const
{
    auto runtime_interfaces = GetRuntimeClass()->GetInterfaces();
    auto interfaces = PandaVector<EtsClass *>(runtime_interfaces.Size());
    for (size_t i = 0; i < interfaces.size(); i++) {
        interfaces[i] = EtsClass::FromRuntimeClass(runtime_interfaces[i]);
    }
    return interfaces;
}

/* static */
EtsClass *EtsClass::GetPrimitiveClass(EtsString *name)
{
    if (name == nullptr || name->GetMUtf8Length() < 2) {  // MUtf8Length must be >= 2
        return nullptr;
    }
    const char *primitive_name = nullptr;
    EtsClassRoot class_root;
    char hash = name->At(0) ^ ((name->At(1) & 0x10) << 1);  // NOLINT
    switch (hash) {
        case 'v':
            primitive_name = "void";
            class_root = EtsClassRoot::VOID;
            break;
        case 'b':
            primitive_name = "boolean";
            class_root = EtsClassRoot::BOOLEAN;
            break;
        case 'B':
            primitive_name = "byte";
            class_root = EtsClassRoot::BYTE;
            break;
        case 'c':
            primitive_name = "char";
            class_root = EtsClassRoot::CHAR;
            break;
        case 's':
            primitive_name = "short";
            class_root = EtsClassRoot::SHORT;
            break;
        case 'i':
            primitive_name = "int";
            class_root = EtsClassRoot::INT;
            break;
        case 'l':
            primitive_name = "long";
            class_root = EtsClassRoot::LONG;
            break;
        case 'f':
            primitive_name = "float";
            class_root = EtsClassRoot::FLOAT;
            break;
        case 'd':
            primitive_name = "double";
            class_root = EtsClassRoot::DOUBLE;
            break;
        default:
            break;
    }

    // StringIndexOutOfBoundsException is not thrown by At method above, because index (0, 1) < length (>= 2)
    if (primitive_name != nullptr && name->IsEqual(primitive_name)) {  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
        return PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(class_root);
    }

    return nullptr;
}

EtsString *EtsClass::CreateEtsClassName([[maybe_unused]] const char *descriptor)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    if (*descriptor == 'L') {
        std::string_view tmp_name(descriptor);
        tmp_name.remove_prefix(1);
        tmp_name.remove_suffix(1);
        PandaString ets_name(tmp_name);
        std::replace(ets_name.begin(), ets_name.end(), '/', '.');
        return EtsString::CreateFromMUtf8(ets_name.data(), ets_name.length());
    }
    if (*descriptor == '[') {
        PandaString ets_name(descriptor);
        std::replace(ets_name.begin(), ets_name.end(), '/', '.');
        return EtsString::CreateFromMUtf8(ets_name.data(), ets_name.length());
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
        success = CompareAndSetName(nullptr, name);
    } while (!success);
    return name;
}

bool EtsClass::IsInSamePackage(std::string_view class_name1, std::string_view class_name2)
{
    size_t i = 0;
    size_t min_length = std::min(class_name1.size(), class_name2.size());
    while (i < min_length && class_name1[i] == class_name2[i]) {
        ++i;
    }
    return class_name1.find('/', i) == std::string::npos && class_name2.find('/', i) == std::string::npos;
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

/* static */
bool EtsClass::IsClassFinalizable(EtsClass *klass)
{
    Method *method = klass->GetRuntimeClass()->GetClassMethod(reinterpret_cast<const uint8_t *>("finalize"));
    if (method != nullptr) {
        uint32_t num_args = method->GetNumArgs();
        const panda_file::Type &return_type = method->GetReturnType();
        auto code_size = method->GetCodeSize();
        // in empty method code_size = 1 (return.Void)
        if (num_args == 1 && return_type.GetId() == panda_file::Type::TypeId::VOID && code_size > 1 &&
            !method->IsStatic()) {
            return true;
        }
    }
    return false;
}

void EtsClass::SetSoftReference()
{
    flags_ = flags_ | IS_SOFT_REFERENCE;
    ASSERT(IsSoftReference() && IsReference());
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
void EtsClass::SetPhantomReference()
{
    flags_ = flags_ | IS_PHANTOM_REFERENCE;
    ASSERT(IsPhantomReference() && IsReference());
}

void EtsClass::SetFinalizable()
{
    flags_ = flags_ | IS_CLASS_FINALIZABLE;
    ASSERT(IsFinalizable() && IsReference());
}

bool EtsClass::IsSoftReference() const
{
    return (flags_ & IS_SOFT_REFERENCE) != 0;
}

bool EtsClass::IsWeakReference() const
{
    return (flags_ & IS_WEAK_REFERENCE) != 0;
}

bool EtsClass::IsFinalizerReference() const
{
    return (flags_ & IS_FINALIZE_REFERENCE) != 0;
}

bool EtsClass::IsPhantomReference() const
{
    return (flags_ & IS_PHANTOM_REFERENCE) != 0;
}

bool EtsClass::IsReference() const
{
    return (flags_ & IS_REFERENCE) != 0;
}

bool EtsClass::IsFinalizable() const
{
    return (flags_ & IS_CLASS_FINALIZABLE) != 0;
}

}  // namespace panda::ets
