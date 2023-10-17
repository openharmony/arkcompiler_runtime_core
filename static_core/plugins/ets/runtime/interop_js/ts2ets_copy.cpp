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

#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/ets_type_visitor-inl.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/handle_scope-inl.h"

namespace panda::ets::interop::js {

static inline EtsConvertorRef::ObjRoot ToObjRoot(uintptr_t ptr)
{
    return reinterpret_cast<EtsConvertorRef::ObjRoot>(ptr);
}

class JsToEtsConvertor final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

public:
    JsToEtsConvertor(napi_env env, napi_value js_value, EtsConvertorRef::ValVariant *data_ptr)
        : loc_(data_ptr), env_(env), js_value_(js_value)
    {
    }
    JsToEtsConvertor(napi_env env, napi_value js_value, EtsConvertorRef::ObjRoot ets_obj, size_t ets_offs)
        : loc_(ets_obj, ets_offs), env_(env), js_value_(js_value)
    {
    }

    void VisitU1() override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_boolean, "bool expected");
        bool val;
        TYPEVIS_NAPI_CHECK(napi_get_value_bool(env_, js_value_, &val));
        loc_.StorePrimitive(static_cast<bool>(val));
    }

    template <typename T>
    inline void VisitIntType()
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_number, "number expected");
        int64_t val;
        TYPEVIS_NAPI_CHECK(napi_get_value_int64(env_, js_value_, &val));
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
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_number, "number expected");
        double val;
        TYPEVIS_NAPI_CHECK(napi_get_value_double(env_, js_value_, &val));
        loc_.StorePrimitive(static_cast<float>(val));
    }

    void VisitF64() override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_number, "number expected");
        double val;
        TYPEVIS_NAPI_CHECK(napi_get_value_double(env_, js_value_, &val));
        loc_.StorePrimitive(val);
    }

    void VisitString([[maybe_unused]] panda::Class *klass) override
    {
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_string, "string expected");

        size_t length;
        TYPEVIS_NAPI_CHECK(napi_get_value_string_utf8(env_, js_value_, nullptr, 0, &length));

        std::string value;
        value.resize(length + 1);
        size_t result;
        TYPEVIS_NAPI_CHECK(napi_get_value_string_utf8(env_, js_value_, &value[0], value.size(), &result));

        auto coro = EtsCoroutine::GetCurrent();
        auto vm = coro->GetVM();
        auto str = panda::coretypes::String::CreateFromMUtf8(reinterpret_cast<uint8_t const *>(value.data()),
                                                             vm->GetLanguageContext(), vm);
        loc_.StoreReference(ToObjRoot(panda::VMHandle<panda::ObjectHeader>(coro, str).GetAddress()));
    }

    void VisitArray(panda::Class *klass) override
    {
        {
            bool is_array;
            TYPEVIS_NAPI_CHECK(napi_is_array(env_, js_value_, &is_array));
            TYPEVIS_CHECK_ERROR(is_array, "array expected");
        }
        auto coro = EtsCoroutine::GetCurrent();

        NapiScope js_handle_scope(env_);

        uint32_t len;
        TYPEVIS_NAPI_CHECK(napi_get_array_length(env_, js_value_, &len));

        panda::VMHandle<panda::ObjectHeader> ets_arr(coro, panda::coretypes::Array::Create(klass, len));

        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);
        auto *sub_type = klass->GetComponentType();
        size_t elem_sz = panda::Class::GetTypeSize(sub_type->GetType());
        constexpr auto ELEMS_OFFS = panda::coretypes::Array::GetDataOffset();
        for (size_t idx = 0; idx < len; ++idx) {
            napi_value js_elem;
            TYPEVIS_NAPI_CHECK(napi_get_element(env_, js_value_, idx, &js_elem));
            JsToEtsConvertor sub_vis(env_, js_elem, ToObjRoot(ets_arr.GetAddress()), ELEMS_OFFS + elem_sz * idx);
            sub_vis.VisitClass(sub_type);
            TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
        }
        loc_.StoreReference(ToObjRoot(ets_arr.GetAddress()));
    }

    void VisitFieldReference(const panda::Field *field, panda::Class *klass) override
    {
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_value sub_val;
        TYPEVIS_NAPI_CHECK(napi_get_named_property(env_, js_value_, fname, &sub_val));
        auto sub_vis = JsToEtsConvertor(env_, sub_val, owner_, field->GetOffset());
        sub_vis.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
    }

    void VisitFieldPrimitive(const panda::Field *field, panda::panda_file::Type type) override
    {
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_value sub_val;
        TYPEVIS_NAPI_CHECK(napi_get_named_property(env_, js_value_, fname, &sub_val));
        auto sub_vis = JsToEtsConvertor(env_, sub_val, owner_, field->GetOffset());
        sub_vis.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
    }

    void VisitObject(panda::Class *klass) override
    {
        ASSERT(klass != InteropCtx::Current()->GetJSValueClass());
        TYPEVIS_CHECK_ERROR(GetValueType(env_, js_value_) == napi_object, "object expected");
        auto coro = EtsCoroutine::GetCurrent();

        NapiScope js_handle_scope(env_);
        {
            auto ets_obj = panda::ObjectHeader::Create(coro, klass);
            owner_ = ToObjRoot(panda::VMHandle<panda::ObjectHeader>(coro, ets_obj).GetAddress());
        }

        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);
        Base::VisitObject(klass);
        TYPEVIS_ABRUPT_ON_ERROR();
        loc_.StoreReference(owner_);
    }

private:
    EtsConvertorRef loc_;
    EtsConvertorRef::ObjRoot owner_ = nullptr;

    napi_env env_ {};
    napi_value js_value_ {};
};

class EtsToJsConvertor final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

public:
    EtsToJsConvertor(napi_env env, EtsConvertorRef::ValVariant *data_ptr) : loc_(data_ptr), env_(env) {}
    EtsToJsConvertor(napi_env env, EtsConvertorRef::ObjRoot ets_obj, size_t ets_offs)
        : loc_(ets_obj, ets_offs), env_(env)
    {
    }

    void VisitU1() override
    {
        bool ets_val = loc_.LoadPrimitive<bool>();
        TYPEVIS_NAPI_CHECK(napi_get_boolean(env_, ets_val, &js_value_));
    }

    template <typename T>
    inline void VisitIntType()
    {
        auto ets_val = static_cast<int64_t>(loc_.LoadPrimitive<T>());
        TYPEVIS_NAPI_CHECK(napi_create_int64(env_, ets_val, &js_value_));
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
        auto ets_val = static_cast<double>(loc_.LoadPrimitive<float>());
        TYPEVIS_NAPI_CHECK(napi_create_double(env_, ets_val, &js_value_));
    }

    void VisitF64() override
    {
        auto ets_val = loc_.LoadPrimitive<double>();
        TYPEVIS_NAPI_CHECK(napi_create_double(env_, ets_val, &js_value_));
    }

    void VisitString([[maybe_unused]] panda::Class *klass) override
    {
        auto ets_str = static_cast<panda::coretypes::String *>(loc_.LoadReference());
        TYPEVIS_NAPI_CHECK(napi_create_string_utf8(env_, reinterpret_cast<const char *>(ets_str->GetDataMUtf8()),
                                                   ets_str->GetMUtf8Length() - 1, &js_value_));
    }

    void VisitArray(panda::Class *klass) override
    {
        auto coro = EtsCoroutine::GetCurrent();

        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);
        auto ets_arr = panda::VMHandle<panda::coretypes::Array>(coro, loc_.LoadReference());
        auto len = ets_arr->GetLength();

        TYPEVIS_NAPI_CHECK(napi_create_array_with_length(env_, len, &js_value_));

        NapiScope js_handle_scope(env_);
        auto *sub_type = klass->GetComponentType();
        size_t elem_sz = panda::Class::GetTypeSize(sub_type->GetType());
        auto constexpr ELEMS_OFFS = panda::coretypes::Array::GetDataOffset();
        for (size_t idx = 0; idx < len; ++idx) {
            EtsToJsConvertor sub_vis(env_, ToObjRoot(ets_arr.GetAddress()), ELEMS_OFFS + elem_sz * idx);
            sub_vis.VisitClass(sub_type);
            TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
            TYPEVIS_NAPI_CHECK(napi_set_element(env_, js_value_, idx, sub_vis.js_value_));
        }
    }

    void VisitFieldReference(const panda::Field *field, panda::Class *klass) override
    {
        auto sub_vis = EtsToJsConvertor(env_, owner_, field->GetOffset());
        sub_vis.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_set_named_property(env_, js_value_, fname, sub_vis.js_value_);
    }

    void VisitFieldPrimitive(const panda::Field *field, panda::panda_file::Type type) override
    {
        auto sub_vis = EtsToJsConvertor(env_, owner_, field->GetOffset());
        sub_vis.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(sub_vis.Error());
        auto fname = reinterpret_cast<const char *>(field->GetName().data);
        napi_set_named_property(env_, js_value_, fname, sub_vis.js_value_);
    }

    void VisitObject(panda::Class *klass) override
    {
        ASSERT(klass != InteropCtx::Current()->GetJSValueClass());
        auto coro = EtsCoroutine::GetCurrent();
        if (klass == InteropCtx::Current(coro)->GetPromiseClass()) {
            VisitPromise();
            return;
        }

        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);
        {
            auto ets_obj = loc_.LoadReference();
            owner_ = ToObjRoot(panda::VMHandle<panda::ObjectHeader>(coro, ets_obj).GetAddress());
        }

        TYPEVIS_NAPI_CHECK(napi_create_object(env_, &js_value_));

        NapiScope js_handle_scope(env_);
        Base::VisitObject(klass);
    }

    void VisitPromise()
    {
        napi_deferred deferred;
        TYPEVIS_NAPI_CHECK(napi_create_promise(env_, &deferred, &js_value_));
        auto *coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] HandleScope<panda::ObjectHeader *> handle_scope(coro);
        VMHandle<EtsPromise> promise(coro, reinterpret_cast<EtsPromise *>(loc_.LoadReference()));
        if (promise->GetState() != EtsPromise::STATE_PENDING) {
            VMHandle<EtsObject> value(coro, promise->GetValue(coro)->GetCoreType());
            napi_value completion_value;
            if (value.GetPtr() == nullptr) {
                napi_get_null(env_, &completion_value);
            } else {
                EtsConvertorRef::ObjRoot v = ToObjRoot(VMHandle<ObjectHeader>(coro, value->GetCoreType()).GetAddress());
                EtsConvertorRef::ValVariant val_var(v);
                EtsToJsConvertor value_conv(env_, &val_var);
                value_conv.VisitReference(value->GetClass()->GetRuntimeClass());
                TYPEVIS_CHECK_FORWARD_ERROR(value_conv.Error());
                completion_value = value_conv.GetResult();
            }
            if (promise->IsResolved()) {
                TYPEVIS_NAPI_CHECK(napi_resolve_deferred(env_, deferred, completion_value));
            } else {
                TYPEVIS_NAPI_CHECK(napi_reject_deferred(env_, deferred, completion_value));
            }
        } else {
            UNREACHABLE();
        }
    }

    napi_value GetResult()
    {
        return js_value_;
    }

private:
    EtsConvertorRef loc_;
    EtsConvertorRef::ObjRoot owner_ {};

    napi_env env_ {};
    napi_value js_value_ {};
};

class JsToEtsArgsConvertor final : public EtsMethodVisitor {
public:
    JsToEtsArgsConvertor(panda::Method *method, napi_env env, napi_value *args, uint32_t num_args, uint32_t args_start)
        : EtsMethodVisitor(method), env_(env), jsargs_(args), num_args_(num_args), args_start_(args_start)
    {
        ASSERT(num_args_ == method->GetNumArgs());
        etsargs_.resize(num_args_);
    }

    std::vector<panda::Value> GetResult()
    {
        std::vector<panda::Value> res;
        for (auto const &e : etsargs_) {
            if (std::holds_alternative<panda::Value>(e)) {
                res.emplace_back(std::get<panda::Value>(e));
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
    void VisitReturn([[maybe_unused]] panda::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitReturn([[maybe_unused]] panda::Class *klass) override
    {
        // do nothing
    }
    void VisitArgument(uint32_t idx, panda::panda_file::Type type) override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        JsToEtsConvertor conv(env_, jsargs_[idx + args_start_], &etsargs_[idx]);
        conv.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
    }
    void VisitArgument(uint32_t idx, panda::Class *klass) override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        JsToEtsConvertor conv(env_, jsargs_[idx + args_start_], &etsargs_[idx]);
        conv.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
    }

    napi_env env_;
    napi_value *jsargs_;
    uint32_t num_args_;
    uint32_t args_start_;
    std::vector<EtsConvertorRef::ValVariant> etsargs_;
};

class EtsArgsClassesCollector final : protected EtsMethodVisitor {
public:
    explicit EtsArgsClassesCollector(panda::Method *method) : EtsMethodVisitor(method) {}

    void GetResult(std::unordered_set<panda::Class *> &to)
    {
        VisitMethod();
        to = std::move(set_);
    }

private:
    void VisitReturn([[maybe_unused]] panda::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitReturn(panda::Class *klass) override
    {
        AddClass(klass);
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] panda::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, panda::Class *klass) override
    {
        AddClass(klass);
    }

    void AddClass(panda::Class *klass)
    {
        set_.insert(klass);
    }

    std::unordered_set<panda::Class *> set_;
};

class EtsClassesRecursionChecker final : public EtsTypeVisitor {
    using Base = EtsTypeVisitor;

    struct DFSData {
        explicit DFSData(panda::Class *cls) : klass(cls) {}

        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        panda::Class *klass = nullptr;

        enum : uint8_t {
            MARK_W = 0,
            MARK_G = 1,
            MARK_B = 2,
        } mark = MARK_W;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    static bool CheckClasses(panda::Method *method)
    {
        std::unordered_set<panda::Class *> class_set;
        EtsArgsClassesCollector collector(method);
        collector.GetResult(class_set);

        std::unordered_map<panda::Class *, DFSData> dfs_data;
        for (auto const &e : class_set) {
            dfs_data.insert(std::make_pair(e, DFSData(e)));
        }

        bool has_loops = false;
        EtsClassesRecursionChecker checker(has_loops, dfs_data);
        for (auto const &e : dfs_data) {
            checker.VisitClass(e.second.klass);
            if (has_loops) {
                break;
            }
        }
        return !has_loops;
    }

private:
    EtsClassesRecursionChecker(bool &loop_found, std::unordered_map<panda::Class *, DFSData> &dfs)
        : loop_found_(loop_found), dfs_(dfs)
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

    void VisitPrimitive([[maybe_unused]] const panda::panda_file::Type type) override {}

    void VisitString([[maybe_unused]] panda::Class *klass) override {}

    void VisitArray(panda::Class *klass) override
    {
        VisitClass(klass->GetComponentType());
    }

    void VisitFieldPrimitive([[maybe_unused]] const panda::Field *field,
                             [[maybe_unused]] panda::panda_file::Type type) override
    {
    }

    void VisitFieldReference([[maybe_unused]] const panda::Field *field, panda::Class *klass) override
    {
        VisitReference(klass);
    }

    void VisitReference(panda::Class *klass) override
    {
        auto str = klass->GetName();
        auto &data = Lookup(klass);
        if (data.mark == DFSData::MARK_B) {
            return;
        }
        if (data.mark == DFSData::MARK_G) {
            loop_found_ = true;
            return;
        }
        data.mark = DFSData::MARK_G;
        Base::VisitReference(klass);
        data.mark = DFSData::MARK_B;
    }

    DFSData &Lookup(panda::Class *klass)
    {
        auto it = dfs_.find(klass);
        if (it == dfs_.end()) {
            auto x = dfs_.insert(std::make_pair(klass, DFSData(klass)));
            return x.first->second;
        }
        return it->second;
    }

    bool &loop_found_;
    std::unordered_map<panda::Class *, DFSData> &dfs_;
};

class EtsToJsRetConvertor final : public EtsMethodVisitor {
public:
    EtsToJsRetConvertor(panda::Method *method, napi_env env, panda::Value ret) : EtsMethodVisitor(method), env_(env)
    {
        if (ret.IsReference()) {
            auto coro = EtsCoroutine::GetCurrent();
            ret_ = ToObjRoot(panda::VMHandle<panda::ObjectHeader>(coro, *ret.GetGCRoot()).GetAddress());
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
        return js_ret_;
    }

private:
    void VisitReturn(panda::panda_file::Type type) override
    {
        if (type.GetId() == panda_file::Type::TypeId::VOID) {
            // Skip 'void' type because it doesn't require conversion
            return;
        }

        EtsToJsConvertor conv(env_, &ret_);
        conv.VisitPrimitive(type);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
        js_ret_ = conv.GetResult();
    }
    void VisitReturn(panda::Class *klass) override
    {
        EtsToJsConvertor conv(env_, &ret_);
        conv.VisitReference(klass);
        TYPEVIS_CHECK_FORWARD_ERROR(conv.Error());
        js_ret_ = conv.GetResult();
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] panda::panda_file::Type type) override
    {
        // do nothing
    }
    void VisitArgument([[maybe_unused]] uint32_t idx, [[maybe_unused]] panda::Class *klass) override
    {
        // do nothing
    }

    napi_env env_ {};
    EtsConvertorRef::ValVariant ret_;
    napi_value js_ret_ {};
};

napi_value InvokeEtsMethodImpl(napi_env env, napi_value *jsargv, uint32_t jsargc, bool do_clscheck)
{
    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsJSNapiEnvScope scope(InteropCtx::Current(coro), env);

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
    auto method_name = std::string("ETSGLOBAL::") + GetString(env, jsargv[0]);
    INTEROP_LOG(INFO) << "InvokeEtsMethod: method name: " << method_name.c_str();

    auto method_res = panda::Runtime::GetCurrent()->ResolveEntryPoint(method_name);
    if (!method_res) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: can't resolve method " + method_name);
        return nullptr;
    }
    auto method = method_res.Value();

    auto num_args = method->GetNumArgs();
    if (num_args != jsargc - 1) {
        InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: wrong argc"));
        return nullptr;
    }

    ScopedManagedCodeThread managed_scope(coro);

    if (do_clscheck && !EtsClassesRecursionChecker::CheckClasses(method)) {
        InteropCtx::ThrowJSError(env, "InvokeEtsMethod: loops possible in args objects");
        return nullptr;
    }

    std::vector<panda::Value> args;
    {
        JsToEtsArgsConvertor js2ets(method, env, jsargv, jsargc - 1, 1);

        NapiScope js_handle_scope(env);
        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);

        auto begin = std::chrono::steady_clock::now();
        js2ets.Process();
        if (js2ets.Error()) {
            InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: js2ets error: ") + js2ets.Error().value());
            return nullptr;
        }
        args = js2ets.GetResult();
        auto end = std::chrono::steady_clock::now();
        int64_t t = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
        INTEROP_LOG(INFO) << "InvokeEtsMethod: js2ets elapsed time: " << t << "us";
    }

    panda::Value ets_res = method->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        NapiScope js_handle_scope(env);
        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);

        auto exc = coro->GetException();
        auto klass = exc->ClassAddr<panda::Class>();
        auto data =
            EtsConvertorRef::ValVariant(ToObjRoot(panda::VMHandle<panda::ObjectHeader>(coro, exc).GetAddress()));
        coro->ClearException();

        EtsToJsConvertor ets2js(env, &data);
        ets2js.VisitClass(klass);
        if (ets2js.Error()) {
            InteropCtx::ThrowJSError(env,
                                     std::string("InvokeEtsMethod: ets2js error while converting pending exception: " +
                                                 ets2js.Error().value()));
            return nullptr;
        }
        InteropCtx::ThrowJSValue(env, ets2js.GetResult());
        return nullptr;
    }

    napi_value js_res;
    {
        NapiEscapableScope js_handle_scope(env);
        panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);

        auto begin = std::chrono::steady_clock::now();
        EtsToJsRetConvertor ets2js(method, env, ets_res);
        ets2js.Process();
        if (ets2js.Error()) {
            InteropCtx::ThrowJSError(env, std::string("InvokeEtsMethod: ets2js error: ") + ets2js.Error().value());
            return nullptr;
        }
        js_res = ets2js.GetResult();
        auto end = std::chrono::steady_clock::now();
        int64_t t = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
        INTEROP_LOG(INFO) << "InvokeEtsMethod: ets2js elapsed time: " << t << "us";

        // Check that the method has a return value
        panda_file::Type ret_type = method->GetProto().GetReturnType();
        if (ret_type.GetId() != panda_file::Type::TypeId::VOID) {
            ASSERT(js_res != nullptr);
            js_handle_scope.Escape(js_res);
        } else {
            ASSERT(js_res == nullptr);
        }
    }
    return js_res;
}

}  // namespace panda::ets::interop::js
