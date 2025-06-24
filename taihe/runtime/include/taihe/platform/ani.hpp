/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_
#define RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_
// NOLINTBEGIN

#include <taihe/runtime.hpp>

#include <taihe.platform.ani.proj.hpp>

namespace taihe {
// convert between ani types and taihe types

template <typename cpp_owner_t>
struct from_ani_t;

template <typename cpp_owner_t>
struct into_ani_t;

template <typename cpp_owner_t>
constexpr inline from_ani_t<cpp_owner_t> from_ani;

template <typename cpp_owner_t>
constexpr inline into_ani_t<cpp_owner_t> into_ani;
}  // namespace taihe

namespace taihe {
// Reference management

class sref_guard {
protected:
    ani_ref ref;

public:
    sref_guard(ani_env *env, ani_ref val)
    {
        env->GlobalReference_Create(val, &ref);
    }

    ~sref_guard() {}

    sref_guard(sref_guard const &) = delete;
    sref_guard &operator=(sref_guard const &) = delete;
    sref_guard(sref_guard &&) = delete;
    sref_guard &operator=(sref_guard &&) = delete;

    ani_ref get_ref()
    {
        return ref;
    }
};

class dref_guard : public sref_guard {
public:
    dref_guard(ani_env *env, ani_ref val) : sref_guard(env, val) {}

    ~dref_guard()
    {
        env_guard guard;
        ani_env *env = guard.get_env();
        env->GlobalReference_Delete(ref);
    }
};

template <typename AniRefGuard>
struct same_impl_t<AniRefGuard, std::enable_if_t<std::is_base_of_v<sref_guard, AniRefGuard>>> {
    bool operator()(data_view lhs, data_view rhs) const
    {
        auto lhs_as_ani = ::taihe::platform::ani::weak::AniObject(lhs);
        auto rhs_as_ani = ::taihe::platform::ani::weak::AniObject(rhs);
        if (lhs_as_ani.is_error() || rhs_as_ani.is_error()) {
            return false;
        }
        env_guard guard;
        ani_env *env = guard.get_env();
        ani_ref lhs_ref = reinterpret_cast<ani_ref>(lhs_as_ani->getGlobalReference());
        ani_ref rhs_ref = reinterpret_cast<ani_ref>(rhs_as_ani->getGlobalReference());
        ani_boolean result;
        return env->Reference_Equals(lhs_ref, rhs_ref, &result) == ANI_OK && result;
    }
};

template <typename AniRefGuard>
struct hash_impl_t<AniRefGuard, std::enable_if_t<std::is_base_of_v<sref_guard, AniRefGuard>>> {
    std::size_t operator()(data_view) const
    {
        TH_THROW(std::runtime_error, "Hashing of ani_ref is not implemented yet.");
    }
};
}  // namespace taihe

#if __cplusplus >= 202002L
namespace taihe {
template <std::size_t N = 0>
struct nullable_fixed_string {
    bool is_null;
    char value[N];

    constexpr nullable_fixed_string(std::nullptr_t) : is_null {true}, value {} {}

    constexpr nullable_fixed_string(char const (&sv)[N]) : is_null {false}
    {
        for (std::size_t i = 0; i < N; ++i) {
            value[i] = sv[i];
        }
    }

    constexpr char const *c_str() const
    {
        return is_null ? nullptr : value;
    }
};

template <nullable_fixed_string descriptor_t>
inline ani_module ani_find_module(ani_env *env)
{
    static sref_guard guard(env, [env]() -> ani_module {
        char const *descriptor = descriptor_t.c_str();
        ani_module mod;
        if (ANI_OK != env->FindModule(descriptor, &mod)) {
            std::cerr << "Module not found: " << descriptor << std::endl;
            return nullptr;
        }
        return mod;
    }());
    return static_cast<ani_module>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t>
inline ani_namespace ani_find_namespace(ani_env *env)
{
    static sref_guard guard(env, [env]() -> ani_namespace {
        char const *descriptor = descriptor_t.c_str();
        ani_namespace ns;
        if (ANI_OK != env->FindNamespace(descriptor, &ns)) {
            std::cerr << "Namespace not found: " << descriptor << std::endl;
            return nullptr;
        }
        return ns;
    }());
    return static_cast<ani_namespace>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t>
inline ani_class ani_find_class(ani_env *env)
{
    static sref_guard guard(env, [env]() -> ani_class {
        char const *descriptor = descriptor_t.c_str();
        ani_class cls;
        if (ANI_OK != env->FindClass(descriptor, &cls)) {
            std::cerr << "Class not found: " << descriptor << std::endl;
            return nullptr;
        }
        return cls;
    }());
    return static_cast<ani_class>(guard.get_ref());
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_function ani_find_module_function(ani_env *env)
{
    static ani_function function = [env]() -> ani_function {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        char const *signature = signature_t.c_str();
        ani_module mod = ani_find_module<descriptor_t>(env);
        if (mod == nullptr) {
            return nullptr;
        }
        ani_function fn;
        if (ANI_OK != env->Module_FindFunction(mod, name, signature, &fn)) {
            std::cerr << "Function not found: " << descriptor << "::" << name << " with signature: " << signature
                      << std::endl;
            return nullptr;
        }
        return fn;
    }();
    return function;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_function ani_find_namespace_function(ani_env *env)
{
    static ani_function function = [env]() -> ani_function {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        char const *signature = signature_t.c_str();
        ani_namespace ns = ani_find_namespace<descriptor_t>(env);
        if (ns == nullptr) {
            return nullptr;
        }
        ani_function fn;
        if (ANI_OK != env->Namespace_FindFunction(ns, name, signature, &fn)) {
            std::cerr << "Function not found: " << descriptor << "::" << name << " with signature: " << signature
                      << std::endl;
            return nullptr;
        }
        return fn;
    }();
    return function;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_method ani_find_class_method(ani_env *env)
{
    static ani_method method = [env]() -> ani_method {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        char const *signature = signature_t.c_str();
        ani_class cls = ani_find_class<descriptor_t>(env);
        if (cls == nullptr) {
            return nullptr;
        }
        ani_method mtd;
        if (ANI_OK != env->Class_FindMethod(cls, name, signature, &mtd)) {
            std::cerr << "Method not found: " << descriptor << "::" << name << " with signature: " << signature
                      << std::endl;
            return nullptr;
        }
        return mtd;
    }();
    return method;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t, nullable_fixed_string signature_t>
inline ani_static_method ani_find_class_static_method(ani_env *env)
{
    static ani_static_method method = [env]() -> ani_static_method {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        char const *signature = signature_t.c_str();
        ani_class cls = ani_find_class<descriptor_t>(env);
        if (cls == nullptr) {
            return nullptr;
        }
        ani_static_method mtd;
        if (ANI_OK != env->Class_FindStaticMethod(cls, name, signature, &mtd)) {
            std::cerr << "Static method not found: " << descriptor << "::" << name << " with signature: " << signature
                      << std::endl;
            return nullptr;
        }
        return mtd;
    }();
    return method;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_variable ani_find_module_variable(ani_env *env)
{
    static ani_variable variable = [env]() -> ani_variable {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        ani_module mod = ani_find_module<descriptor_t>(env);
        if (mod == nullptr) {
            return nullptr;
        }
        ani_variable var;
        if (ANI_OK != env->Module_FindVariable(mod, name, &var)) {
            std::cerr << "Variable not found: " << descriptor << "::" << name << std::endl;
            return nullptr;
        }
        return var;
    }();
    return variable;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_variable ani_find_namespace_variable(ani_env *env)
{
    static ani_variable variable = [env]() -> ani_variable {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        ani_namespace ns = ani_find_namespace<descriptor_t>(env);
        if (ns == nullptr) {
            return nullptr;
        }
        ani_variable var;
        if (ANI_OK != env->Namespace_FindVariable(ns, name, &var)) {
            std::cerr << "Variable not found: " << descriptor << "::" << name << std::endl;
            return nullptr;
        }
        return var;
    }();
    return variable;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_field ani_find_class_field(ani_env *env)
{
    static ani_field field = [env]() -> ani_field {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        ani_class cls = ani_find_class<descriptor_t>(env);
        if (cls == nullptr) {
            return nullptr;
        }
        ani_field fld;
        if (ANI_OK != env->Class_FindField(cls, name, &fld)) {
            std::cerr << "Field not found: " << descriptor << "::" << name << std::endl;
            return nullptr;
        }
        return fld;
    }();
    return field;
}

template <nullable_fixed_string descriptor_t, nullable_fixed_string name_t>
inline ani_static_field ani_find_class_static_field(ani_env *env)
{
    static ani_static_field field = [env]() -> ani_static_field {
        char const *descriptor = descriptor_t.c_str();
        char const *name = name_t.c_str();
        ani_class cls = ani_find_class<descriptor_t>(env);
        if (cls == nullptr) {
            return nullptr;
        }
        ani_static_field fld;
        if (ANI_OK != env->Class_FindStaticField(cls, name, &fld)) {
            std::cerr << "Static field not found: " << descriptor << "::" << name << std::endl;
            return nullptr;
        }
        return fld;
    }();
    return field;
}
}  // namespace taihe

#define TH_ANI_FIND_MODULE(env, descriptor) ::taihe::ani_find_module<descriptor>(env)
#define TH_ANI_FIND_NAMESPACE(env, descriptor) ::taihe::ani_find_namespace<descriptor>(env)
#define TH_ANI_FIND_CLASS(env, descriptor) ::taihe::ani_find_class<descriptor>(env)

#define TH_ANI_FIND_MODULE_FUNCTION(env, descriptor, name, signature) \
    ::taihe::ani_find_module_function<descriptor, name, signature>(env)
#define TH_ANI_FIND_NAMESPACE_FUNCTION(env, descriptor, name, signature) \
    ::taihe::ani_find_namespace_function<descriptor, name, signature>(env)
#define TH_ANI_FIND_CLASS_METHOD(env, descriptor, name, signature) \
    ::taihe::ani_find_class_method<descriptor, name, signature>(env)
#define TH_ANI_FIND_CLASS_STATIC_METHOD(env, descriptor, name, signature) \
    ::taihe::ani_find_class_static_method<descriptor, name, signature>(env)

#define TH_ANI_FIND_MODULE_VARIABLE(env, descriptor, name) ::taihe::ani_find_module_variable<descriptor, name>(env)
#define TH_ANI_FIND_NAMESPACE_VARIABLE(env, descriptor, name) \
    ::taihe::ani_find_namespace_variable<descriptor, name>(env)
#define TH_ANI_FIND_CLASS_FIELD(env, descriptor, name) ::taihe::ani_find_class_field<descriptor, name>(env)
#define TH_ANI_FIND_CLASS_STATIC_FIELD(env, descriptor, name) \
    ::taihe::ani_find_class_static_field<descriptor, name>(env)
#else  // __cplusplus >= 202002L
#define TH_ANI_FIND_MODULE(env, descriptor)                                   \
    ([env] {                                                                  \
        static ::taihe::sref_guard __guard(env, [env]() -> ani_module {       \
            ani_module __mod;                                                 \
            if (ANI_OK != env->FindModule(descriptor, &__mod)) {              \
                std::cerr << "Module not found: " << descriptor << std::endl; \
                return nullptr;                                               \
            }                                                                 \
            return __mod;                                                     \
        }());                                                                 \
        return static_cast<ani_module>(__guard.get_ref());                    \
    }())

#define TH_ANI_FIND_NAMESPACE(env, descriptor)                                   \
    ([env] {                                                                     \
        static ::taihe::sref_guard __guard(env, [env]() -> ani_namespace {       \
            ani_namespace __ns;                                                  \
            if (ANI_OK != env->FindNamespace(descriptor, &__ns)) {               \
                std::cerr << "Namespace not found: " << descriptor << std::endl; \
                return nullptr;                                                  \
            }                                                                    \
            return __ns;                                                         \
        }());                                                                    \
        return static_cast<ani_namespace>(__guard.get_ref());                    \
    }())

#define TH_ANI_FIND_CLASS(env, descriptor)                                   \
    ([env] {                                                                 \
        static ::taihe::sref_guard __guard(env, [env]() -> ani_class {       \
            ani_class __cls;                                                 \
            if (ANI_OK != env->FindClass(descriptor, &__cls)) {              \
                std::cerr << "Class not found: " << descriptor << std::endl; \
                return nullptr;                                              \
            }                                                                \
            return __cls;                                                    \
        }());                                                                \
        return static_cast<ani_class>(__guard.get_ref());                    \
    }())

#define TH_ANI_FIND_MODULE_FUNCTION(env, descriptor, name, signature)                                                 \
    ([env] {                                                                                                          \
        static ani_function __function = [env]() -> ani_function {                                                    \
            ani_module __mod = TH_ANI_FIND_MODULE(env, descriptor);                                                   \
            if (__mod == nullptr) {                                                                                   \
                return nullptr;                                                                                       \
            }                                                                                                         \
            ani_function __fn;                                                                                        \
            if (ANI_OK != env->Module_FindFunction(__mod, name, signature, &__fn)) {                                  \
                std::cerr << "Function not found: " << descriptor << "::" << name << " with signature: " << signature \
                          << std::endl;                                                                               \
                return nullptr;                                                                                       \
            }                                                                                                         \
            return __fn;                                                                                              \
        }();                                                                                                          \
        return __function;                                                                                            \
    }())

#define TH_ANI_FIND_NAMESPACE_FUNCTION(env, descriptor, name, signature)                                              \
    ([env] {                                                                                                          \
        static ani_function __function = [env]() -> ani_function {                                                    \
            ani_namespace __ns = TH_ANI_FIND_NAMESPACE(env, descriptor);                                              \
            if (__ns == nullptr) {                                                                                    \
                return nullptr;                                                                                       \
            }                                                                                                         \
            ani_function __fn;                                                                                        \
            if (ANI_OK != env->Namespace_FindFunction(__ns, name, signature, &__fn)) {                                \
                std::cerr << "Function not found: " << descriptor << "::" << name << " with signature: " << signature \
                          << std::endl;                                                                               \
                return nullptr;                                                                                       \
            }                                                                                                         \
            return __fn;                                                                                              \
        }();                                                                                                          \
        return __function;                                                                                            \
    }())

#define TH_ANI_FIND_CLASS_METHOD(env, descriptor, name, signature)                                                  \
    ([env] {                                                                                                        \
        static ani_method __method = [env]() -> ani_method {                                                        \
            ani_class __cls = TH_ANI_FIND_CLASS(env, descriptor);                                                   \
            if (__cls == nullptr) {                                                                                 \
                return nullptr;                                                                                     \
            }                                                                                                       \
            ani_method __mtd;                                                                                       \
            if (ANI_OK != env->Class_FindMethod(__cls, name, signature, &__mtd)) {                                  \
                std::cerr << "Method not found: " << descriptor << "::" << name << " with signature: " << signature \
                          << std::endl;                                                                             \
                return nullptr;                                                                                     \
            }                                                                                                       \
            return __mtd;                                                                                           \
        }();                                                                                                        \
        return __method;                                                                                            \
    }())

#define TH_ANI_FIND_CLASS_STATIC_METHOD(env, descriptor, name, signature)                \
    ([env] {                                                                             \
        static ani_static_method __method = [env]() -> ani_static_method {               \
            ani_class __cls = TH_ANI_FIND_CLASS(env, descriptor);                        \
            if (__cls == nullptr) {                                                      \
                return nullptr;                                                          \
            }                                                                            \
            ani_static_method __mtd;                                                     \
            if (ANI_OK != env->Class_FindStaticMethod(__cls, name, signature, &__mtd)) { \
                std::cerr << "Static method not found: " << descriptor << "::" << name   \
                          << " with signature: " << signature << std::endl;              \
                return nullptr;                                                          \
            }                                                                            \
            return __mtd;                                                                \
        }();                                                                             \
        return __method;                                                                 \
    }())

#define TH_ANI_FIND_MODULE_VARIABLE(env, descriptor, name)                                      \
    ([env] {                                                                                    \
        static ani_variable __variable = [env]() -> ani_variable {                              \
            ani_module __mod = TH_ANI_FIND_MODULE(env, descriptor);                             \
            if (__mod == nullptr) {                                                             \
                return nullptr;                                                                 \
            }                                                                                   \
            ani_variable __var;                                                                 \
            if (ANI_OK != env->Module_FindVariable(__mod, name, &__var)) {                      \
                std::cerr << "Variable not found: " << descriptor << "::" << name << std::endl; \
                return nullptr;                                                                 \
            }                                                                                   \
            return __var;                                                                       \
        }();                                                                                    \
        return __variable;                                                                      \
    }())

#define TH_ANI_FIND_NAMESPACE_VARIABLE(env, descriptor, name)                                   \
    ([env] {                                                                                    \
        static ani_variable __variable = [env]() -> ani_variable {                              \
            ani_namespace __ns = TH_ANI_FIND_NAMESPACE(env, descriptor);                        \
            if (__ns == nullptr) {                                                              \
                return nullptr;                                                                 \
            }                                                                                   \
            ani_variable __var;                                                                 \
            if (ANI_OK != env->Namespace_FindVariable(__ns, name, &__var)) {                    \
                std::cerr << "Variable not found: " << descriptor << "::" << name << std::endl; \
                return nullptr;                                                                 \
            }                                                                                   \
            return __var;                                                                       \
        }();                                                                                    \
        return __variable;                                                                      \
    }())

#define TH_ANI_FIND_CLASS_FIELD(env, descriptor, name)                                       \
    ([env] {                                                                                 \
        static ani_field __field = [env]() -> ani_field {                                    \
            ani_class __cls = TH_ANI_FIND_CLASS(env, descriptor);                            \
            if (__cls == nullptr) {                                                          \
                return nullptr;                                                              \
            }                                                                                \
            ani_field __fld;                                                                 \
            if (ANI_OK != env->Class_FindField(__cls, name, &__fld)) {                       \
                std::cerr << "Field not found: " << descriptor << "::" << name << std::endl; \
                return nullptr;                                                              \
            }                                                                                \
            return __fld;                                                                    \
        }();                                                                                 \
        return __field;                                                                      \
    }())

#define TH_ANI_FIND_CLASS_STATIC_FIELD(env, descriptor, name)                                       \
    ([env] {                                                                                        \
        static ani_static_field __field = [env]() -> ani_static_field {                             \
            ani_class __cls = TH_ANI_FIND_CLASS(env, descriptor);                                   \
            if (__cls == nullptr) {                                                                 \
                return nullptr;                                                                     \
            }                                                                                       \
            ani_static_field __fld;                                                                 \
            if (ANI_OK != env->Class_FindStaticField(__cls, name, &__fld)) {                        \
                std::cerr << "Static field not found: " << descriptor << "::" << name << std::endl; \
                return nullptr;                                                                     \
            }                                                                                       \
            return __fld;                                                                           \
        }();                                                                                        \
        return __field;                                                                             \
    }())
#endif  // __cplusplus >= 202002L
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_PLATFORM_ANI_HPP_