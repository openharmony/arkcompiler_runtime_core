/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H

#include "plugins/ets/runtime/interop_js/call/proto_reader.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"

namespace ark::ets::interop::js {

template <typename FUnwrapVal, typename FStore>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertRefArgToEts(InteropCtx *ctx, ProtoReader &protoReader,
                                                                  FStore &storeRes, napi_value jsVal,
                                                                  FUnwrapVal &unwrapVal)
{
    auto env = ctx->GetJSEnv();

    if (IsNull(env, jsVal)) {
        storeRes(nullptr);
        return true;
    }
    auto klass = protoReader.GetClass();
    // start fastpath
    if (klass == ctx->GetJSValueClass()) {
        return unwrapVal(helpers::TypeIdentity<JSConvertJSValue>());
    }
    if (klass == ctx->GetStringClass()) {
        return unwrapVal(helpers::TypeIdentity<JSConvertString>());
    }
    if (IsUndefined(env, jsVal)) {
        if (UNLIKELY(!klass->IsAssignableFrom(ctx->GetUndefinedClass()))) {
            return false;
        }
        storeRes(ctx->GetUndefinedObject()->GetCoreType());
        return true;
    }
    // start slowpath
    auto refconv = JSRefConvertResolve<true>(ctx, klass);
    if (UNLIKELY(refconv == nullptr)) {
        return false;
    }
    ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
    storeRes(res);
    return res != nullptr;
}

template <typename FStore>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertArgToEts(InteropCtx *ctx, ProtoReader &protoReader,
                                                               FStore &storeRes, napi_value jsVal)
{
    auto env = ctx->GetJSEnv();

    auto unwrapVal = [&ctx, &env, &jsVal, &storeRes](auto convTag) {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        auto res = Convertor::Unwrap(ctx, env, jsVal);
        if (UNLIKELY(!res.has_value())) {
            return false;
        }
        if constexpr (std::is_pointer_v<cpptype>) {
            storeRes(AsEtsObject(res.value())->GetCoreType());
        } else {
            storeRes(Value(res.value()).GetAsLong());
        }
        return true;
    };

    switch (protoReader.GetType().GetId()) {
        case panda_file::Type::TypeId::VOID: {
            return true;  // do nothing
        }
        case panda_file::Type::TypeId::U1:
            return unwrapVal(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return unwrapVal(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return unwrapVal(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return unwrapVal(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return unwrapVal(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return unwrapVal(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return unwrapVal(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return unwrapVal(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return unwrapVal(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return unwrapVal(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return unwrapVal(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE:
            return ConvertRefArgToEts(ctx, protoReader, storeRes, jsVal, unwrapVal);
        default:
            UNREACHABLE();
    }
}

template <typename FRead>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertRefArgToJS(InteropCtx *ctx, napi_value *resSlot, FRead &readVal)
{
    auto env = ctx->GetJSEnv();
    auto setResult = [resSlot](napi_value res) {
        *resSlot = res;
        return res != nullptr;
    };
    auto wrapRef = [&env, setResult](auto convTag, ObjectHeader *ref) -> bool {
        using Convertor = typename decltype(convTag)::type;  // conv_tag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        cpptype value = std::remove_pointer_t<cpptype>::FromEtsObject(EtsObject::FromCoreType(ref));
        return setResult(Convertor::Wrap(env, value));
    };

    ObjectHeader *ref = readVal(helpers::TypeIdentity<ObjectHeader *>());
    if (UNLIKELY(ref == nullptr)) {
        *resSlot = GetNull(env);
        return true;
    }
    if (UNLIKELY(ref == ctx->GetUndefinedObject()->GetCoreType())) {
        *resSlot = GetUndefined(env);
        return true;
    }

    auto klass = ref->template ClassAddr<Class>();
    // start fastpath
    if (klass == ctx->GetJSValueClass()) {
        return wrapRef(helpers::TypeIdentity<JSConvertJSValue>(), ref);
    }
    if (klass == ctx->GetStringClass()) {
        return wrapRef(helpers::TypeIdentity<JSConvertString>(), ref);
    }
    // start slowpath
    auto refconv = JSRefConvertResolve(ctx, klass);
    return setResult(refconv->Wrap(ctx, EtsObject::FromCoreType(ref)));
}

template <typename FRead>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertArgToJS(InteropCtx *ctx, ProtoReader &protoReader,
                                                              napi_value *resSlot, FRead &readVal)
{
    auto env = ctx->GetJSEnv();

    auto wrapPrim = [&env, &readVal, resSlot](auto convTag) -> bool {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        napi_value res = Convertor::Wrap(env, readVal(helpers::TypeIdentity<cpptype>()));
        *resSlot = res;
        return res != nullptr;
    };

    switch (protoReader.GetType().GetId()) {
        case panda_file::Type::TypeId::VOID: {
            *resSlot = GetUndefined(env);
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return wrapPrim(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return wrapPrim(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return wrapPrim(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return wrapPrim(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return wrapPrim(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return wrapPrim(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return wrapPrim(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return wrapPrim(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return wrapPrim(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return wrapPrim(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return wrapPrim(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE:
            return ConvertRefArgToJS(ctx, resSlot, readVal);
        default:
            UNREACHABLE();
    }
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H
