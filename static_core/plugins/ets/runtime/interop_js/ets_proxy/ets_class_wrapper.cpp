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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"

#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::interop::js::ets_proxy {

class JSRefConvertEtsProxy : public JSRefConvert {
public:
    explicit JSRefConvertEtsProxy(EtsClassWrapper *etsClassWrapper)
        : JSRefConvert(this), etsClassWrapper_(etsClassWrapper)
    {
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
    {
        return etsClassWrapper_->Wrap(ctx, etsObject);
    }
    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue)
    {
        return etsClassWrapper_->Unwrap(ctx, jsValue);
    }

private:
    EtsClassWrapper *etsClassWrapper_ {};
};

napi_value EtsClassWrapper::Wrap(InteropCtx *ctx, EtsObject *etsObject)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(etsObject != nullptr);
    ASSERT(etsClass_ == etsObject->GetClass());

    SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(etsObject))) {
        SharedReference *sharedRef = storage->GetReference(etsObject);
        ASSERT(sharedRef != nullptr);
        return sharedRef->GetJsObject(env);
    }

    napi_value jsValue;
    // etsObject will be wrapped in jsValue in responce to jsCtor call
    ctx->SetPendingNewInstance(etsObject);
    NAPI_CHECK_FATAL(napi_new_instance(env, GetJsCtor(env), 0, nullptr, &jsValue));
    return jsValue;
}

// Use UnwrapEtsProxy if you expect exactly a EtsProxy
EtsObject *EtsClassWrapper::Unwrap(InteropCtx *ctx, napi_value jsValue)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(!IsNull(env, jsValue));

    // Check if object has SharedReference
    SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *etsObject = sharedRef->GetEtsObject(ctx);
        if (UNLIKELY(!etsClass_->IsAssignableFrom(etsObject->GetClass()))) {
            ThrowJSErrorNotAssignable(env, etsObject->GetClass(), etsClass_);
            return nullptr;
        }
        return etsObject;
    }

    // Check if object is subtype of js builtin class
    if (LIKELY(HasBuiltin())) {
        ASSERT(jsBuiltinMatcher_ != nullptr);
        auto res = jsBuiltinMatcher_(ctx, jsValue, false);
        ASSERT(res != nullptr || ctx->SanityJSExceptionPending() || ctx->SanityETSExceptionPending());
        return res;
    }

    InteropCtx::ThrowJSTypeError(env, std::string("Value is not assignable to ") + etsClass_->GetDescriptor());
    return nullptr;
}

// Special method to ensure unwrapped object is not a JSProxy
EtsObject *EtsClassWrapper::UnwrapEtsProxy(InteropCtx *ctx, napi_value jsValue)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(!IsNullOrUndefined(env, jsValue));

    // Check if object has SharedReference
    SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *etsObject = sharedRef->GetEtsObject(ctx);
        if (UNLIKELY(!etsClass_->IsAssignableFrom(etsObject->GetClass()))) {
            ThrowJSErrorNotAssignable(env, etsObject->GetClass(), etsClass_);
            return nullptr;
        }
        if (UNLIKELY(!sharedRef->GetField<SharedReference::HasETSObject>())) {
            InteropCtx::ThrowJSTypeError(env, std::string("JS object in context of EtsProxy of class ") +
                                                  etsClass_->GetDescriptor());
            return nullptr;
        }
        return etsObject;
    }
    return nullptr;
}

void EtsClassWrapper::ThrowJSErrorNotAssignable(napi_env env, EtsClass *fromKlass, EtsClass *toKlass)
{
    const char *from = fromKlass->GetDescriptor();
    const char *to = toKlass->GetDescriptor();
    InteropCtx::ThrowJSTypeError(env, std::string(from) + " is not assignable to " + to);
}

EtsObject *EtsClassWrapper::CreateJSBuiltinProxy(InteropCtx *ctx, napi_value jsValue)
{
    ASSERT(jsproxyWrapper_ != nullptr);
    ASSERT(SharedReference::ExtractMaybeReference(ctx->GetJSEnv(), jsValue) == nullptr);

    EtsObject *etsObject = EtsObject::Create(jsproxyWrapper_->GetProxyClass());
    if (UNLIKELY(etsObject == nullptr)) {
        ctx->ForwardEtsException(EtsCoroutine::GetCurrent());
        return nullptr;
    }

    SharedReference *sharedRef = ctx->GetSharedRefStorage()->CreateJSObjectRef(ctx, etsObject, jsValue);
    if (UNLIKELY(sharedRef == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }
    return sharedRef->GetEtsObject(ctx);  // fetch again after gc
}

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass)
{
    ASSERT(!klass->IsInterface());
    EtsClass *etsClass = EtsClass::FromRuntimeClass(klass);
    EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, etsClass);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    ASSERT(wrapper->etsClass_ == etsClass);
    return std::make_unique<JSRefConvertEtsProxy>(wrapper);
}

class JSRefConvertJSProxy : public JSRefConvert {
public:
    explicit JSRefConvertJSProxy() : JSRefConvert(this) {}

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
    {
        SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        INTEROP_FATAL_IF(!storage->HasReference(etsObject));
        SharedReference *sharedRef = storage->GetReference(etsObject);
        ASSERT(sharedRef != nullptr);
        return sharedRef->GetJsObject(ctx->GetJSEnv());
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, [[maybe_unused]] napi_value jsValue)
    {
        ctx->Fatal("Unwrap called on JSProxy class");
        return nullptr;
    }
};

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertJSProxy([[maybe_unused]] InteropCtx *ctx,
                                                                         [[maybe_unused]] Class *klass)
{
    ASSERT(js_proxy::JSProxy::IsProxyClass(klass));
    return std::make_unique<JSRefConvertJSProxy>();
}

EtsMethodSet *EtsClassWrapper::GetMethod(const std::string &name) const
{
    for (const auto &item : etsMethods_) {
        if (name == item->GetName()) {
            return item.get();
        }
    }
    return nullptr;
}

/*static*/
EtsClassWrapper *EtsClassWrapper::Get(InteropCtx *ctx, EtsClass *etsClass)
{
    EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();

    ASSERT(etsClass != nullptr);
    EtsClassWrapper *etsClassWrapper = cache->Lookup(etsClass);
    if (LIKELY(etsClassWrapper != nullptr)) {
        return etsClassWrapper;
    }

    ASSERT(!etsClass->IsPrimitive() && etsClass->GetComponentType() == nullptr);
    ASSERT(ctx->GetRefConvertCache()->Lookup(etsClass->GetRuntimeClass()) == nullptr);

    if (IsStdClass(etsClass) &&
        !etsClass->IsInterface()) {  // NOTE(gogabr): temporary ugly workaround for Function... interfaces
        ctx->Fatal(std::string("ets_proxy requested for ") + etsClass->GetDescriptor() + " must add or forbid");
    }
    ASSERT(!js_proxy::JSProxy::IsProxyClass((etsClass->GetRuntimeClass())));

    std::unique_ptr<EtsClassWrapper> wrapper = EtsClassWrapper::Create(ctx, etsClass);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    return cache->Insert(etsClass, std::move(wrapper));
}

bool EtsClassWrapper::SetupHierarchy(InteropCtx *ctx, const char *jsBuiltinName)
{
    ASSERT(etsClass_->GetBase() != etsClass_);
    if (etsClass_->GetBase() != nullptr) {
        baseWrapper_ = EtsClassWrapper::Get(ctx, etsClass_->GetBase());
        if (baseWrapper_ == nullptr) {
            return false;
        }
    }

    if (jsBuiltinName != nullptr) {
        auto env = ctx->GetJSEnv();
        napi_value jsBuiltinCtor;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), jsBuiltinName, &jsBuiltinCtor));
        NAPI_CHECK_FATAL(napi_create_reference(env, jsBuiltinCtor, 1, &jsBuiltinCtorRef_));
    }
    return true;
}

std::pair<EtsClassWrapper::FieldsVec, EtsClassWrapper::MethodsVec> EtsClassWrapper::CalculateProperties(
    const OverloadsMap *overloads)
{
    auto fatalNoMethod = [](Class *klass, const uint8_t *name, const char *signature) {
        InteropCtx::Fatal(std::string("No method ") + utf::Mutf8AsCString(name) + " " + signature + " in " +
                          klass->GetName());
    };

    auto klass = etsClass_->GetRuntimeClass();
    PropsMap props;

    // Collect fields
    for (auto &f : klass->GetFields()) {
        if (f.IsPublic()) {
            props.insert({f.GetName().data, &f});
        }
    }
    // Select preferred overloads
    if (overloads != nullptr) {
        for (auto &[name, signature] : *overloads) {
            Method *method = etsClass_->GetDirectMethod(name, signature)->GetPandaMethod();
            if (UNLIKELY(method == nullptr)) {
                fatalNoMethod(klass, name, signature);
            }
            auto etsMethodSet = std::make_unique<EtsMethodSet>(EtsMethodSet::Create(method));
            auto it = props.insert({method->GetName().data, etsMethodSet.get()});
            etsMethods_.push_back(std::move(etsMethodSet));
            INTEROP_FATAL_IF(!it.second);
        }
    }

    // If class is std.core.Object
    auto klassDesc = utf::Mutf8AsCString(klass->GetDescriptor());
    if (klassDesc == panda_file_items::class_descriptors::OBJECT) {
        // Ingore all methods of std.core.Object due to names intersection with JS Object
        // Keep constructors only
        CollectConstructors(&props);
        // NOTE(shumilov-petr): Think about removing methods from std.core.Object
        // that are already presented in JS Object, others should be kept
    } else {
        // Collect methods
        CollectClassMethods(&props, overloads);
    }

    UpdatePropsWithBaseClasses(&props);

    return CalculateFieldsAndMethods(props);
}

void EtsClassWrapper::CollectConstructors(EtsClassWrapper::PropsMap *props)
{
    auto objCtors = etsClass_->GetConstructors();
    // Assuming that ETS StdLib guarantee that Object has the only one ctor
    ASSERT(objCtors.size() == 1);
    auto ctor = objCtors[0]->GetPandaMethod();
    auto etsMethodSet = std::make_unique<EtsMethodSet>(EtsMethodSet::Create(ctor));
    props->insert({ctor->GetName().data, etsMethodSet.get()});
    etsMethods_.push_back(std::move(etsMethodSet));
}

void EtsClassWrapper::CollectClassMethods(EtsClassWrapper::PropsMap *props, const OverloadsMap *overloads)
{
    auto fatalMethodOverloaded = [](Method *method) {
        for (auto &m : method->GetClass()->GetMethods()) {
            if (utf::IsEqual(m.GetName().data, method->GetName().data)) {
                INTEROP_LOG(ERROR) << "overload: " << EtsMethod::FromRuntimeMethod(&m)->GetMethodSignature(true);
            }
        }
        InteropCtx::Fatal(std::string("Method ") + utf::Mutf8AsCString(method->GetName().data) + " of class " +
                          utf::Mutf8AsCString(method->GetClass()->GetDescriptor()) + " is overloaded");
    };

    auto klass = etsClass_->GetRuntimeClass();
    for (auto &m : klass->GetMethods()) {
        if (m.IsPrivate()) {
            continue;
        }
        auto name = m.GetName().data;
        if (overloads != nullptr && overloads->find(name) != overloads->end()) {
            continue;
        }
        auto methodSet = std::make_unique<EtsMethodSet>(EtsMethodSet::Create(&m));
        auto it = props->insert({m.GetName().data, methodSet.get()});
        if (!it.second && !std::holds_alternative<EtsMethodSet *>(it.first->second)) {
            // Method overloads non-method field
            fatalMethodOverloaded(&m);
        }
        if (!it.second && std::holds_alternative<EtsMethodSet *>(it.first->second)) {
            // Possible correct method overloading: merge to existing entry
            auto addedMethods = std::get<EtsMethodSet *>(it.first->second);
            addedMethods->MergeWith(*methodSet.get());
        } else {
            etsMethods_.push_back(std::move(methodSet));
        }
    }
}

void EtsClassWrapper::UpdatePropsWithBaseClasses(EtsClassWrapper::PropsMap *props)
{
    auto hasSquashedProtoOhosImpl = [](EtsClassWrapper *wclass) {
        ASSERT(wclass->HasBuiltin() || wclass->baseWrapper_ != nullptr);
        return wclass->HasBuiltin() || wclass->baseWrapper_->HasBuiltin();
    };

    auto hasSquashedProtoOtherImpl = [](EtsClassWrapper *wclass) {
        // NOTE(vpukhov): some napi implementations add explicit receiver checks in call handler,
        //                thus method inheritance via prototype chain wont work
        (void)wclass;
        return true;
    };

#if PANDA_TARGET_OHOS
    auto hasSquashedProto = hasSquashedProtoOhosImpl;
    (void)hasSquashedProtoOtherImpl;
#else
    auto hasSquashedProto = hasSquashedProtoOtherImpl;
    (void)hasSquashedProtoOhosImpl;
#endif

    if (hasSquashedProto(this)) {
        // Copy properties of base classes if we have to split prototype chain
        for (auto wclass = baseWrapper_; wclass != nullptr; wclass = wclass->baseWrapper_) {
            for (auto &wfield : wclass->GetFields()) {
                Field *field = wfield.GetField();
                props->insert({field->GetName().data, field});
            }
            for (auto &link : wclass->GetMethods()) {
                EtsMethodSet *methodSet = link.IsResolved() ? link.GetResolved()->GetMethodSet() : link.GetUnresolved();
                props->insert({utf::CStringAsMutf8(methodSet->GetName()), methodSet});
            }

            if (hasSquashedProto(wclass)) {
                break;
            }
        }
    }
}

std::pair<EtsClassWrapper::FieldsVec, EtsClassWrapper::MethodsVec> EtsClassWrapper::CalculateFieldsAndMethods(
    const PropsMap &props)
{
    std::vector<EtsMethodSet *> methods;
    std::vector<Field *> fields;

    for (auto &[n, p] : props) {
        if (std::holds_alternative<EtsMethodSet *>(p)) {
            auto method = std::get<EtsMethodSet *>(p);
            if (method->IsConstructor() && !method->IsStatic()) {
                etsCtorLink_ = LazyEtsMethodWrapperLink(method);
            }
            if (!method->IsConstructor()) {
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

std::vector<napi_property_descriptor> EtsClassWrapper::BuildJSProperties(Span<Field *> fields,
                                                                         Span<EtsMethodSet *> methods)
{
    std::vector<napi_property_descriptor> jsProps;
    jsProps.reserve(fields.size() + methods.size());

    // Process fields
    numFields_ = fields.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    etsFieldWrappers_ = std::make_unique<EtsFieldWrapper[]>(numFields_);
    Span<EtsFieldWrapper> etsFieldWrappers(etsFieldWrappers_.get(), numFields_);
    size_t fieldIdx = 0;

    for (Field *field : fields) {
        auto wfield = &etsFieldWrappers[fieldIdx++];
        if (field->IsStatic()) {
            EtsClassWrapper *fieldWclass = LookupBaseWrapper(EtsClass::FromRuntimeClass(field->GetClass()));
            ASSERT(fieldWclass != nullptr);
            jsProps.emplace_back(wfield->MakeStaticProperty(fieldWclass, field));
        } else {
            jsProps.emplace_back(wfield->MakeInstanceProperty(this, field));
        }
    }

    // Process methods
    numMethods_ = methods.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    etsMethodWrappers_ = std::make_unique<LazyEtsMethodWrapperLink[]>(numMethods_);
    Span<LazyEtsMethodWrapperLink> etsMethodWrappers(etsMethodWrappers_.get(), numMethods_);
    size_t methodIdx = 0;

    for (auto &method : methods) {
        ASSERT(!method->IsConstructor());
        auto lazyLink = &etsMethodWrappers[methodIdx++];
        lazyLink->Set(method);
        jsProps.emplace_back(EtsMethodWrapper::MakeNapiProperty(method, lazyLink));
    }

    if (UNLIKELY(!IsEtsGlobalClass() && etsCtorLink_.GetUnresolved() == nullptr)) {
        InteropCtx::Fatal("Class " + etsClass_->GetRuntimeClass()->GetName() + " has no constructor");
    }

    return jsProps;
}

EtsClassWrapper *EtsClassWrapper::LookupBaseWrapper(EtsClass *klass)
{
    for (auto wclass = this; wclass != nullptr; wclass = wclass->baseWrapper_) {
        if (wclass->etsClass_ == klass) {
            return wclass;
        }
    }
    return nullptr;
}

static void SimulateJSInheritance(napi_env env, napi_value jsCtor, napi_value jsBaseCtor)
{
    napi_value builtinObject;
    napi_value setprotoFn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), "Object", &builtinObject));
    NAPI_CHECK_FATAL(napi_get_named_property(env, builtinObject, "setPrototypeOf", &setprotoFn));

    auto setproto = [&env, &builtinObject, &setprotoFn](napi_value obj, napi_value proto) {
        std::array args = {obj, proto};
        NAPI_CHECK_FATAL(NapiCallFunction(env, builtinObject, setprotoFn, args.size(), args.data(), nullptr));
    };

    napi_value cprototype;
    napi_value baseCprototype;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsCtor, "prototype", &cprototype));
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsBaseCtor, "prototype", &baseCprototype));

    setproto(jsCtor, jsBaseCtor);
    setproto(cprototype, baseCprototype);
}

/*static*/
std::unique_ptr<EtsClassWrapper> EtsClassWrapper::Create(InteropCtx *ctx, EtsClass *etsClass, const char *jsBuiltinName,
                                                         const OverloadsMap *overloads)
{
    auto env = ctx->GetJSEnv();

    // NOLINTNEXTLINE(readability-identifier-naming)
    auto _this = std::unique_ptr<EtsClassWrapper>(new EtsClassWrapper(etsClass));
    if (!_this->SetupHierarchy(ctx, jsBuiltinName)) {
        return nullptr;
    }

    auto [fields, methods] = _this->CalculateProperties(overloads);
    for (auto method : methods) {
        const std::string methodName(method->GetName());
        auto baseClassWrapper = _this->baseWrapper_;
        while (nullptr != baseClassWrapper) {
            EtsMethodSet *baseMethodSet = baseClassWrapper->GetMethod(methodName);
            if (nullptr != baseMethodSet) {
                method->SetBaseMethodSet(baseMethodSet);
                break;
            }
            baseClassWrapper = baseClassWrapper->baseWrapper_;
        }
    }

    auto jsProps = _this->BuildJSProperties({fields.data(), fields.size()}, {methods.data(), methods.size()});

    // NOTE(vpukhov): fatal no-public-fields check when escompat adopt accessors
    if (_this->HasBuiltin() && !fields.empty()) {
        INTEROP_LOG(ERROR) << "built-in class " << etsClass->GetDescriptor() << " has field properties";
    }
    if (_this->HasBuiltin() && etsClass->IsFinal()) {
        INTEROP_LOG(FATAL) << "built-in class " << etsClass->GetDescriptor() << " is final";
    }
    // NOTE(vpukhov): forbid "true" ets-field overriding in js-derived class, as it cannot be proxied back
    if (!etsClass->IsFinal()) {
        auto ungroupedMethods = CollectAllPandaMethods(methods.begin(), methods.end());
        _this->jsproxyWrapper_ =
            js_proxy::JSProxy::Create(etsClass, {ungroupedMethods.data(), ungroupedMethods.size()});
    }

    napi_value jsCtor {};
    NAPI_CHECK_FATAL(napi_define_class(env, etsClass->GetDescriptor(), NAPI_AUTO_LENGTH,
                                       EtsClassWrapper::JSCtorCallback, _this.get(), jsProps.size(), jsProps.data(),
                                       &jsCtor));

    auto base = _this->baseWrapper_;
    napi_value fakeSuper = _this->HasBuiltin() ? _this->GetBuiltin(env)
                                               : (base->HasBuiltin() ? base->GetBuiltin(env) : base->GetJsCtor(env));

    SimulateJSInheritance(env, jsCtor, fakeSuper);
    NAPI_CHECK_FATAL(NapiObjectSeal(env, jsCtor));
    NAPI_CHECK_FATAL(napi_create_reference(env, jsCtor, 1, &_this->jsCtorRef_));

    return _this;
}

/*static*/
napi_value EtsClassWrapper::JSCtorCallback(napi_env env, napi_callback_info cinfo)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro, env);

    napi_value jsThis;
    size_t argc;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, &jsThis, &data));
    auto etsClassWrapper = reinterpret_cast<EtsClassWrapper *>(data);

    EtsObject *etsObject = ctx->AcquirePendingNewInstance();

    if (LIKELY(etsObject != nullptr)) {
        // Create shared reference for existing ets object
        SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (UNLIKELY(!storage->CreateETSObjectRef(ctx, etsObject, jsThis))) {
            ASSERT(InteropCtx::SanityJSExceptionPending());
            return nullptr;
        }
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
        return nullptr;
    }

    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, jsArgs->data(), nullptr, nullptr));

    napi_value jsNewtarget;
    NAPI_CHECK_FATAL(napi_get_new_target(env, cinfo, &jsNewtarget));

    // create new object and wrap it
    if (UNLIKELY(!etsClassWrapper->CreateAndWrap(env, jsNewtarget, jsThis, *jsArgs))) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }

    // NOTE(ivagin): JS constructor is not required to return 'this', but ArkUI NAPI requires it
    return jsThis;
}

bool EtsClassWrapper::CreateAndWrap(napi_env env, napi_value jsNewtarget, napi_value jsThis, Span<napi_value> jsArgs)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    if (UNLIKELY(!CheckClassInitialized<true>(etsClass_->GetRuntimeClass()))) {
        ctx->ForwardEtsException(coro);
        return false;
    }

    bool notExtensible;
    NAPI_CHECK_FATAL(napi_strict_equals(env, jsNewtarget, GetJsCtor(env), &notExtensible));

    EtsClass *instanceClass {};

    if (LIKELY(notExtensible)) {
#ifndef PANDA_TARGET_OHOS
        // In case of OHOS sealed object can't be wrapped, therefore seal it after wrapping
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
#endif  // PANDA_TARGET_OHOS
        instanceClass = etsClass_;
    } else {
        if (UNLIKELY(jsproxyWrapper_ == nullptr)) {
            ctx->ThrowJSTypeError(env, std::string("Proxy for ") + etsClass_->GetDescriptor() + " is not extensible");
            return false;
        }
        instanceClass = jsproxyWrapper_->GetProxyClass();
    }

    LocalObjectHandle<EtsObject> etsObject(coro, EtsObject::Create(instanceClass));
    if (UNLIKELY(etsObject.GetPtr() == nullptr)) {
        return false;
    }

    SharedReference *sharedRef;
    if (LIKELY(notExtensible)) {
        sharedRef = ctx->GetSharedRefStorage()->CreateETSObjectRef(ctx, etsObject.GetPtr(), jsThis);
#ifdef PANDA_TARGET_OHOS
        // In case of OHOS sealed object can't be wrapped, therefore seal it after wrapping
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
#endif  // PANDA_TARGET_OHOS
    } else {
        sharedRef = ctx->GetSharedRefStorage()->CreateHybridObjectRef(ctx, etsObject.GetPtr(), jsThis);
    }
    if (UNLIKELY(sharedRef == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return false;
    }

    EtsMethodWrapper *ctorWrapper = EtsMethodWrapper::ResolveLazyLink(ctx, etsCtorLink_);
    ASSERT(ctorWrapper != nullptr);
    EtsMethod *ctorMethod = ctorWrapper->GetEtsMethod(jsArgs.Size());
    ASSERT(ctorMethod->IsInstanceConstructor());

    napi_value callRes = CallETSInstance(coro, ctx, ctorMethod->GetPandaMethod(), jsArgs, etsObject.GetPtr());
    return callRes != nullptr;
}

}  // namespace ark::ets::interop::js::ets_proxy
