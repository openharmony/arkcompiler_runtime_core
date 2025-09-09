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

#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"

namespace libabckit {

bool ModuleEnumerateNamespacesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *ns, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, ns] : m->nt) {
        if (!cb(ns.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateClassesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    for (auto &[className, klass] : m->ct) {
        if (!cb(klass.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateInterfacesHelper(AbckitCoreModule *m, void *data,
                                     bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, iface] : m->it) {
        if (!cb(iface.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateEnumsHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, enm] : m->et) {
        if (!cb(enm.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateTopLevelFunctionsHelper(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &function : m->functions) {
        if (!cb(function.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateFieldsHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreModuleField *field, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &field : m->fields) {
        if (!cb(field.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ModuleEnumerateAnnotationInterfacesHelper(AbckitCoreModule *m, void *data,
                                               bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[atName, at] : m->at) {
        if (at.get() != nullptr && !cb(at.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateNamespacesHelper(AbckitCoreNamespace *n, void *data,
                                        bool (*cb)(AbckitCoreNamespace *ns, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, ns] : n->nt) {
        if (!cb(ns.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateClassesHelper(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[className, klass] : n->ct) {
        if (!cb(klass.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateInterfacesHelper(AbckitCoreNamespace *n, void *data,
                                        bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, iface] : n->it) {
        if (!cb(iface.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateEnumsHelper(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, enm] : n->et) {
        if (!cb(enm.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateTopLevelFunctionsHelper(AbckitCoreNamespace *n, void *data,
                                               bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &function : n->functions) {
        if (!cb(function.get(), data)) {
            return false;
        }
    }
    return true;
}

bool NamespaceEnumerateFieldsHelper(AbckitCoreNamespace *n, void *data,
                                    bool (*cb)(AbckitCoreNamespaceField *field, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &field : n->fields) {
        if (!cb(field.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ClassEnumerateMethodsHelper(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    LIBABCKIT_LOG_FUNC;
    for (auto &method : klass->methods) {
        if (!cb(method.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ClassEnumerateFieldsHelper(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreClassField *field, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &field : klass->fields) {
        if (!cb(field.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ClassEnumerateSubClassesHelper(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreClass *subClass, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &subClass : klass->subClasses) {
        if (!cb(subClass, data)) {
            return false;
        }
    }
    return true;
}

bool ClassEnumerateInterfacesHelper(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &iface : klass->interfaces) {
        if (!cb(iface, data)) {
            return false;
        }
    }
    return true;
}

bool InterfaceEnumerateMethodsHelper(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreFunction *method, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &method : iface->methods) {
        if (!cb(method.get(), data)) {
            return false;
        }
    }
    return true;
}

bool InterfaceEnumerateFieldsHelper(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreInterfaceField *field, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &[_, field] : iface->fields) {
        if (!cb(field.get(), data)) {
            return false;
        }
    }
    return true;
}

bool InterfaceEnumerateSuperInterfacesHelper(AbckitCoreInterface *iface, void *data,
                                             bool (*cb)(AbckitCoreInterface *superInterface, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &superInterface : iface->superInterfaces) {
        if (!cb(superInterface, data)) {
            return false;
        }
    }
    return true;
}

bool InterfaceEnumerateSubInterfacesHelper(AbckitCoreInterface *iface, void *data,
                                           bool (*cb)(AbckitCoreInterface *subInterface, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &subInterface : iface->subInterfaces) {
        if (!cb(subInterface, data)) {
            return false;
        }
    }
    return true;
}

bool InterfaceEnumerateClassesHelper(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreClass *klass, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &klass : iface->classes) {
        if (!cb(klass, data)) {
            return false;
        }
    }
    return true;
}

bool EnumEnumerateMethodsHelper(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(enm, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &method : enm->methods) {
        if (!cb(method.get(), data)) {
            return false;
        }
    }
    return true;
}

bool EnumEnumerateFieldsHelper(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(enm, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &field : enm->fields) {
        if (!cb(field.get(), data)) {
            return false;
        }
    }
    return true;
}

bool IsDynamic(AbckitTarget target)
{
    return target == ABCKIT_TARGET_JS || target == ABCKIT_TARGET_ARK_TS_V1;
}

static size_t HashCombine(size_t seed, size_t value)
{
    const auto m = uint64_t {0xC6A4A7935BD1E995};
    const uint32_t r = 47;

    value *= m;
    value ^= value >> r;
    value *= m;

    seed ^= value;
    seed *= m;
    return seed;
}

AbckitType *GetOrCreateType(AbckitFile *file, AbckitTypeId id, size_t rank, AbckitCoreClass *klass)
{
    size_t hash = 0;
    hash = HashCombine(hash, (size_t)id);
    hash = HashCombine(hash, rank);
    hash = HashCombine(hash, reinterpret_cast<size_t>(klass));

    auto &cache = file->types;
    if (cache.count(hash) == 1) {
        return cache[hash].get();
    }

    auto type = std::make_unique<AbckitType>();
    type->id = id;
    type->rank = rank;
    type->klass = klass;
    cache.insert({hash, std::move(type)});
    return cache[hash].get();
}

}  // namespace libabckit
