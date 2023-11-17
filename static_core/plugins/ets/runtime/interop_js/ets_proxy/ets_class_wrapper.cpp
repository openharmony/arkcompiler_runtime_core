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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"

#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/js_value_call.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "runtime/mem/local_object_handle.h"

namespace panda::ets::interop::js::ets_proxy {

class JSRefConvertEtsProxy : public JSRefConvert {
public:
    explicit JSRefConvertEtsProxy(EtsClassWrapper *ets_class_wrapper)
        : JSRefConvert(this), ets_class_wrapper_(ets_class_wrapper)
    {
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *ets_object)
    {
        return ets_class_wrapper_->Wrap(ctx, ets_object);
    }
    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value js_value)
    {
        return ets_class_wrapper_->Unwrap(ctx, js_value);
    }

private:
    EtsClassWrapper *ets_class_wrapper_ {};
};

napi_value EtsClassWrapper::Wrap(InteropCtx *ctx, EtsObject *ets_object)
{
    CheckClassInitialized(ets_class_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(ets_object != nullptr);
    ASSERT(ets_class_ == ets_object->GetClass());

    SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(ets_object))) {
        SharedReference *shared_ref = storage->GetReference(ets_object);
        ASSERT(shared_ref != nullptr);
        return shared_ref->GetJsObject(env);
    }

    napi_value js_value;
    // ets_object will be wrapped in js_value in responce to js_ctor call
    ctx->SetPendingNewInstance(ets_object);
    NAPI_CHECK_FATAL(napi_new_instance(env, GetJsCtor(env), 0, nullptr, &js_value));
    return js_value;
}

// Use UnwrapEtsProxy if you expect exactly a EtsProxy
EtsObject *EtsClassWrapper::Unwrap(InteropCtx *ctx, napi_value js_value)
{
    CheckClassInitialized(ets_class_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(!IsNullOrUndefined(env, js_value));

    // Check if object has SharedReference
    SharedReference *shared_ref = ctx->GetSharedRefStorage()->GetReference(env, js_value);
    if (LIKELY(shared_ref != nullptr)) {
        EtsObject *ets_object = shared_ref->GetEtsObject(ctx);
        if (UNLIKELY(!ets_class_->IsAssignableFrom(ets_object->GetClass()))) {
            ThrowJSErrorNotAssignable(env, ets_object->GetClass(), ets_class_);
            return nullptr;
        }
        return ets_object;
    }

    // Check if object is subtype of js builtin class
    if (LIKELY(HasBuiltin())) {
        ASSERT(js_builtin_matcher_ != nullptr);
        auto res = js_builtin_matcher_(ctx, js_value, false);
        ASSERT(res != nullptr || ctx->SanityJSExceptionPending() || ctx->SanityETSExceptionPending());
        return res;
    }

    InteropCtx::ThrowJSTypeError(env, std::string("Value is not assignable to ") + ets_class_->GetDescriptor());
    return nullptr;
}

// Special method to ensure unwrapped object is not a JSProxy
EtsObject *EtsClassWrapper::UnwrapEtsProxy(InteropCtx *ctx, napi_value js_value)
{
    CheckClassInitialized(ets_class_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(!IsNullOrUndefined(env, js_value));

    // Check if object has SharedReference
    SharedReference *shared_ref = ctx->GetSharedRefStorage()->GetReference(env, js_value);
    if (LIKELY(shared_ref != nullptr)) {
        EtsObject *ets_object = shared_ref->GetEtsObject(ctx);
        if (UNLIKELY(!ets_class_->IsAssignableFrom(ets_object->GetClass()))) {
            ThrowJSErrorNotAssignable(env, ets_object->GetClass(), ets_class_);
            return nullptr;
        }
        if (UNLIKELY(!shared_ref->GetField<SharedReference::HasETSObject>())) {
            InteropCtx::ThrowJSTypeError(env, std::string("JS object in context of EtsProxy of class ") +
                                                  ets_class_->GetDescriptor());
            return nullptr;
        }
        return ets_object;
    }
    return nullptr;
}

void EtsClassWrapper::ThrowJSErrorNotAssignable(napi_env env, EtsClass *from_klass, EtsClass *to_klass)
{
    const char *from = from_klass->GetDescriptor();
    const char *to = to_klass->GetDescriptor();
    InteropCtx::ThrowJSTypeError(env, std::string(from) + " is not assignable to " + to);
}

EtsObject *EtsClassWrapper::CreateJSBuiltinProxy(InteropCtx *ctx, napi_value js_value)
{
    ASSERT(jsproxy_wrapper_ != nullptr);
    ASSERT(SharedReference::ExtractMaybeReference(ctx->GetJSEnv(), js_value) == nullptr);

    EtsObject *ets_object = EtsObject::Create(jsproxy_wrapper_->GetProxyClass());
    if (UNLIKELY(ets_object == nullptr)) {
        ctx->ForwardEtsException(EtsCoroutine::GetCurrent());
        return nullptr;
    }

    SharedReference *shared_ref = ctx->GetSharedRefStorage()->CreateJSObjectRef(ctx, ets_object, js_value);
    if (UNLIKELY(shared_ref == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }
    return shared_ref->GetEtsObject(ctx);  // fetch again after gc
}

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass)
{
    EtsClass *ets_class = EtsClass::FromRuntimeClass(klass);
    EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, ets_class);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    ASSERT(wrapper->ets_class_ == ets_class);
    return std::make_unique<JSRefConvertEtsProxy>(wrapper);
}

class JSRefConvertJSProxy : public JSRefConvert {
public:
    explicit JSRefConvertJSProxy() : JSRefConvert(this) {}

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *ets_object)
    {
        SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        INTEROP_FATAL_IF(!storage->HasReference(ets_object));
        SharedReference *shared_ref = storage->GetReference(ets_object);
        ASSERT(shared_ref != nullptr);
        return shared_ref->GetJsObject(ctx->GetJSEnv());
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, [[maybe_unused]] napi_value js_value)
    {
        ctx->Fatal("Unwrap called on JSProxy class");
    }
};

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertJSProxy([[maybe_unused]] InteropCtx *ctx,
                                                                         [[maybe_unused]] Class *klass)
{
    ASSERT(js_proxy::JSProxy::IsProxyClass(klass));
    return std::make_unique<JSRefConvertJSProxy>();
}

/*static*/
EtsClassWrapper *EtsClassWrapper::Get(InteropCtx *ctx, EtsClass *ets_class)
{
    EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();

    ASSERT(ets_class != nullptr);
    EtsClassWrapper *ets_class_wrapper = cache->Lookup(ets_class);
    if (LIKELY(ets_class_wrapper != nullptr)) {
        return ets_class_wrapper;
    }

    ASSERT(!ets_class->IsPrimitive() && ets_class->GetComponentType() == nullptr);
    ASSERT(ctx->GetRefConvertCache()->Lookup(ets_class->GetRuntimeClass()) == nullptr);

    if (IsStdClass(ets_class)) {
        ctx->Fatal(std::string("ets_proxy requested for ") + ets_class->GetDescriptor() + " must add or forbid");
    }
    ASSERT(!js_proxy::JSProxy::IsProxyClass((ets_class->GetRuntimeClass())));

    std::unique_ptr<EtsClassWrapper> wrapper = EtsClassWrapper::Create(ctx, ets_class);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    return cache->Insert(ets_class, std::move(wrapper));
}

bool EtsClassWrapper::SetupHierarchy(InteropCtx *ctx, const char *js_builtin_name)
{
    ASSERT(ets_class_->GetBase() != ets_class_);
    if (ets_class_->GetBase() != nullptr) {
        base_wrapper_ = EtsClassWrapper::Get(ctx, ets_class_->GetBase());
        if (base_wrapper_ == nullptr) {
            return false;
        }
    }

    if (js_builtin_name != nullptr) {
        auto env = ctx->GetJSEnv();
        napi_value js_builtin_ctor;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), js_builtin_name, &js_builtin_ctor));
        NAPI_CHECK_FATAL(napi_create_reference(env, js_builtin_ctor, 1, &js_builtin_ctor_ref_));
    }
    return true;
}

std::pair<std::vector<Field *>, std::vector<Method *>> EtsClassWrapper::CalculateProperties(
    const OverloadsMap *overloads)
{
    auto fatal_method_overloaded = [](Method *method) {
        for (auto &m : method->GetClass()->GetMethods()) {
            if (utf::IsEqual(m.GetName().data, method->GetName().data)) {
                INTEROP_LOG(ERROR) << "overload: " << EtsMethod::FromRuntimeMethod(&m)->GetMethodSignature(true);
            }
        }
        InteropCtx::Fatal(std::string("Method ") + utf::Mutf8AsCString(method->GetName().data) + " of class " +
                          utf::Mutf8AsCString(method->GetClass()->GetDescriptor()) + " is overloaded");
    };
    auto fatal_no_method = [](Class *klass, const uint8_t *name, const char *signature) {
        InteropCtx::Fatal(std::string("No method ") + utf::Mutf8AsCString(name) + " " + signature + " in " +
                          klass->GetName());
    };

    auto klass = ets_class_->GetRuntimeClass();
    std::unordered_map<uint8_t const *, std::variant<Method *, Field *>, utf::Mutf8Hash, utf::Mutf8Equal> props;

    // Collect fields
    for (auto &f : klass->GetFields()) {
        if (f.IsPublic()) {
            props.insert({f.GetName().data, &f});
        }
    }
    // Select preferred overloads
    if (overloads != nullptr) {
        for (auto &[name, signature] : *overloads) {
            Method *method = ets_class_->GetDirectMethod(name, signature)->GetPandaMethod();
            if (UNLIKELY(method == nullptr)) {
                fatal_no_method(klass, name, signature);
            }
            auto it = props.insert({method->GetName().data, method});
            INTEROP_FATAL_IF(!it.second);
        }
    }

    // If class is std.core.Object
    auto klass_desc = utf::Mutf8AsCString(klass->GetDescriptor());
    if (klass_desc == panda_file_items::class_descriptors::OBJECT) {
        // Ingore all methods of std.core.Object due to names intersection with JS Object
        // Keep constructors only
        auto obj_ctors = ets_class_->GetConstructors();
        // Assuming that ETS StdLib guarantee that Object has the only one ctor
        ASSERT(obj_ctors.size() == 1);
        auto ctor = obj_ctors[0]->GetPandaMethod();
        props.insert({ctor->GetName().data, ctor});
        // NOTE(shumilov-petr): Think about removing methods from std.core.Object
        // that are already presented in JS Object, others should be kept
    } else {
        // Collect methods
        for (auto &m : klass->GetMethods()) {
            if (m.IsPrivate()) {
                continue;
            }
            auto name = m.GetName().data;
            if (overloads != nullptr && overloads->find(name) != overloads->end()) {
                continue;
            }
            auto it = props.insert({m.GetName().data, &m});
            if (UNLIKELY(!it.second)) {
                fatal_method_overloaded(&m);
            }
        }
    }

    auto has_squashed_proto = [](EtsClassWrapper *wclass) {
        ASSERT(wclass->HasBuiltin() || wclass->base_wrapper_ != nullptr);
#if PANDA_TARGET_OHOS
        return wclass->HasBuiltin() || wclass->base_wrapper_->HasBuiltin();
#else
        (void)wclass;
        return true;  // NOTE(vpukhov): some napi implementations add explicit receiver checks in call handler,
                      // thus method inheritance via prototype chain wont work
#endif
    };

    if (has_squashed_proto(this)) {
        // Copy properties of base classes if we have to split prototype chain
        for (auto wclass = base_wrapper_; wclass != nullptr; wclass = wclass->base_wrapper_) {
            for (auto &wfield : wclass->GetFields()) {
                Field *field = wfield.GetField();
                props.insert({field->GetName().data, field});
            }
            for (auto &link : wclass->GetMethods()) {
                Method *method = link.IsResolved() ? link.GetResolved()->GetMethod() : link.GetUnresolved();
                props.insert({method->GetName().data, method});
            }

            if (has_squashed_proto(wclass)) {
                break;
            }
        }
    }

    std::vector<Method *> methods;
    std::vector<Field *> fields;

    for (auto &[n, p] : props) {
        if (std::holds_alternative<Method *>(p)) {
            auto method = std::get<Method *>(p);
            if (method->IsConstructor()) {
                if (!method->IsStatic()) {
                    ets_ctor_link_ = LazyEtsMethodWrapperLink(method);
                }
            } else {
                methods.push_back(method);
            }
        } else if (std::holds_alternative<Field *>(p)) {
            fields.push_back(std::get<Field *>(p));
        } else {
            UNREACHABLE();
        }
    }

    return {fields, methods};
}

std::vector<napi_property_descriptor> EtsClassWrapper::BuildJSProperties(Span<Field *> fields, Span<Method *> methods)
{
    std::vector<napi_property_descriptor> js_props;
    js_props.reserve(fields.size() + methods.size());

    // Process fields
    num_fields_ = fields.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ets_field_wrappers_ = std::make_unique<EtsFieldWrapper[]>(num_fields_);
    Span<EtsFieldWrapper> ets_field_wrappers(ets_field_wrappers_.get(), num_fields_);
    size_t field_idx = 0;

    for (Field *field : fields) {
        auto wfield = &ets_field_wrappers[field_idx++];
        if (field->IsStatic()) {
            EtsClassWrapper *field_wclass = LookupBaseWrapper(EtsClass::FromRuntimeClass(field->GetClass()));
            ASSERT(field_wclass != nullptr);
            js_props.emplace_back(wfield->MakeStaticProperty(field_wclass, field));
        } else {
            js_props.emplace_back(wfield->MakeInstanceProperty(this, field));
        }
    }

    // Process methods
    num_methods_ = methods.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ets_method_wrappers_ = std::make_unique<LazyEtsMethodWrapperLink[]>(num_methods_);
    Span<LazyEtsMethodWrapperLink> ets_method_wrappers(ets_method_wrappers_.get(), num_methods_);
    size_t method_idx = 0;

#ifndef NDEBUG
    std::unordered_set<const uint8_t *> method_names;
#endif

    for (auto &method : methods) {
        ASSERT(!method->IsConstructor());
        ASSERT(method_names.insert(method->GetName().data).second);
        auto lazy_link = &ets_method_wrappers[method_idx++];
        lazy_link->Set(method);
        js_props.emplace_back(EtsMethodWrapper::MakeNapiProperty(method, lazy_link));
    }
    ASSERT(ets_ctor_link_.GetUnresolved() != nullptr);

    return js_props;
}

EtsClassWrapper *EtsClassWrapper::LookupBaseWrapper(EtsClass *klass)
{
    for (auto wclass = this; wclass != nullptr; wclass = wclass->base_wrapper_) {
        if (wclass->ets_class_ == klass) {
            return wclass;
        }
    }
    return nullptr;
}

static void SimulateJSInheritance(napi_env env, napi_value js_ctor, napi_value js_base_ctor)
{
    napi_value builtin_object;
    napi_value setproto_fn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), "Object", &builtin_object));
    NAPI_CHECK_FATAL(napi_get_named_property(env, builtin_object, "setPrototypeOf", &setproto_fn));

    auto setproto = [&](napi_value obj, napi_value proto) {
        std::array args = {obj, proto};
        NAPI_CHECK_FATAL(NapiCallFunction(env, builtin_object, setproto_fn, args.size(), args.data(), nullptr));
    };

    napi_value cprototype;
    napi_value base_cprototype;
    NAPI_CHECK_FATAL(napi_get_named_property(env, js_ctor, "prototype", &cprototype));
    NAPI_CHECK_FATAL(napi_get_named_property(env, js_base_ctor, "prototype", &base_cprototype));

    setproto(js_ctor, js_base_ctor);
    setproto(cprototype, base_cprototype);
}

/*static*/
std::unique_ptr<EtsClassWrapper> EtsClassWrapper::Create(InteropCtx *ctx, EtsClass *ets_class,
                                                         const char *js_builtin_name, const OverloadsMap *overloads)
{
    auto env = ctx->GetJSEnv();

    // NOLINTNEXTLINE(readability-identifier-naming)
    auto _this = std::unique_ptr<EtsClassWrapper>(new EtsClassWrapper(ets_class));
    if (!_this->SetupHierarchy(ctx, js_builtin_name)) {
        return nullptr;
    }

    auto [fields, methods] = _this->CalculateProperties(overloads);
    auto js_props = _this->BuildJSProperties({fields.data(), fields.size()}, {methods.data(), methods.size()});

    // NOTE(vpukhov): restore no-public-fields check when escompat adopt accessors
    if (_this->HasBuiltin() && !fields.empty()) {
        // ctx->Fatal(std::string("built-in class ") + ets_class->GetDescriptor() + " has field properties");
        INTEROP_LOG(ERROR) << "built-in class " << ets_class->GetDescriptor() << " has field properties";
    }
    // NOTE(vpukhov): forbid "true" ets-field overriding in js-derived class, as it cannot be proxied back
    //                simple solution: ban JSProxy if !fields.empty()
    _this->jsproxy_wrapper_ = js_proxy::JSProxy::Create(ets_class, {methods.data(), methods.size()});

    napi_value js_ctor {};
    NAPI_CHECK_FATAL(napi_define_class(env, ets_class->GetDescriptor(), NAPI_AUTO_LENGTH,
                                       EtsClassWrapper::JSCtorCallback, _this.get(), js_props.size(), js_props.data(),
                                       &js_ctor));

    auto base = _this->base_wrapper_;
    napi_value fake_super = _this->HasBuiltin() ? _this->GetBuiltin(env)
                                                : (base->HasBuiltin() ? base->GetBuiltin(env) : base->GetJsCtor(env));

    SimulateJSInheritance(env, js_ctor, fake_super);
    NAPI_CHECK_FATAL(NapiObjectSeal(env, js_ctor));
    NAPI_CHECK_FATAL(napi_create_reference(env, js_ctor, 1, &_this->js_ctor_ref_));

    return _this;
}

/*static*/
napi_value EtsClassWrapper::JSCtorCallback(napi_env env, napi_callback_info cinfo)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope envscope(ctx, env);

    napi_value js_this;
    size_t argc;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, &js_this, &data));
    auto ets_class_wrapper = reinterpret_cast<EtsClassWrapper *>(data);

    EtsObject *ets_object = ctx->AcquirePendingNewInstance();

    if (LIKELY(ets_object != nullptr)) {
        // Create shared reference for existing ets object
        SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (UNLIKELY(!storage->CreateETSObjectRef(ctx, ets_object, js_this))) {
            ASSERT(InteropCtx::SanityJSExceptionPending());
            return nullptr;
        }
        NAPI_CHECK_FATAL(napi_object_seal(env, js_this));
        return nullptr;
    }

    auto js_args = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, js_args->data(), nullptr, nullptr));

    napi_value js_newtarget;
    NAPI_CHECK_FATAL(napi_get_new_target(env, cinfo, &js_newtarget));

    // create new object and wrap it
    if (UNLIKELY(!ets_class_wrapper->CreateAndWrap(env, js_newtarget, js_this, *js_args))) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }

    // NOTE(ivagin): JS constructor is not required to return 'this', but ArkUI NAPI requires it
    return js_this;
}

bool EtsClassWrapper::CreateAndWrap(napi_env env, napi_value js_newtarget, napi_value js_this, Span<napi_value> js_args)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    ScopedManagedCodeThread managed_scope(coro);

    if (UNLIKELY(!CheckClassInitialized<true>(ets_class_->GetRuntimeClass()))) {
        ctx->ForwardEtsException(coro);
        return false;
    }

    bool not_extensible;
    NAPI_CHECK_FATAL(napi_strict_equals(env, js_newtarget, GetJsCtor(env), &not_extensible));

    EtsClass *instance_class {};

    if (LIKELY(not_extensible)) {
        NAPI_CHECK_FATAL(napi_object_seal(env, js_this));
        instance_class = ets_class_;
    } else {
        if (UNLIKELY(jsproxy_wrapper_ == nullptr)) {
            ctx->ThrowJSTypeError(env, std::string("Proxy for ") + ets_class_->GetDescriptor() + " is not extensible");
            return false;
        }
        instance_class = jsproxy_wrapper_->GetProxyClass();
    }

    LocalObjectHandle<EtsObject> ets_object(coro, EtsObject::Create(instance_class));
    if (UNLIKELY(ets_object.GetPtr() == nullptr)) {
        return false;
    }

    SharedReference *shared_ref;
    if (LIKELY(not_extensible)) {
        shared_ref = ctx->GetSharedRefStorage()->CreateETSObjectRef(ctx, ets_object.GetPtr(), js_this);
    } else {
        shared_ref = ctx->GetSharedRefStorage()->CreateHybridObjectRef(ctx, ets_object.GetPtr(), js_this);
    }
    if (UNLIKELY(shared_ref == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return false;
    }

    EtsMethodWrapper *ctor_wrapper = EtsMethodWrapper::ResolveLazyLink(ctx, ets_ctor_link_);
    ASSERT(ctor_wrapper != nullptr);
    EtsMethod *ctor_method = ctor_wrapper->GetEtsMethod();
    ASSERT(ctor_method->IsInstanceConstructor());

    napi_value call_res = EtsCallImplInstance(coro, ctx, ctor_method->GetPandaMethod(), js_args, ets_object.GetPtr());
    return call_res != nullptr;
}

}  // namespace panda::ets::interop::js::ets_proxy
