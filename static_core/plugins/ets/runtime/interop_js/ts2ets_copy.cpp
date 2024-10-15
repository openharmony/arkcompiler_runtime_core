/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ts2ets_copy.h"
#include <chrono>
#include <string_view>

#include "ets_coroutine.h"
#include "interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/ets_type_visitor-inl.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/handle_scope-inl.h"

namespace ark::ets::interop::js {

static inline EtsConvertorRef::ObjRoot ToObjRoot(uintptr_t ptr)
{
    return reinterpret_cast<EtsConvertorRef::ObjRoot>(ptr);
}

class JsToEtsConvertor final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

public:
    JsToEtsConvertor(napi_env env, napi_value jsValue, EtsConvertorRef::ValVariant *dataPtr)
        : loc_(dataPtr), env_(env), jsValue_(jsValue)
    {
    }
    JsToEtsConvertor(napi_env env, napi_value jsValue, EtsConvertorRef::ObjRoot etsObj, size_t etsOffs)
        : loc_(etsObj, etsOffs), env_(env), jsValue_(jsValue)
    {
    }

    void VisitU1() override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_boolean, "bool expected");
        bool val;
        TYPEVIS_NAPI_CHECK(napi_get_value_bool(env_, jsValue_, &val));
        loc_.StorePrimitive(static_cast<bool>(val));
    }

    template <typename T>
    inline void VisitIntType()
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_number, "number expected");
        int64_t val;
        TYPEVIS_NAPI_CHECK(napi_get_value_int64(env_, jsValue_, &val));
        loc_.StorePrimitive(static_cast<T>(val));
    }

    void VisitI8() override
    {
        VisitIntType<int8_t>();
    }

    void VisitI16() override
    {
        VisitIntType<int16_t>();
    }

    void VisitU16() override
    {
        VisitIntType<uint16_t>();
    }

    void VisitI32() override
    {
        VisitIntType<int32_t>();
    }

    void VisitI64() override
    {
        VisitIntType<int64_t>();
    }

    void VisitF32() override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_number, "number expected");
        double val;
        TYPEVIS_NAPI_CHECK(napi_get_value_double(env_, jsValue_, &val));
        loc_.StorePrimitive(static_cast<float>(val));
    }

    void VisitF64() override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_number, "number expected");
        double val;
        TYPEVIS_NAPI_CHECK(napi_get_value_double(env_, jsValue_, &val));
        loc_.StorePrimitive(val);
    }

    void VisitString([[maybe_unused]] ark::Class *klass) override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_string, "string expected");

        size_t length;
        TYPEVIS_NAPI_CHECK(napi_get_value_string_utf8(env_, jsValue_, nullptr, 0, &length));

        std::string value;
        value.resize(length + 1);
        size_t result;
        TYPEVIS_NAPI_CHECK(napi_get_value_string_utf8(env_, jsValue_, &value[0], value.size(), &result));

        auto coro = EtsCoroutine::GetCurrent();
        auto vm = coro->GetVM();
        auto str = ark::coretypes::String::CreateFromMUtf8(reinterpret_cast<uint8_t const *>(value.data()),
                                                           vm->GetLanguageContext(), vm);
        loc_.StoreReference(ToObjRoot(ark::VMHandle<ark::ObjectHeader>(coro, str).GetAddress()));
    }

    void VisitArray(ark::Class *klass) override
    {
        {
            bool isArray;
            TYPEVIS_NAPI_CHECK(napi_is_array(env_, jsValue_, &isArray));
            TYPEVIS_CHECK_ERROR(isArray, "array expected");
        }
        auto coro = EtsCoroutine::GetCurrent();

        NapiScope jsHandleScope(env_);

        uint32_t len;
        TYPEVIS_NAPI_CHECK(napi_get_array_length(env_, jsValue_, &len));

        ark::VMHandle<ark::ObjectHeader> etsArr(coro, ark::coretypes::Array::Create(klass, len));

        ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);
        auto *subType = klass->GetComponentType();
        size_t elemSz = ark::Class::GetTypeSize(subType->GetType());
        constexpr auto ELEMS_OFFS = ark::coretypes::Array::GetDataOffset();
        for (size_t idx = 0; idx < len; ++idx) {
            napi_value jsElem;
            TYPEVIS_NAPI_CHECK(napi_get_element(env_, jsValue_, idx, &jsElem));
            JsToEtsConvertor subVis(env_, jsElem, ToObjRoot(etsArr.GetAddress()), ELEMS_OFFS + elemSz * idx);
            subVis.VisitClass(subType);
            TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
        }
        loc_.StoreReference(ToObjRoot(etsArr.GetAddress()));
    }

    void VisitFieldReference(const ark::Field *field, ark::Class *klass) override
    {
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_value subVal;
        TYPEVIS_NAPI_CHECK(napi_get_named_property(env_, jsValue_, fname, &subVal));
        auto subVis = JsToEtsConvertor(env_, subVal, owner_, field->GetOffset());
        subVis.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
    }

    void VisitFieldPrimitive(const ark::Field *field, ark::panda_file::Type type) override
    {
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_value subVal;
        TYPEVIS_NAPI_CHECK(napi_get_named_property(env_, jsValue_, fname, &subVal));
        auto subVis = JsToEtsConvertor(env_, subVal, owner_, field->GetOffset());
        subVis.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
    }

    void VisitObject(ark::Class *klass) override
    {
        ASSERT(klass != InteropCtx::Current()->GetJSValueClass());
        TYPEVIS_CHECK_ERROR(GetValueType(env_, jsValue_) == napi_object, "object expected");
        auto coro = EtsCoroutine::GetCurrent();

        NapiScope jsHandleScope(env_);
        {
            auto etsObj = ark::ObjectHeader::Create(coro, klass);
            owner_ = ToObjRoot(ark::VMHandle<ark::ObjectHeader>(coro, etsObj).GetAddress());
        }

        ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);
        Base::VisitObject(klass);
        TYPEVIS_ABRUPT_ON_ERROR();
        loc_.StoreReference(owner_);
    }

private:
    EtsConvertorRef loc_;
    EtsConvertorRef::ObjRoot owner_ = nullptr;

    napi_env env_ {};
    napi_value jsValue_ {};
};

class EtsToJsConvertor final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

public:
    EtsToJsConvertor(napi_env env, EtsConvertorRef::ValVariant *dataPtr) : loc_(dataPtr), env_(env) {}
    EtsToJsConvertor(napi_env env, EtsConvertorRef::ObjRoot etsObj, size_t etsOffs) : loc_(etsObj, etsOffs), env_(env)
    {
    }

    void VisitU1() override
    {
        bool etsVal = loc_.LoadPrimitive<bool>();
        TYPEVIS_NAPI_CHECK(napi_get_boolean(env_, etsVal, &jsValue_));
    }

    template <typename T>
    inline void VisitIntType()
    {
        auto etsVal = static_cast<int64_t>(loc_.LoadPrimitive<T>());
        TYPEVIS_NAPI_CHECK(napi_create_int64(env_, etsVal, &jsValue_));
    }

    void VisitI8() override
    {
        VisitIntType<int8_t>();
    }

    void VisitI16() override
    {
        VisitIntType<int16_t>();
    }

    void VisitU16() override
    {
        VisitIntType<uint16_t>();
    }

    void VisitI32() override
    {
        VisitIntType<int32_t>();
    }

    void VisitI64() override
    {
        VisitIntType<int64_t>();
    }

    void VisitF32() override
    {
        auto etsVal = static_cast<double>(loc_.LoadPrimitive<float>());
        TYPEVIS_NAPI_CHECK(napi_create_double(env_, etsVal, &jsValue_));
    }

    void VisitF64() override
    {
        auto etsVal = loc_.LoadPrimitive<double>();
        TYPEVIS_NAPI_CHECK(napi_create_double(env_, etsVal, &jsValue_));
    }

    void VisitString([[maybe_unused]] ark::Class *klass) override
    {
        auto etsStr = static_cast<ark::coretypes::String *>(loc_.LoadReference());
        TYPEVIS_NAPI_CHECK(napi_create_string_utf8(env_, reinterpret_cast<const char *>(etsStr->GetDataMUtf8()),
                                                   etsStr->GetMUtf8Length() - 1, &jsValue_));
    }

    void VisitArray(ark::Class *klass) override
    {
        auto coro = EtsCoroutine::GetCurrent();

        ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);
        auto etsArr = ark::VMHandle<ark::coretypes::Array>(coro, loc_.LoadReference());
        auto len = etsArr->GetLength();

        TYPEVIS_NAPI_CHECK(napi_create_array_with_length(env_, len, &jsValue_));

        NapiScope jsHandleScope(env_);
        auto *subType = klass->GetComponentType();
        size_t elemSz = ark::Class::GetTypeSize(subType->GetType());
        auto constexpr ELEMS_OFFS = ark::coretypes::Array::GetDataOffset();
        for (size_t idx = 0; idx < len; ++idx) {
            EtsToJsConvertor subVis(env_, ToObjRoot(etsArr.GetAddress()), ELEMS_OFFS + elemSz * idx);
            subVis.VisitClass(subType);
            TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
            TYPEVIS_NAPI_CHECK(napi_set_element(env_, jsValue_, idx, subVis.jsValue_));
        }
    }

    void VisitFieldReference(const ark::Field *field, ark::Class *klass) override
    {
        auto subVis = EtsToJsConvertor(env_, owner_, field->GetOffset());
        subVis.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_set_named_property(env_, jsValue_, fname, subVis.jsValue_);
    }

    void VisitFieldPrimitive(const ark::Field *field, ark::panda_file::Type type) override
    {
        auto subVis = EtsToJsConvertor(env_, owner_, field->GetOffset());
        subVis.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(subVis.Error());
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_set_named_property(env_, jsValue_, fname, subVis.jsValue_);
    }

    void VisitObject(ark::Class *klass) override
    {
        ASSERT(klass != InteropCtx::Current()->GetJSValueClass());
        auto coro = EtsCoroutine::GetCurrent();
        if (klass == InteropCtx::Current(coro)->GetPromiseClass()) {
            VisitPromise();
            return;
        }

        ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);
        {
            auto etsObj = loc_.LoadReference();
            owner_ = ToObjRoot(ark::VMHandle<ark::ObjectHeader>(coro, etsObj).GetAddress());
        }

        TYPEVIS_NAPI_CHECK(napi_create_object(env_, &jsValue_));

        NapiScope jsHandleScope(env_);
        Base::VisitObject(klass);
    }

    void VisitPromise()
    {
        napi_deferred deferred;
        TYPEVIS_NAPI_CHECK(napi_create_promise(env_, &deferred, &jsValue_));
        auto *coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> handleScope(coro);
        VMHandle<EtsPromise> promise(coro, reinterpret_cast<EtsPromise *>(loc_.LoadReference()));
        if (promise->GetState() != EtsPromise::STATE_PENDING) {
            VMHandle<EtsObject> value(coro, promise->GetValue(coro)->GetCoreType());
            napi_value completionValue;
            if (value.GetPtr() == nullptr) {
                napi_get_null(env_, &completionValue);
            } else {
                EtsConvertorRef::ObjRoot v = ToObjRoot(VMHandle<ObjectHeader>(coro, value->GetCoreType()).GetAddress());
                EtsConvertorRef::ValVariant valVar(v);
                EtsToJsConvertor valueConv(env_, &valVar);
                valueConv.VisitReference(value->GetClass()->GetRuntimeClass());
                TYPEVIS_CHECK_FORWARD_ERROR(valueConv.Error());
                completionValue = valueConv.GetResult();
            }
            if (promise->IsResolved()) {
                TYPEVIS_NAPI_CHECK(napi_resolve_deferred(env_, deferred, completionValue));
            } else {
                TYPEVIS_NAPI_CHECK(napi_reject_deferred(env_, deferred, completionValue));
            }
        } else {
            UNREACHABLE();
        }
    }

    napi_value GetResult()
    {
        return jsValue_;
    }

private:
    EtsConvertorRef loc_;
    EtsConvertorRef::ObjRoot owner_ {};

    napi_env env_ {};
    napi_value jsValue_ {};
};

class JsToEtsArgsConvertor final : public EtsMethodVisitor {
public:
    JsToEtsArgsConvertor(ark::Method *method, napi_env env, napi_value *args, uint32_t numArgs, uint32_t argsStart)
        : EtsMethodVisitor(method), env_(env), jsargs_(args), numArgs_(numArgs), argsStart_(argsStart)
    {
        ASSERT(numArgs_ == method->GetNumArgs());
        etsargs_.resize(numArgs_);
    }

    std::vector<ark::Value> GetResult()
    {
        std::vector<ark::Value> res;
        for (auto const &e : etsargs_) {
            if (std::holds_alternative<ark::Value>(e)) {
                res.emplace_back(std::get<ark::Value>(e));
            } else {
                res.emplace_back(*std::get<EtsConvertorRef::ObjRoot>(e));
            }
        }
        return res;
    }

    void Process()
    {
        VisitMethod();
    }

private:
    void VisitReturn([[maybe_unused]] ark::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitReturn([[maybe_unused]] ark::Class *klass) override
    {
        // do nothing
    }
    void VisitArgument(uint32_t idx, ark::panda_file::Type type) override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        JsToEtsConvertor conv(env_, jsargs_[idx + argsStart_], &etsargs_[idx]);
        conv.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
    }
    void VisitArgument(uint32_t idx, ark::Class *klass) override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        JsToEtsConvertor conv(env_, jsargs_[idx + argsStart_], &etsargs_[idx]);
        conv.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
    }

    napi_env env_;
    napi_value *jsargs_;
    uint32_t numArgs_;
    uint32_t argsStart_;
    std::vector<EtsConvertorRef::ValVariant> etsargs_;
};

class EtsArgsClassesCollector final : protected EtsMethodVisitor {
public:
    explicit EtsArgsClassesCollector(ark::Method *method) : EtsMethodVisitor(method) {}

    void GetResult(std::unordered_set<ark::Class *> &to)
    {
        VisitMethod();
        to = std::move(set_);
    }

private:
    void VisitReturn([[maybe_unused]] ark::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitReturn(ark::Class *klass) override
    {
        AddClass(klass);
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] ark::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, ark::Class *klass) override
    {
        AddClass(klass);
    }

    void AddClass(ark::Class *klass)
    {
        set_.insert(klass);
    }

    std::unordered_set<ark::Class *> set_;
};

class EtsClassesRecursionChecker final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

    struct DFSData {
        explicit DFSData(ark::Class *cls) : klass(cls) {}

        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        ark::Class *klass = nullptr;

        enum : uint8_t {
            MARK_W = 0,
            MARK_G = 1,
            MARK_B = 2,
        } mark = MARK_W;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    static bool CheckClasses(ark::Method *method)
    {
        std::unordered_set<ark::Class *> classSet;
        EtsArgsClassesCollector collector(method);
        collector.GetResult(classSet);

        std::unordered_map<ark::Class *, DFSData> dfsData;
        for (auto const &e : classSet) {
            dfsData.insert(std::make_pair(e, DFSData(e)));
        }

        bool hasLoops = false;
        EtsClassesRecursionChecker checker(hasLoops, dfsData);
        for (auto const &e : dfsData) {
            checker.VisitClass(e.second.klass);
            if (hasLoops) {
                break;
            }
        }
        return !hasLoops;
    }

private:
    EtsClassesRecursionChecker(bool &loopFound, std::unordered_map<ark::Class *, DFSData> &dfs)
        : loopFound_(loopFound), dfs_(dfs)
    {
    }

    void VisitU1() override {}
    void VisitI8() override {}
    void VisitU16() override {}
    void VisitI16() override {}
    void VisitI32() override {}
    void VisitI64() override {}
    void VisitF32() override {}
    void VisitF64() override {}

    void VisitPrimitive([[maybe_unused]] const ark::panda_file::Type type) override {}

    void VisitString([[maybe_unused]] ark::Class *klass) override {}

    void VisitArray(ark::Class *klass) override
    {
        VisitClass(klass->GetComponentType());
    }

    void VisitFieldPrimitive([[maybe_unused]] const ark::Field *field,
                             [[maybe_unused]] ark::panda_file::Type type) override
    {
    }

    void VisitFieldReference([[maybe_unused]] const ark::Field *field, ark::Class *klass) override
    {
        VisitReference(klass);
    }

    void VisitReference(ark::Class *klass) override
    {
        auto str = klass->GetName();
        auto &data = Lookup(klass);
        if (data.mark == DFSData::MARK_B) {
            return;
        }
        if (data.mark == DFSData::MARK_G) {
            loopFound_ = true;
            return;
        }
        data.mark = DFSData::MARK_G;
        Base::VisitReference(klass);
        data.mark = DFSData::MARK_B;
    }

    DFSData &Lookup(ark::Class *klass)
    {
        auto it = dfs_.find(klass);
        if (it == dfs_.end()) {
            auto x = dfs_.insert(std::make_pair(klass, DFSData(klass)));
            return x.first->second;
        }
        return it->second;
    }

    bool &loopFound_;
    std::unordered_map<ark::Class *, DFSData> &dfs_;
};

class EtsToJsRetConvertor final : public EtsMethodVisitor {
public:
    EtsToJsRetConvertor(ark::Method *method, napi_env env, ark::Value ret) : EtsMethodVisitor(method), env_(env)
    {
        if (ret.IsReference()) {
            auto coro = EtsCoroutine::GetCurrent();
            ret_ = ToObjRoot(ark::VMHandle<ark::ObjectHeader>(coro, *ret.GetGCRoot()).GetAddress());
        } else {
            ret_ = ret;
        }
    }

    void Process()
    {
        VisitMethod();
    }

    napi_value GetResult()
    {
        return jsRet_;
    }

private:
    void VisitReturn(ark::panda_file::Type type) override
    {
        if (type.GetId() == panda_file::Type::TypeId::VOID) {
            // Skip 'void' type because it doesn't require conversion
            return;
        }

        EtsToJsConvertor conv(env_, &ret_);
        conv.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
        jsRet_ = conv.GetResult();
    }
    void VisitReturn(ark::Class *klass) override
    {
        EtsToJsConvertor conv(env_, &ret_);
        conv.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
        jsRet_ = conv.GetResult();
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] ark::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] ark::Class *klass) override
    {
        // do nothing
    }

    napi_env env_ {};
    EtsConvertorRef::ValVariant ret_;
    napi_value jsRet_ {};
};

void ThrowProperError(napi_env env, EtsCoroutine *coro)
{
    NapiScope jsHandleScope(env);
    ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);

    auto exc = coro->GetException();
    auto klass = exc->ClassAddr<ark::Class>();
    auto data = EtsConvertorRef::ValVariant(ToObjRoot(ark::VMHandle<ark::ObjectHeader>(coro, exc).GetAddress()));
    coro->ClearException();

    EtsToJsConvertor ets2js(env, &data);
    ets2js.VisitClass(klass);
    if (ets2js.Error()) {
        InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: ets2js error while converting pending exception: " +
                                                  ets2js.Error().value()));
        return;
    }
    InteropCtx::ThrowJSValue(env, ets2js.GetResult());
}

static void GetConverterResult(std::string_view converterType, std::chrono::steady_clock::time_point begin)
{
    auto end = std::chrono::steady_clock::now();
    int64_t t = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    INTEROP_LOG(INFO) << "InvokeEtsMethod: " << converterType << "elapsed time: " << t << "us";
}

static napi_value GetJsRef(napi_env env, const ark::Value &etsRes, Method *method)
{
    NapiEscapableScope jsHandleScope(env);
    ark::HandleScope<ark::ObjectHeader *> etsHandleScope(EtsCoroutine::GetCurrent());

    auto begin = std::chrono::steady_clock::now();
    EtsToJsRetConvertor ets2js(method, env, etsRes);
    ets2js.Process();
    if (ets2js.Error()) {
        InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: ets2js error: ") + ets2js.Error().value());
        return nullptr;
    }
    napi_value jsRes = ets2js.GetResult();
    GetConverterResult("ets2js", begin);

    // Check that the method has a return value
    panda_file::Type retType = method->GetProto().GetReturnType();
    if (retType.GetId() != panda_file::Type::TypeId::VOID) {
        ASSERT(jsRes != nullptr);
        jsHandleScope.Escape(jsRes);
    } else {
        ASSERT(jsRes == nullptr);
    }

    return jsRes;
}

static std::optional<std::vector<ark::Value>> GetArgs(napi_env env, Method *method, napi_value *jsargv, uint32_t jsargc)
{
    JsToEtsArgsConvertor js2ets(method, env, jsargv, jsargc - 1, 1);

    NapiScope jsHandleScope(env);
    ark::HandleScope<ark::ObjectHeader *> etsHandleScope(EtsCoroutine::GetCurrent());

    auto begin = std::chrono::steady_clock::now();
    js2ets.Process();
    if (js2ets.Error()) {
        InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: js2ets error: ") + js2ets.Error().value());
        return std::nullopt;
    }

    std::vector<ark::Value> args = js2ets.GetResult();
    GetConverterResult("js2ets", begin);

    return args;
}

napi_value InvokeEtsMethodImpl(napi_env env, napi_value *jsargv, uint32_t jsargc, bool doClscheck)
{
    auto coro = EtsCoroutine::GetCurrent();
    INTEROP_CODE_SCOPE_JS(coro, env);

    if (jsargc < 1) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: method name required");
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (GetValueType(env, jsargv[0]) != napi_string) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: method name is not a string");
        return nullptr;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto methodName = GetString(env, jsargv[0]);
    INTEROP_LOG(INFO) << "InvokeEtsMethod: method name: " << methodName;

    auto methodRes = ResolveEntryPoint(InteropCtx::Current(coro), methodName);
    if (!methodRes) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: can't resolve method " + methodName);
        return nullptr;
    }
    auto method = methodRes.Value();

    auto numArgs = method->GetNumArgs();
    if (numArgs != jsargc - 1) {
        InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: wrong argc"));
        return nullptr;
    }

    if (doClscheck && !EtsClassesRecursionChecker::CheckClasses(method)) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: loops possible in args objects");
        return nullptr;
    }

    auto args = GetArgs(env, method, jsargv, jsargc);
    if (!args.has_value()) {
        return nullptr;
    }

    ark::Value etsRes = method->Invoke(coro, args.value().data());
    if (UNLIKELY(coro->HasPendingException())) {
        ThrowProperError(env, coro);
        return nullptr;
    }

    return GetJsRef(env, etsRes, method);
}

}  // namespace ark::ets::interop::js
