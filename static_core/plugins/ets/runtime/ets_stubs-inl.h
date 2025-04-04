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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "libpandafile/proto_data_accessor-inl.h"

namespace ark::ets {

static constexpr const char *GETTER_PREFIX = "<get>";
static constexpr const char *SETTER_PREFIX = "<set>";

ALWAYS_INLINE inline bool EtsReferenceNullish(EtsCoroutine *coro, EtsObject *ref)
{
    return ref == nullptr || ref == EtsObject::FromCoreType(coro->GetNullValue());
}

ALWAYS_INLINE inline bool IsReferenceNullish(EtsCoroutine *coro, EtsObject *ref)
{
    return ref == nullptr || ref == EtsObject::FromCoreType(coro->GetNullValue());
}

template <bool IS_STRICT>
ALWAYS_INLINE inline bool EtsReferenceEquals(EtsCoroutine *coro, EtsObject *ref1, EtsObject *ref2)
{
    if (UNLIKELY(ref1 == ref2)) {
        return true;
    }

    if (IsReferenceNullish(coro, ref1) || IsReferenceNullish(coro, ref2)) {
        if constexpr (IS_STRICT) {
            return false;
        } else {
            return IsReferenceNullish(coro, ref1) && IsReferenceNullish(coro, ref2);
        }
    }

    if (LIKELY(!(ref1->GetClass()->IsValueTyped() && ref2->GetClass()->IsValueTyped()))) {
        return false;
    }
    return EtsValueTypedEquals(coro, ref1, ref2);
}

ALWAYS_INLINE inline EtsString *EtsReferenceTypeof(EtsCoroutine *coro, EtsObject *ref)
{
    return EtsGetTypeof(coro, ref);
}

ALWAYS_INLINE inline bool EtsIstrue(EtsCoroutine *coro, EtsObject *obj)
{
    return EtsGetIstrue(coro, obj);
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
inline EtsClass *GetMethodOwnerClassInFrames(EtsCoroutine *coro, uint32_t depth)
{
    auto stack = StackWalker::Create(coro);
    for (uint32_t i = 0; i < depth && stack.HasFrame(); ++i) {
        stack.NextFrame();
    }
    if (UNLIKELY(!stack.HasFrame())) {
        return nullptr;
    }
    auto method = stack.GetMethod();
    if (UNLIKELY(method == nullptr)) {
        return nullptr;
    }
    return EtsClass::FromRuntimeClass(method->GetClass());
}

ALWAYS_INLINE inline void IsValidByType(EtsCoroutine *coro, ark::Class *argClass, Field *metaField, bool isGetter)
{
    auto sourceClass = isGetter ? argClass : metaField->ResolveTypeClass();
    auto targetClass = isGetter ? metaField->ResolveTypeClass() : argClass;
    if (UNLIKELY(!targetClass->IsAssignableFrom(sourceClass))) {
        auto errorMsg = targetClass->GetName() + " cannot be cast to " + sourceClass->GetName();
        ThrowEtsException(coro,
                          utf::Mutf8AsCString(Runtime::GetCurrent()
                                                  ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                  .GetClassCastExceptionClassDescriptor()),
                          errorMsg);
    }
}

ALWAYS_INLINE inline void IsValidAccessorByName(EtsCoroutine *coro, Field *metaField, Method *resolved, bool isGetter)
{
    auto pf = resolved->GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, resolved->GetFileId()));
    auto argClass = classLinker->GetClass(*pf, pda.GetReferenceType(0), resolved->GetClass()->GetLoadContext());
    IsValidByType(coro, argClass, metaField, isGetter);
}

ALWAYS_INLINE inline void IsValidFieldByName(EtsCoroutine *coro, Field *metaField, Field *resolved, bool isGetter)
{
    auto pf = resolved->GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto argClass = classLinker->GetClass(*pf, panda_file::FieldDataAccessor::GetTypeId(*pf, resolved->GetFileId()),
                                          resolved->GetClass()->GetLoadContext());
    IsValidByType(coro, argClass, metaField, isGetter);
}

template <bool IS_GETTER>
inline void LookUpException(ark::Class *klass, Field *rawField)
{
    auto type = IS_GETTER ? "getter" : "setter";
    auto errorMsg = "Class " + ark::ConvertToString(klass->GetName()) + " does not have field and " +
                    ark::ConvertToString(type) + " with name " + utf::Mutf8AsCString(rawField->GetName().data);
    ThrowEtsException(EtsCoroutine::GetCurrent(),
                      panda_file_items::class_descriptors::LINKER_UNRESOLVED_FIELD_ERROR.data(), errorMsg);
}

template <bool IS_GETTER>
ALWAYS_INLINE Field *GetFieldByName(InterpreterCache::Entry *entry, ark::Method *method, Field *rawField,
                                    const uint8_t *address, ark::Class *klass)
{
    auto *res = entry->item;
    auto resUint = reinterpret_cast<uint64_t>(res);
    auto fieldPtr = reinterpret_cast<Field *>(resUint & ~METHOD_FLAG_MASK);
    bool cacheExists = res != nullptr && ((resUint & METHOD_FLAG_MASK) == 1);
    Class *current = klass;
    Field *field = nullptr;
    while (current != nullptr) {
        if (cacheExists && (fieldPtr->GetClass() == current)) {
            return fieldPtr;
        }
        field = LookupFieldByName(rawField->GetName(), current);
        if (field == nullptr) {
            current = current->GetBase();
            continue;
        }
        if (field->GetTypeId() == panda_file::Type::TypeId::REFERENCE) {
            IsValidFieldByName(EtsCoroutine::GetCurrent(), rawField, field, IS_GETTER);
        }
        *entry = {address, method, static_cast<void *>(field)};
        return field;
    }
    return field;
}

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
ALWAYS_INLINE inline ark::Method *GetAccessorByName(InterpreterCache::Entry *entry, ark::Method *method,
                                                    Field *rawField, const uint8_t *address, ark::Class *klass)
{
    auto *res = entry->item;
    auto resUint = reinterpret_cast<uint64_t>(res);
    bool cacheExists = res != nullptr && ((resUint & METHOD_FLAG_MASK) == 1);
    auto methodPtr = reinterpret_cast<Method *>(resUint & ~METHOD_FLAG_MASK);
    ark::Method *callee = nullptr;
    Class *current = klass;
    while (current != nullptr) {
        if (cacheExists && (methodPtr->GetClass() == klass)) {
            return methodPtr;
        }
        if constexpr (IS_GETTER) {
            callee = LookupGetterByName<FIELD_TYPE>(rawField->GetName(), current);
        } else {
            callee = LookupSetterByName<FIELD_TYPE>(rawField->GetName(), current);
        }
        if (callee == nullptr) {
            current = current->GetBase();
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            IsValidAccessorByName(EtsCoroutine::GetCurrent(), rawField, callee, IS_GETTER);
        }
        auto mUint = reinterpret_cast<uint64_t>(callee);
        *entry = {address, method, reinterpret_cast<Method *>(mUint | METHOD_FLAG_MASK)};
        return callee;
    }
    return callee;
}

template <bool IS_GETTER>
ALWAYS_INLINE inline bool CheckAccessorNameMatch(Span<const uint8_t> name, Span<const uint8_t> method)
{
    std::string_view methodStr(utf::Mutf8AsCString(method.data()), method.size());
    std::string_view nameStr(utf::Mutf8AsCString(name.data()), name.size());
    std::string_view prefix = IS_GETTER ? GETTER_PREFIX : SETTER_PREFIX;
    if (methodStr.size() - nameStr.size() != prefix.size() || (methodStr.substr(0, prefix.size()) != prefix)) {
        return false;
    }
    return methodStr.substr(prefix.size()) == nameStr;
}

ALWAYS_INLINE inline Field *LookupFieldByName(panda_file::File::StringData name, const ark::Class *klass)
{
    for (auto &f : klass->GetInstanceFields()) {
        if (name == f.GetName()) {
            return &f;
        }
    }
    return nullptr;
}

template <panda_file::Type::TypeId FIELD_TYPE>
ALWAYS_INLINE inline ark::Method *LookupGetterByName(panda_file::File::StringData name, const ark::Class *klass)
{
    Span<const uint8_t> nameSpan((name.data), utf::Mutf8Size(name.data));
    for (auto &m : klass->GetMethods()) {
        Span<const uint8_t> methodSpan(m.GetName().data, utf::Mutf8Size(m.GetName().data));
        if (!CheckAccessorNameMatch<true>(nameSpan, methodSpan)) {
            continue;
        }
        auto retType = m.GetReturnType();
        if (retType.IsVoid()) {
            continue;
        }
        if (m.GetNumArgs() != 1) {
            continue;
        }
        if (!m.GetArgType(0).IsReference()) {
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            if (retType.IsPrimitive()) {
                continue;
            }
            return &m;
        }

        if (retType.IsReference()) {
            continue;
        }
        if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
            if (retType.GetBitWidth() != coretypes::INT64_BITS) {
                continue;
            }
        } else {
            if (retType.GetBitWidth() > coretypes::INT32_BITS) {
                continue;
            }
        }
        return &m;
    }
    return nullptr;
}

template <panda_file::Type::TypeId FIELD_TYPE>
ALWAYS_INLINE inline ark::Method *LookupSetterByName(panda_file::File::StringData name, const ark::Class *klass)
{
    Span<const uint8_t> nameSpan((name.data), utf::Mutf8Size(name.data));
    for (auto &m : klass->GetMethods()) {
        Span<const uint8_t> methodSpan(m.GetName().data, utf::Mutf8Size(m.GetName().data));
        if (!CheckAccessorNameMatch<false>(nameSpan, methodSpan)) {
            continue;
        }
        if (m.IsStatic() || !m.GetReturnType().IsVoid() || m.GetNumArgs() != 2U || !m.GetArgType(0).IsReference()) {
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            if (m.GetArgType(1).IsPrimitive()) {
                continue;
            }
            return &m;
        }

        auto arg1 = m.GetArgType(1);
        if (arg1.IsReference()) {
            continue;
        }
        if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
            if (arg1.GetBitWidth() != coretypes::INT64_BITS) {
                continue;
            }
        } else {
            if (arg1.GetBitWidth() > coretypes::INT32_BITS) {
                continue;
            }
        }
        return &m;
    }
    return nullptr;
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
