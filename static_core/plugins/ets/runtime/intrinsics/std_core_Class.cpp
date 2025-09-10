/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "libpandabase/utils/logger.h"
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
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/handle_scope-inl.h"
#include "libpandabase/utils/utf.h"

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

EtsBoolean StdCoreClassIsNamespace(EtsClass *cls)
{
    return cls->IsModule();
}

EtsRuntimeLinker *StdCoreClassGetLinker(EtsClass *cls)
{
    return EtsClassLinkerExtension::GetOrCreateEtsRuntimeLinker(cls->GetLoadContext());
}

EtsClass *StdCoreClassOf(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    auto *cls = obj->GetClass();
    return cls->ResolvePublicClass();
}

EtsClass *StdCoreClassOfNull()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    const auto *nullObject = EtsObject::FromCoreType(coro->GetNullValue());
    return nullObject->GetClass();
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
    const auto name = clsName->GetMutf8();
    PandaString descriptor;
    const auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);
    return runtimeLinker->FindLoadedClass(classDescriptor);
}

void EtsRuntimeLinkerInitializeContext(EtsRuntimeLinker *runtimeLinker)
{
    auto *ext = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension();
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
    const auto name = clsName->GetMutf8();
    PandaString descriptor;
    auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);

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
                return reinterpret_cast<EtsClassLinkerContext *>(ctx)->GetRuntimeLinker();
            }
        }
    }
    return nullptr;
}

static EtsReflectMethod *CreateEtsReflectMethodUnderHandleScope(EtsCoroutine *coro, EtsMethod *method)
{
    ASSERT(coro != nullptr);
    ASSERT(method != nullptr);

    auto reflectMethod = EtsHandle<EtsReflectMethod>(coro, EtsReflectMethod::Create(coro));
    if (UNLIKELY(reflectMethod.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(reflectMethod.GetPtr() != nullptr);

    auto *ownerType = method->GetClass();
    reflectMethod->SetOwnerType(ownerType);
    reflectMethod->SetEtsMethod(reinterpret_cast<EtsLong>(method));

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (method->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (method->IsConstructor()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::CONSTRUCTOR) : 0U;
    attr |= (method->IsAbstract()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ABSTRACT) : 0U;
    attr |= (method->IsGetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::GETTER) : 0U;
    attr |= (method->IsSetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::SETTER) : 0U;
    attr |= (method->IsFinal()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::FINAL) : 0U;
    panda_file::File::EntityId asyncAnnId = EtsAnnotation::FindAsyncAnnotation(method->GetPandaMethod());
    attr |= (method->IsNative() && !asyncAnnId.IsValid()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::NATIVE) : 0U;
    attr |= (asyncAnnId.IsValid()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ASYNC) : 0U;

    reflectMethod->SetAttributes(attr);

    int8_t accessMod = method->IsPublic()
                           ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PUBLIC)
                           : (method->IsProtected() ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PROTECTED)
                                                    : static_cast<int8_t>(EtsTypeAPIAccessModifier::PRIVATE));

    reflectMethod->SetAccessModifier(accessMod);

    return reflectMethod.GetPtr();
}

template <typename Func>
static void GetInterfaceMethodsRecursive(Class *currIntf, Func &&collectDirectMethods)
{
    ASSERT(currIntf != nullptr);

    collectDirectMethods(currIntf);

    for (auto *intf : currIntf->GetInterfaces()) {
        ASSERT(intf != nullptr);
        GetInterfaceMethodsRecursive(intf, std::forward<Func>(collectDirectMethods));
    }
}

static PandaVector<EtsMethod *> GetInstanceMethodsForInterface(EtsClass *cls, bool onlyPublic)
{
    ASSERT(cls->IsInterface());

    PandaUnorderedMap<PandaString, EtsMethod *> uniqNames;
    PandaVector<EtsMethod *> instanceMethods;

    auto collectDirectMethods = [&uniqNames, &onlyPublic](Class *intfClass) {
        for (auto &method : intfClass->GetMethods()) {
            if ((onlyPublic && !method.IsPublic())) {
                continue;
            }
            // Must be fixed: #30139
            PandaString methodName = utf::Mutf8AsCString(method.GetName().data);
            if (auto methodIt = uniqNames.find(methodName); methodIt == uniqNames.end()) {
                uniqNames.emplace(methodName, EtsMethod::FromRuntimeMethod(&method));
            }
        }
    };

    GetInterfaceMethodsRecursive(cls->GetRuntimeClass(), std::move(collectDirectMethods));

    instanceMethods.reserve(uniqNames.size());
    for (auto &[name, method] : uniqNames) {
        instanceMethods.push_back(method);
    }
    return instanceMethods;
}

static PandaVector<EtsMethod *> GetInstanceMethodsForClass(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsMethod *> instanceMethods;
    instanceMethods.reserve(cls->GetRuntimeClass()->GetVTableSize());

    cls->EnumerateVtable([&instanceMethods, onlyPublic](Method *method) {
        if (!method->IsConstructor() && ((onlyPublic && method->IsPublic()) || !onlyPublic)) {
            instanceMethods.push_back(EtsMethod::FromRuntimeMethod(method));
        }
        return false;
    });

    return instanceMethods;
}

static PandaVector<EtsMethod *> GetInstanceMethods(EtsClass *cls, bool onlyPublic)
{
    if (cls->IsInterface()) {
        return GetInstanceMethodsForInterface(cls, onlyPublic);
    }
    return GetInstanceMethodsForClass(cls, onlyPublic);
}

static ObjectHeader *CreateEtsReflectMethodArray(EtsCoroutine *coro, const PandaVector<EtsMethod *> &methods,
                                                 bool isStatic = false, bool isConstructor = false)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsClass *klass = isConstructor ? PlatformTypes(coro)->reflectConstructor
                                    : (isStatic ? PlatformTypes(coro)->reflectStaticMethod
                                                : PlatformTypes(coro)->reflectInstanceMethod);

    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(klass, methods.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(arrayH.GetPtr() != nullptr);

    for (size_t idx = 0; idx < methods.size(); ++idx) {
        auto *reflectMethod = CreateEtsReflectMethodUnderHandleScope(coro, methods[idx]);
        arrayH->Set(idx, reflectMethod->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

ObjectHeader *StdCoreClassGetInstanceMethodsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto instanceMethods = GetInstanceMethods(cls, publicOnly != 0U);

    return CreateEtsReflectMethodArray(coro, instanceMethods);
}

EtsReflectMethod *StdCoreClassGetInstanceMethodByNameInternal(EtsClass *cls, EtsString *name, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    // Hadle case with overloads in case of spec issue#362
    EtsMethod *method = cls->GetInstanceMethod(name->GetUtf8().c_str(), nullptr);
    if (method == nullptr) {
        return nullptr;
    }
    if ((publicOnly != 0U) && !method->IsPublic()) {
        return nullptr;
    }

    return CreateEtsReflectMethodUnderHandleScope(coro, method);
}

static EtsReflectField *CreateEtsReflectFieldUnderHandleScope(EtsCoroutine *coro, EtsField *field)
{
    ASSERT(field != nullptr);

    auto reflectField = EtsHandle<EtsReflectField>(coro, EtsReflectField::Create(coro, field->IsStatic()));
    if (UNLIKELY(reflectField.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(reflectField.GetPtr() != nullptr);

    // do not expose primitive types and string types except std.sore.String
    auto *resolvedType = field->GetType()->ResolvePublicClass();
    reflectField->SetFieldType(resolvedType);
    reflectField->SetEtsField(reinterpret_cast<EtsLong>(field));
    reflectField->SetOwnerType(field->GetDeclaringClass());

    uint32_t attr = 0;
    attr |= (field->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (field->IsReadonly()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::READONLY) : 0U;

    reflectField->SetAttributes(attr);

    int8_t accessMod = field->IsPublic()
                           ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PUBLIC)
                           : (field->IsProtected() ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PROTECTED)
                                                   : static_cast<int8_t>(EtsTypeAPIAccessModifier::PRIVATE));
    reflectField->SetAccessModifier(accessMod);

    return reflectField.GetPtr();
}

EtsReflectField *StdCoreClassGetInstanceFieldByNameInternal(EtsClass *cls, EtsString *name, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsField *field = cls->GetFieldIDByName(name->GetUtf8().c_str(), nullptr);
    if (field == nullptr) {
        return nullptr;
    }
    if ((publicOnly != 0U) && !field->IsPublic()) {
        return nullptr;
    }
    return CreateEtsReflectFieldUnderHandleScope(coro, field);
}

EtsReflectField *StdCoreClassGetStaticFieldByNameInternal(EtsClass *cls, EtsString *name, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsField *field = cls->GetStaticFieldIDByName(name->GetUtf8().data(), nullptr);
    if (field == nullptr) {
        return nullptr;
    }
    if (publicOnly != 0U && !field->IsPublic()) {
        return nullptr;
    }
    return CreateEtsReflectFieldUnderHandleScope(coro, field);
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

    auto constructors = GetConstructors(cls, publicOnly != 0U);

    return CreateEtsReflectMethodArray(coro, constructors, false, true);
}

static PandaVector<EtsMethod *> GetStaticMethods(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsMethod *> staticMethods;
    PandaUnorderedSet<PandaString> uniqNames;

    EnumerateBaseClassesReverseOrder(cls, [&](EtsClass *c) {
        auto methods = c->GetRuntimeClass()->GetStaticMethods();
        auto mnum = methods.Size();
        for (uint32_t i = 0; i < mnum; i++) {
            if (onlyPublic && !methods[i].IsPublic()) {
                continue;
            }
            PandaString methodName = methods[i].GetFullName(true);
            if (!uniqNames.count(methodName)) {
                staticMethods.push_back(EtsMethod::FromRuntimeMethod(&methods[i]));
                uniqNames.emplace(methodName);
            }
        }
        return false;
    });
    return staticMethods;
}

ObjectHeader *StdCoreClassGetStaticMethodsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto staticMethods = GetStaticMethods(cls, publicOnly != 0U);

    return CreateEtsReflectMethodArray(coro, staticMethods, true, false);
}

static PandaVector<EtsField *> GetInstanceFields(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsField *> instanceFields;
    PandaUnorderedSet<PandaString> uniqNames;

    EnumerateBaseClassesReverseOrder(cls, [&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetInstanceFields();
        auto fnum = fields.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            if (onlyPublic && !fields[i].IsPublic()) {
                continue;
            }
            PandaString fieldName = utf::Mutf8AsCString(fields[i].GetName().data);
            if (!uniqNames.count(fieldName)) {
                instanceFields.push_back(EtsField::FromRuntimeField(&fields[i]));
                uniqNames.emplace(fieldName);
            }
        }
        return false;
    });
    return instanceFields;
}

static PandaVector<EtsField *> GetStaticFields(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsField *> staticFields;
    PandaUnorderedSet<PandaString> uniqNames;

    EnumerateBaseClassesReverseOrder(cls, [&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetStaticFields();
        auto fnum = fields.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            if (onlyPublic && !fields[i].IsPublic()) {
                continue;
            }
            PandaString fieldName = utf::Mutf8AsCString(fields[i].GetName().data);
            if (!uniqNames.count(fieldName)) {
                staticFields.push_back(EtsField::FromRuntimeField(&fields[i]));
                uniqNames.emplace(fieldName);
            }
        }
        return false;
    });
    return staticFields;
}

static ObjectHeader *CreateEtsReflectFieldArray(EtsCoroutine *coro, const PandaVector<EtsField *> &fields,
                                                bool isStatic = false)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsClass *klass = isStatic ? PlatformTypes(coro)->reflectStaticField : PlatformTypes(coro)->reflectInstanceField;

    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(klass, fields.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(arrayH.GetPtr() != nullptr);

    for (size_t idx = 0; idx < fields.size(); ++idx) {
        auto *reflectField = CreateEtsReflectFieldUnderHandleScope(coro, fields[idx]);
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

    auto instanceFields = GetInstanceFields(cls, publicOnly != 0U);

    return CreateEtsReflectFieldArray(coro, instanceFields);
}

ObjectHeader *StdCoreClassGetStaticFieldsInternal(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(!(cls->IsInterface()));

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto staticFields = GetStaticFields(cls, publicOnly != 0U);

    return CreateEtsReflectFieldArray(coro, staticFields, true);
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
    if (LIKELY(other->IsInterface())) {
        return static_cast<EtsBoolean>(cls->GetRuntimeClass()->Implements(other->GetRuntimeClass()));
    }
    return static_cast<EtsBoolean>(cls->IsSubClass(other));
}

namespace {
EtsClass *ConvertPandaTypeIdToEtsClass(panda_file::Type::TypeId typeId)
{
    switch (typeId) {
        case ark::panda_file::Type::TypeId::I32:
            return PlatformTypes()->coreInt;
        case ark::panda_file::Type::TypeId::I64:
            return PlatformTypes()->coreLong;
        case ark::panda_file::Type::TypeId::F64:
            return PlatformTypes()->coreDouble;
        case ark::panda_file::Type::TypeId::F32:
            return PlatformTypes()->coreFloat;
        case ark::panda_file::Type::TypeId::I8:
            return PlatformTypes()->coreByte;
        case ark::panda_file::Type::TypeId::I16:
            return PlatformTypes()->coreShort;
        case ark::panda_file::Type::TypeId::U16:
            return PlatformTypes()->coreChar;
        case ark::panda_file::Type::TypeId::U1:
            return PlatformTypes()->coreBoolean;
        default:
            UNREACHABLE();
    }
}
}  // namespace

EtsClass *StdCoreClassGetFixedArrayComponentType(EtsClass *cls)
{
    if (!cls->IsArrayClass()) {
        return nullptr;
    }

    EtsClass *compCls = cls->GetComponentType();
    if (compCls->IsPrimitive()) {
        auto *rtCls = compCls->GetRuntimeClass();
        return ConvertPandaTypeIdToEtsClass(rtCls->GetType().GetId());
    }

    return compCls;
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

}  // namespace ark::ets::intrinsics
