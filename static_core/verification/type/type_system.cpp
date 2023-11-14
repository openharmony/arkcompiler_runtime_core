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

#include "verification/type/type_system.h"

#include "runtime/include/thread_scopes.h"
#include "verification/jobs/job.h"
#include "verification/jobs/service.h"
#include "verification/public_internal.h"
#include "verification/plugins.h"
#include "verifier_messages.h"

namespace panda::verifier {

static PandaString ClassNameToDescriptorString(char const *name)
{
    PandaString str = "L";
    for (char const *s = name; *s != '\0'; s++) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        char c = (*s == '.') ? '/' : *s;
        str += c;
    }
    str += ';';
    return str;
}

TypeSystem::TypeSystem(VerifierService *service, panda_file::SourceLang lang)
    : service_ {service},
      plugin_ {plugin::GetLanguagePlugin(lang)},
      lang_ctx_ {panda::plugins::GetLanguageContextBase(lang)},
      linker_ctx_ {service->GetClassLinker()->GetExtension(LanguageContext {lang_ctx_})->GetBootContext()}
{
    ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
    auto compute = [&](uint8_t const *descr) -> Type {
        if (descr != nullptr) {
            Job::ErrorHandler handler;
            auto *klass = service->GetClassLinker()->GetClass(descr, true, linker_ctx_, &handler);
            if (klass == nullptr) {
                return Type {};
            }
            return Type {klass};
        }
        return Type {};
    };

    class_ = compute(lang_ctx_.GetClassClassDescriptor());
    object_ = compute(lang_ctx_.GetObjectClassDescriptor());
    // Throwable is not given to us as descriptor for some reason. NOTE(gogabr): correct this.
    auto throwable_class_name = lang_ctx_.GetVerificationTypeThrowable();
    if (throwable_class_name != nullptr) {
        auto throwable_descr_str = ClassNameToDescriptorString(throwable_class_name);
        throwable_ = compute(utf::CStringAsMutf8(throwable_descr_str.c_str()));
    } else {
        throwable_ = Type {};
    }
    plugin_->TypeSystemSetup(this);
}

Class const *TypeSystem::DescriptorToClass(uint8_t const *descr)
{
    Job::ErrorHandler handler;
    return service_->GetClassLinker()->GetClass(descr, true, linker_ctx_, &handler);
}

Type TypeSystem::DescriptorToType(uint8_t const *descr)
{
    auto cls = DescriptorToClass(descr);
    if (cls == nullptr) {
        return Type {};
    }
    return Type {cls};
}

void TypeSystem::ExtendBySupers(PandaUnorderedSet<Type> *set, Class const *klass)
{
    // NOTE(gogabr): do we need to cache intermediate results? Measure!
    Type new_tp = Type {klass};
    if (set->count(new_tp) > 0) {
        return;
    }
    set->insert(new_tp);

    Class const *super = klass->GetBase();
    if (super != nullptr) {
        ExtendBySupers(set, super);
    }

    for (auto const *a : klass->GetInterfaces()) {
        ExtendBySupers(set, a);
    }
}

Type TypeSystem::NormalizedTypeOf(Type type)
{
    ASSERT(!type.IsNone());
    if (type == Type::Bot() || type == Type::Top()) {
        return type;
    }
    auto t = normalized_type_of_.find(type);
    if (t != normalized_type_of_.cend()) {
        return t->second;
    }
    Type result = plugin_->NormalizeType(type, this);
    normalized_type_of_[type] = result;
    return result;
}

MethodSignature const *TypeSystem::GetMethodSignature(Method const *method)
{
    ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
    auto &&method_id = method->GetUniqId();
    auto it = signature_of_method_.find(method_id);
    if (it != signature_of_method_.end()) {
        return &it->second;
    }

    method_of_id_[method_id] = method;

    MethodSignature sig;
    if (method->GetReturnType().IsReference()) {
        sig.result = DescriptorToType(method->GetRefReturnType().data);
    } else {
        sig.result = Type::FromTypeId(method->GetReturnType().GetId());
    }
    size_t ref_idx = 0;
    for (size_t i = 0; i < method->GetNumArgs(); i++) {
        Type arg_type;
        if (method->GetArgType(i).IsReference()) {
            arg_type = DescriptorToType(method->GetRefArgType(ref_idx++).data);
        } else {
            arg_type = Type::FromTypeId(method->GetArgType(i).GetId());
        }
        sig.args.push_back(arg_type);
    }
    signature_of_method_[method_id] = sig;
    return &signature_of_method_[method_id];
}

PandaUnorderedSet<Type> const *TypeSystem::SupertypesOfClass(Class const *klass)
{
    ASSERT(!klass->IsArrayClass());
    if (supertypes_cache_.count(klass) > 0) {
        return &supertypes_cache_[klass];
    }

    PandaUnorderedSet<Type> to_cache;
    ExtendBySupers(&to_cache, klass);
    supertypes_cache_[klass] = std::move(to_cache);
    return &supertypes_cache_[klass];
}

void TypeSystem::MentionClass(Class const *klass)
{
    known_classes_.insert(klass);
}

void TypeSystem::DisplayTypeSystem(std::function<void(PandaString const &)> const &handler)
{
    handler("Classes:");
    DisplayClasses([&handler](auto const &name) { handler(name); });
    handler("Methods:");
    DisplayMethods([&handler](auto const &name, auto const &sig) { handler(name + " : " + sig); });
    handler("Subtyping (type <; supertype)");
    DisplaySubtyping([&handler](auto const &type, auto const &supertype) { handler(type + " <: " + supertype); });
}

void TypeSystem::DisplayClasses(std::function<void(PandaString const &)> const &handler) const
{
    for (auto const *klass : known_classes_) {
        handler((Type {klass}).ToString(this));
    }
}

void TypeSystem::DisplayMethods(std::function<void(PandaString const &, PandaString const &)> const &handler) const
{
    for (auto const &it : signature_of_method_) {
        auto &id = it.first;
        auto &sig = it.second;
        auto const *method = method_of_id_.at(id);
        handler(method->GetFullName(), sig.ToString(this));
    }
}

void TypeSystem::DisplaySubtyping(std::function<void(PandaString const &, PandaString const &)> const &handler)
{
    for (auto const *klass : known_classes_) {
        if (klass->IsArrayClass()) {
            continue;
        }
        auto klass_str = (Type {klass}).ToString(this);
        for (auto const &supertype : *SupertypesOfClass(klass)) {
            handler(klass_str, supertype.ToString(this));
        }
    }
}

}  // namespace panda::verifier
