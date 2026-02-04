/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "include/object_header.h"
#include "intrinsics.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_reflect_field.h"
#include "include/mem/panda_containers.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/intrinsics/helpers/reflection_helpers.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/handle_scope-inl.h"
#include "libarkbase/utils/utf.h"

#include <optional>

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/interop_context.h"
#endif /* PANDA_ETS_INTEROP_JS */

namespace ark::ets::intrinsics {

template <class Callback>
static void EnumerateBaseClassesReverseOrder(EtsClass *cls, const Callback &callback)
{
    auto currentClass = cls;
    bool finished = false;
    while (currentClass && !finished) {
        if constexpr (std::is_same_v<std::invoke_result_t<Callback, decltype(cls)>, void>) {
            callback(currentClass);
            finished = false;
        } else {
            finished = callback(currentClass);
        }
        currentClass = currentClass->GetSuperClass();
    }
}

EtsString *StdCoreClassGetNameInternal(EtsClass *cls)
{
    return cls->GetName();
}

EtsRuntimeLinker *StdCoreClassGetLinkerInternal(EtsClass *cls)
{
    auto *ctx = cls->GetLoadContext();
    return ctx->IsBootContext() ? nullptr : EtsClassLinkerContext::FromCoreType(ctx)->GetRuntimeLinker();
}

EtsBoolean StdCoreClassIsNamespace(EtsClass *cls)
{
    return cls->IsModule();
}

EtsClass *StdCoreClassOf(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    auto *cls = obj->GetClass();
    return cls->ResolvePublicClass();
}

EtsClass *StdCoreClassCurrent()
{
    return GetMethodOwnerClassInFrames(EtsCoroutine::GetCurrent(), 0);
}

EtsClass *StdCoreClassOfCaller()
{
    return GetMethodOwnerClassInFrames(EtsCoroutine::GetCurrent(), 1);
}

void StdCoreClassInitialize(EtsClass *cls)
{
    ASSERT(cls != nullptr);

    if (UNLIKELY(!cls->IsInitialized())) {
        auto coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
        linker->InitializeClass(coro, cls);
    }
}

EtsString *StdCoreClassGetDescriptor(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return EtsString::CreateFromMUtf8(cls->GetDescriptor());
}

ObjectHeader *StdCoreClassGetInterfaces(EtsClass *cls)
{
    auto *coro = EtsCoroutine::GetCurrent();
    const auto interfaces = cls->GetRuntimeClass()->GetInterfaces();
    auto result = EtsObjectArray::Create(PlatformTypes(coro)->coreClass, interfaces.size());
    for (size_t i = 0; i < interfaces.size(); i++) {
        result->Set(i, reinterpret_cast<EtsObject *>(EtsClass::FromRuntimeClass(interfaces[i])));
    }
    return reinterpret_cast<ObjectHeader *>(result);
}

EtsObject *StdCoreClassCreateInstance(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->CreateInstance();
}

EtsClass *EtsRuntimeLinkerFindLoadedClass(EtsRuntimeLinker *runtimeLinker, EtsString *clsName)
{
    ark::ets::ClassPublicNameParser parser(clsName->GetMutf8());
    const auto name = parser.Resolve();
    const auto *classDescriptor = utf::CStringAsMutf8(name.c_str());
    if (classDescriptor == nullptr) {
        return nullptr;
    }
    return runtimeLinker->FindLoadedClass(classDescriptor);
}

void EtsRuntimeLinkerInitializeContext(EtsRuntimeLinker *runtimeLinker)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *ext = coro->GetPandaVM()->GetEtsClassLinkerExtension();
    if (UNLIKELY(runtimeLinker->IsInstanceOf(PlatformTypes(coro)->coreBootRuntimeLinker))) {
        // BootRuntimeLinker is a singletone, initialize it once
        runtimeLinker->SetClassLinkerContext(ext->GetBootContext());
        return;
    }

    ext->RegisterContext([ext, runtimeLinker]() {
        ASSERT(runtimeLinker->GetClassLinkerContext() == nullptr);
        auto allocator = ext->GetClassLinker()->GetAllocator();
        auto *ctx = allocator->New<EtsClassLinkerContext>(runtimeLinker);
        runtimeLinker->SetClassLinkerContext(ctx);
        return ctx;
    });
#ifdef PANDA_ETS_INTEROP_JS
    // At first call to ets from js, no available class linker context on the stack
    // Thus, register the first available application class linker context as default class linker context for interop
    // Tracked in #24199
    interop::js::InteropCtx::InitializeDefaultLinkerCtxIfNeeded(runtimeLinker);
#endif /* PANDA_ETS_INTEROP_JS */
}

EtsClass *EtsBootRuntimeLinkerFindAndLoadClass(ObjectHeader *runtimeLinker, EtsString *clsName, EtsBoolean init)
{
    ark::ets::ClassPublicNameParser parser(clsName->GetMutf8());
    const auto name = parser.Resolve();
    auto *classDescriptor = utf::CStringAsMutf8(name.c_str());
    if (classDescriptor == nullptr) {
        return nullptr;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    // Use core ClassLinker in order to pass nullptr as error handler,
    // as exception is thrown in managed RuntimeLinker.loadClass
    auto *linker = Runtime::GetCurrent()->GetClassLinker();
    auto *ctx = EtsRuntimeLinker::FromCoreType(runtimeLinker)->GetClassLinkerContext();
    ASSERT(ctx->IsBootContext());
    auto *klass = linker->GetClass(classDescriptor, true, ctx, nullptr);
    if (klass == nullptr) {
        return nullptr;
    }

    if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
        if (UNLIKELY(!linker->InitializeClass(coro, klass))) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
    }
    return EtsClass::FromRuntimeClass(klass);
}

EtsRuntimeLinker *EtsGetNearestNonBootRuntimeLinker()
{
    auto *coro = EtsCoroutine::GetCurrent();
    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        auto *method = stack.GetMethod();
        if (LIKELY(method != nullptr)) {
            auto *cls = method->GetClass();
            ASSERT(cls != nullptr);
            auto *ctx = cls->GetLoadContext();
            ASSERT(ctx != nullptr);
            if (!ctx->IsBootContext()) {
                return EtsClassLinkerContext::FromCoreType(ctx)->GetRuntimeLinker();
            }
        }
    }
    return nullptr;
}

template <bool IS_STATIC = false, bool IS_CONSTRUCTOR = false>
static ObjectHeader *CreateEtsReflectMethodArray(EtsCoroutine *coro, const PandaVector<EtsMethod *> &methods)
{
    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsClass *klass = IS_CONSTRUCTOR ? PlatformTypes(coro)->coreReflectConstructor
                                     : (IS_STATIC ? PlatformTypes(coro)->coreReflectStaticMethod
                                                  : PlatformTypes(coro)->coreReflectInstanceMethod);
    ASSERT(klass != nullptr);

    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(klass, methods.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(arrayH.GetPtr() != nullptr);

    for (size_t idx = 0; idx < methods.size(); ++idx) {
        auto *reflectMethod = EtsReflectMethod::CreateFromEtsMethod(coro, methods[idx]);
        arrayH->Set(idx, reflectMethod->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

ObjectHeader *StdCoreClassGetInstanceMethodsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    auto *coro = EtsCoroutine::GetCurrent();
    PandaVector<EtsMethod *> instanceMethods;
    for (auto iter = helpers::InstanceMethodsIterator(cls, FromEtsBoolean(publicOnly)); !iter.IsEmpty(); iter.Next()) {
        instanceMethods.emplace_back(iter.Value());
    }
    return CreateEtsReflectMethodArray(coro, instanceMethods);
}

// Match method by comparing each param type to the caller's Class array (same runtime class).
// Returns std::nullopt when an exception occurred (caller should return nullptr).
static std::optional<bool> MethodParamsMatchClassArray(ark::Method &m, const EtsObjectArray *signatureArr)
{
    EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(&m);
    const uint32_t paramCount = etsMethod->GetParametersNum();
    if (paramCount != signatureArr->GetLength()) {
        return false;
    }
    for (uint32_t i = 0; i < paramCount; ++i) {
        EtsClass *expected = etsMethod->ResolveArgType(i + 1);  // +1: instance method has 'this' at 0
        if (UNLIKELY(expected == nullptr)) {
            ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
            return std::nullopt;
        }
        EtsClass *actual = EtsClass::FromEtsClassObject(signatureArr->Get(i));
        if (expected != actual) {
            return false;
        }
    }
    return true;
}

// Helper to build StringData from method name.
static panda_file::File::StringData MakeStringData(const char *name)
{
    const uint8_t *mutf8Name = utf::CStringAsMutf8(name);
    return {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
}

// Resolve instance method by name. Returns:
//   - std::nullopt if multiple overloads found (ambiguous)
//   - std::optional<EtsMethod*> with nullptr if no method found
//   - std::optional<EtsMethod*> with non-null value if single method found
static std::optional<EtsMethod *> ResolveInstanceMethodByName(EtsClass *cls, const char *name, bool publicOnly)
{
    panda_file::File::StringData methodName = MakeStringData(name);
    const ark::MethodNameComp comp;
    EtsMethod *firstWithName = nullptr;
    for (auto iter = helpers::InstanceMethodsIterator(cls, publicOnly); !iter.IsEmpty(); iter.Next()) {
        ark::Method &m = *iter.Value()->GetPandaMethod();
        if (!comp.equal(m, methodName)) {
            continue;
        }
        if (firstWithName == nullptr) {
            firstWithName = iter.Value();
        } else {
            return std::nullopt;  // multiple overloads found
        }
    }
    return firstWithName;  // nullptr if not found, non-null if single method found
}

static EtsMethod *GetInstanceMethodByParamClasses(EtsClass *cls, const char *name, const EtsObjectArray *signatureArr,
                                                  bool publicOnly)
{
    panda_file::File::StringData methodName = MakeStringData(name);
    const ark::MethodNameComp comp;

    for (auto iter = helpers::InstanceMethodsIterator(cls, publicOnly); !iter.IsEmpty(); iter.Next()) {
        ark::Method &m = *iter.Value()->GetPandaMethod();
        if (!comp.equal(m, methodName)) {
            continue;
        }
        std::optional<bool> match = MethodParamsMatchClassArray(m, signatureArr);
        if (UNLIKELY(!match.has_value())) {
            return nullptr;
        }
        if (*match) {
            return iter.Value();
        }
    }
    return nullptr;
}

EtsReflectMethod *StdCoreClassGetInstanceMethodInternal(EtsClass *cls, EtsString *name, ObjectHeader *signature,
                                                        EtsBoolean publicOnly)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    bool publicOnlyBool = FromEtsBoolean(publicOnly);

    PandaString nameStr = name->GetMutf8();
    const char *nameCStr = nameStr.c_str();

    // signature contains param types only (no return type); match by param types only.
    // Only the given class is queried (no base classes), consistent with InstanceMethodsIterator / getInstanceMethods.
    EtsMethod *method = nullptr;
    if (signature != nullptr) {
        // Caller passed a signature array of Class instances (e.g. [Class.PRIMITIVE_INT, Class.STRING]).
        // Match by comparing each param's Class object (same GetRuntimeClass()) to the method's param type.
        const auto *signatureArr = EtsObjectArray::FromCoreType(signature);
        method = GetInstanceMethodByParamClasses(cls, nameCStr, signatureArr, publicOnlyBool);
    } else {
        std::optional<EtsMethod *> resolved = ResolveInstanceMethodByName(cls, nameCStr, publicOnlyBool);
        if (UNLIKELY(!resolved.has_value())) {
            // Multiple overloads found
            PandaOStringStream msg;
            msg << "Found two methods with the same name '" << nameCStr << "'. Please specify signature.";
            ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR, msg.str().c_str());
            return nullptr;  // unreachable after throw, but makes control flow clear
        }
        method = *resolved;  // nullptr if not found, non-null if single method found
    }

    if (UNLIKELY(method == nullptr || (publicOnlyBool && !method->IsPublic()))) {
        return nullptr;
    }

    return EtsReflectMethod::CreateFromEtsMethod(coro, method);
}

EtsReflectField *StdCoreClassGetInstanceFieldByNameInternal(EtsClass *cls, EtsString *name, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    PandaVector<uint8_t> tree8Buf;
    Field *field = cls->GetRuntimeClass()->GetDeclaredInstanceFieldByName(name->ConvertToStringView(&tree8Buf));
    if (field == nullptr || (FromEtsBoolean(publicOnly) && !field->IsPublic())) {
        return nullptr;
    }

    return EtsReflectField::CreateFromEtsField(coro, EtsField::FromRuntimeField(field));
}

EtsReflectField *StdCoreClassGetStaticFieldByNameInternal(EtsClass *cls, EtsString *name, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    PandaVector<uint8_t> tree8Buf;
    Field *field = cls->GetRuntimeClass()->GetDeclaredStaticFieldByName(name->ConvertToStringView(&tree8Buf));
    if (field == nullptr || (FromEtsBoolean(publicOnly) && !field->IsPublic())) {
        return nullptr;
    }

    return EtsReflectField::CreateFromEtsField(coro, EtsField::FromRuntimeField(field));
}

static PandaVector<EtsMethod *> GetConstructors(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsMethod *> constructors;

    cls->EnumerateDirectMethods([&constructors, onlyPublic](EtsMethod *method) {
        if (method->IsConstructor()) {
            if ((onlyPublic && method->IsPublic()) || !onlyPublic) {
                constructors.push_back(method);
            }
        }
        return false;
    });

    return constructors;
}

ObjectHeader *StdCoreClassGetConstructorsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto constructors = GetConstructors(cls, FromEtsBoolean(publicOnly));

    return CreateEtsReflectMethodArray<false, true>(coro, constructors);
}

ObjectHeader *StdCoreClassGetStaticMethodsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    bool collectPublic = FromEtsBoolean(publicOnly);
    PandaVector<EtsMethod *> staticMethods;

    Span<Method> sMethods = cls->GetRuntimeClass()->GetStaticMethods();

    for (auto &method : sMethods) {
        if (!collectPublic || method.IsPublic()) {
            staticMethods.push_back(EtsMethod::FromRuntimeMethod(&method));
        }
    }

    return CreateEtsReflectMethodArray<true, false>(coro, staticMethods);
}

template <bool IS_STATIC = false>
static ObjectHeader *CreateEtsReflectFieldArray(EtsCoroutine *coro, const PandaVector<EtsField *> &fields)
{
    EtsClass *klass =
        IS_STATIC ? PlatformTypes(coro)->coreReflectStaticField : PlatformTypes(coro)->coreReflectInstanceField;
    ASSERT(klass != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(klass, fields.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    for (size_t idx = 0; idx < fields.size(); ++idx) {
        auto *reflectField = EtsReflectField::CreateFromEtsField(coro, fields[idx]);
        if (UNLIKELY(reflectField == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
        arrayH->Set(idx, reflectField->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

ObjectHeader *StdCoreClassGetInstanceFieldsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(!(cls->IsInterface()));

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    bool collectPublic = FromEtsBoolean(publicOnly);

    PandaVector<EtsField *> instanceFields;
    Span<Field> fields(cls->GetRuntimeClass()->GetInstanceFields());

    for (auto &field : fields) {
        if (!collectPublic || field.IsPublic()) {
            instanceFields.push_back(EtsField::FromRuntimeField(&field));
        }
    }

    return CreateEtsReflectFieldArray(coro, instanceFields);
}

ObjectHeader *StdCoreClassGetStaticFieldsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(!(cls->IsInterface()));

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    bool collectPublic = FromEtsBoolean(publicOnly);

    PandaVector<EtsField *> staticFields;
    Span<Field> fields(cls->GetRuntimeClass()->GetStaticFields());

    for (auto &field : fields) {
        if (!collectPublic || field.IsPublic()) {
            staticFields.push_back(EtsField::FromRuntimeField(&field));
        }
    }

    return CreateEtsReflectFieldArray<true>(coro, staticFields);
}

EtsBoolean StdCoreClassIsEnum(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsEtsEnum();
}

EtsBoolean StdCoreClassIsInterface(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsInterface();
}

EtsBoolean StdCoreClassIsFixedArray(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsArrayClass();
}

EtsBoolean StdCoreClassIsSubtypeOf(EtsClass *cls, EtsClass *other)
{
    if (other->IsInterface()) {
        return static_cast<EtsBoolean>(cls->GetRuntimeClass()->Implements(other->GetRuntimeClass()));
    }
    return static_cast<EtsBoolean>(other->IsAssignableFrom(cls));
}

EtsClass *StdCoreClassGetFixedArrayComponentType(EtsClass *cls)
{
    if (!cls->IsArrayClass()) {
        return nullptr;
    }
    return cls->GetComponentType();
}

EtsBoolean StdCoreClassIsUnion(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsUnionClass();
}

ObjectHeader *StdCoreClassGetUnionConstituentTypesInternal(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    ASSERT(cls->IsUnionClass());

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto constituentTypes = cls->GetRuntimeClass()->GetConstituentTypes();
    auto *typesArray = EtsObjectArray::Create(PlatformTypes(coro)->coreClass, constituentTypes.size());
    if (UNLIKELY(typesArray == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(typesArray != nullptr);

    for (size_t idx = 0; idx < constituentTypes.size(); ++idx) {
        typesArray->Set(idx, reinterpret_cast<EtsObject *>(EtsClass::FromRuntimeClass(constituentTypes[idx])));
    }

    return typesArray->AsObject()->GetCoreType();
}

EtsBoolean StdCoreClassIsFinal(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsFinal() || cls->IsStringClass();
}

EtsBoolean StdCoreClassIsAbstract(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsAbstract();
}

EtsBoolean StdCoreClassIsPrimitive(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->IsPrimitive();
}

}  // namespace ark::ets::intrinsics
