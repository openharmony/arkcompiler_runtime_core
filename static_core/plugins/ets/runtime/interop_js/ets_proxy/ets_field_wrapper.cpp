/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_field_wrapper.h"

#include "libpandafile/file.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/local_object_handle.h"

#include "runtime/mem/vm_handle-inl.h"

namespace panda::ets::interop::js::ets_proxy {

template <bool IS_STATIC>
static EtsObject *EtsAccessorsHandleThis(EtsFieldWrapper *field_wrapper, EtsCoroutine *coro, InteropCtx *ctx,
                                         napi_env env, napi_value js_this)
{
    if constexpr (IS_STATIC) {
        EtsClass *ets_class = field_wrapper->GetOwner()->GetEtsClass();
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, ets_class))) {
            ctx->ForwardEtsException(coro);
            return nullptr;
        }
        return ets_class->AsObject();
    }

    if (UNLIKELY(IsNullOrUndefined(env, js_this))) {
        ctx->ThrowJSTypeError(env, "ets this in set accessor cannot be null or undefined");
        return nullptr;
    }

    EtsObject *ets_this = field_wrapper->GetOwner()->UnwrapEtsProxy(ctx, js_this);
    if (UNLIKELY(ets_this == nullptr)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        return nullptr;
    }
    return ets_this;
}

template <typename FieldAccessor, bool IS_STATIC>
static napi_value EtsFieldGetter(napi_env env, napi_callback_info cinfo)
{
    size_t argc = 0;
    napi_value js_this;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, &js_this, &data));
    if (UNLIKELY(argc != 0)) {
        InteropCtx::ThrowJSError(env, "getter called in wrong context");
        return napi_value {};
    }

    auto ets_field_wrapper = reinterpret_cast<EtsFieldWrapper *>(data);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope scope(ctx, env);
    ScopedManagedCodeThread managed_scope(coro);

    EtsObject *ets_this = EtsAccessorsHandleThis<IS_STATIC>(ets_field_wrapper, coro, ctx, env, js_this);
    if (UNLIKELY(ets_this == nullptr)) {
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    napi_value res = FieldAccessor::Getter(ctx, env, ets_this, ets_field_wrapper);
    ASSERT(res != nullptr || ctx->SanityJSExceptionPending());
    return res;
}

template <typename FieldAccessor, bool IS_STATIC>
static napi_value EtsFieldSetter(napi_env env, napi_callback_info cinfo)
{
    size_t argc = 1;
    napi_value js_value;
    napi_value js_this;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, &js_value, &js_this, &data));
    if (UNLIKELY(argc != 1)) {
        InteropCtx::ThrowJSError(env, "setter called in wrong context");
        return napi_value {};
    }

    auto ets_field_wrapper = reinterpret_cast<EtsFieldWrapper *>(data);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope scope(ctx, env);
    ScopedManagedCodeThread managed_scope(coro);

    EtsObject *ets_this = EtsAccessorsHandleThis<IS_STATIC>(ets_field_wrapper, coro, ctx, env, js_this);
    if (UNLIKELY(ets_this == nullptr)) {
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    LocalObjectHandle<EtsObject> ets_this_handle(coro, ets_this);
    auto res = FieldAccessor::Setter(ctx, env, EtsHandle<EtsObject>(VMHandle<EtsObject>(ets_this_handle)),
                                     ets_field_wrapper, js_value);
    if (UNLIKELY(!res)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
    }
    return nullptr;
}

struct EtsFieldAccessorREFERENCE {
    static napi_value Getter(InteropCtx *ctx, napi_env env, EtsObject *ets_object, EtsFieldWrapper *ets_field_wrapper)
    {
        EtsObject *ets_value = ets_object->GetFieldObject(ets_field_wrapper->GetObjOffset());
        if (ets_value == nullptr) {
            return GetNull(env);
        }
        auto refconv = JSRefConvertResolve(ctx, ets_value->GetClass()->GetRuntimeClass());
        ASSERT(refconv != nullptr);
        return refconv->Wrap(ctx, ets_value);
    }

    static bool Setter(InteropCtx *ctx, napi_env env, EtsHandle<EtsObject> ets_object,
                       EtsFieldWrapper *ets_field_wrapper, napi_value js_value)
    {
        EtsObject *ets_value;
        if (IsNullOrUndefined(env, js_value)) {
            ets_value = nullptr;
        } else {
            JSRefConvert *refconv = ets_field_wrapper->GetRefConvert<true>(ctx);
            if (UNLIKELY(refconv == nullptr)) {
                return false;
            }
            ets_value = refconv->Unwrap(ctx, js_value);
            if (UNLIKELY(ets_value == nullptr)) {
                return false;
            }
        }
        ets_object->SetFieldObject(ets_field_wrapper->GetObjOffset(), ets_value);
        return true;
    }
};

template <typename Convertor>
struct EtsFieldAccessorPRIMITIVE {
    using PrimitiveType = typename Convertor::cpptype;

    static napi_value Getter(InteropCtx * /*ctx*/, napi_env env, EtsObject *ets_object,
                             EtsFieldWrapper *ets_field_wrapper)
    {
        auto ets_value = ets_object->GetFieldPrimitive<PrimitiveType>(ets_field_wrapper->GetObjOffset());
        return Convertor::Wrap(env, ets_value);
    }

    // NOTE(vpukhov): elide ets_object handle
    static bool Setter(InteropCtx *ctx, napi_env env, EtsHandle<EtsObject> ets_object,
                       EtsFieldWrapper *ets_field_wrapper, napi_value js_value)
    {
        std::optional<PrimitiveType> ets_value = Convertor::Unwrap(ctx, env, js_value);
        if (LIKELY(ets_value.has_value())) {
            ets_object->SetFieldPrimitive<PrimitiveType>(ets_field_wrapper->GetObjOffset(), ets_value.value());
        }
        return ets_value.has_value();
    }
};

template <bool ALLOW_INIT>
JSRefConvert *EtsFieldWrapper::GetRefConvert(InteropCtx *ctx)
{
    if (LIKELY(lazy_refconvert_link_.IsResolved())) {
        return lazy_refconvert_link_.GetResolved();
    }

    const Field *field = lazy_refconvert_link_.GetUnresolved();
    ASSERT(field->GetTypeId() == panda_file::Type::TypeId::REFERENCE);

    const auto *panda_file = field->GetPandaFile();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *field_class =
        class_linker->GetClass(*panda_file, panda_file::FieldDataAccessor::GetTypeId(*panda_file, field->GetFileId()),
                               ctx->LinkerCtx(), nullptr);

    JSRefConvert *refconv = JSRefConvertResolve<ALLOW_INIT>(ctx, field_class);
    if (UNLIKELY(refconv == nullptr)) {
        return nullptr;
    }
    lazy_refconvert_link_.Set(refconv);  // Update link
    return refconv;
}

// Explicit instantiation
template JSRefConvert *EtsFieldWrapper::GetRefConvert<false>(InteropCtx *ctx);
template JSRefConvert *EtsFieldWrapper::GetRefConvert<true>(InteropCtx *ctx);

template <bool IS_STATIC>
static napi_property_descriptor DoMakeNapiProperty(EtsFieldWrapper *wrapper)
{
    Field *field = wrapper->GetField();
    napi_property_descriptor prop {};
    prop.utf8name = utf::Mutf8AsCString(field->GetName().data);
    prop.attributes = IS_STATIC ? EtsClassWrapper::STATIC_FIELD_ATTR : EtsClassWrapper::FIELD_ATTR;
    prop.data = wrapper;

    // NOTE(vpukhov): apply the same rule to instance fields?
    ASSERT(!IS_STATIC || wrapper->GetOwner()->GetEtsClass()->GetRuntimeClass() == field->GetClass());

    auto setup_accessors = [&](auto accessor_tag) {
        using Accessor = typename decltype(accessor_tag)::type;
        prop.getter = EtsFieldGetter<Accessor, IS_STATIC>;
        prop.setter = EtsFieldSetter<Accessor, IS_STATIC>;
        return prop;
    };

    panda_file::Type type = field->GetType();
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU1>>());
        case panda_file::Type::TypeId::I8:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI8>>());
        case panda_file::Type::TypeId::U8:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU8>>());
        case panda_file::Type::TypeId::I16:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI16>>());
        case panda_file::Type::TypeId::U16:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU16>>());
        case panda_file::Type::TypeId::I32:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI32>>());
        case panda_file::Type::TypeId::U32:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU32>>());
        case panda_file::Type::TypeId::I64:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI64>>());
        case panda_file::Type::TypeId::U64:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU64>>());
        case panda_file::Type::TypeId::F32:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertF32>>());
        case panda_file::Type::TypeId::F64:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertF64>>());
        case panda_file::Type::TypeId::REFERENCE:
            return setup_accessors(helpers::TypeIdentity<EtsFieldAccessorREFERENCE>());
        default:
            InteropCtx::Fatal(std::string("ConvertEtsVal: unsupported typeid ") +
                              panda_file::Type::GetSignatureByTypeId(type));
    }
    UNREACHABLE();
}

napi_property_descriptor EtsFieldWrapper::MakeInstanceProperty(EtsClassWrapper *owner, Field *field)
{
    new (this) EtsFieldWrapper(owner, field);
    return DoMakeNapiProperty</*IS_STATIC=*/false>(this);
}

napi_property_descriptor EtsFieldWrapper::MakeStaticProperty(EtsClassWrapper *owner, Field *field)
{
    new (this) EtsFieldWrapper(owner, field);
    return DoMakeNapiProperty</*IS_STATIC=*/true>(this);
}

}  // namespace panda::ets::interop::js::ets_proxy
