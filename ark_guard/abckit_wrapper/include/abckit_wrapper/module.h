/**
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

#ifndef ABCKIT_WRAPPER_MODULE_H
#define ABCKIT_WRAPPER_MODULE_H

#include "base_module.h"
#include "namespace.h"
#include "class.h"
#include "visitor.h"

namespace abckit_wrapper {
/**
 * @brief Module
 */
class Module final : public BaseModule {
public:
    /**
     * @brief Module Constructor
     * @param module abckit::core::Module
     * @param objectPool objectPool reference
     */
    explicit Module(abckit::core::Module module, ObjectPool &objectPool)
        : BaseModule(std::move(module)), objectPool_(objectPool)
    {
    }

    /**
     * Add object to object pool
     * @tparam T object type
     * @param object object unique ptr
     */
    template <typename T>
    void Add(std::unique_ptr<T> &&object)
    {
        const auto &objectName = object->GetFullyQualifiedName();
        this->typedObjectTable_.emplace(objectName, object.get());
        this->objectPool_.Add(std::move(object));
    }

    /**
     * Get object ptr by object full name
     * @tparam T object type
     * @param objectName object full name
     * @return object ptr
     */
    template <typename T>
    std::optional<T *> Get(const std::string &objectName) const
    {
        if (this->typedObjectTable_.find(objectName) == this->typedObjectTable_.end()) {
            return std::nullopt;
        }

        auto value = this->typedObjectTable_.at(objectName);
        if (auto object = std::get_if<T *>(&value)) {
            return *object;
        }

        return std::nullopt;
    }

    /**
     * @brief Accept visit
     * @param visitor ModuleVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(ModuleVisitor &visitor);

    /**
     * @brief Module namespaces accept visit
     * @param visitor NamespaceVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool NamespacesAccept(NamespaceVisitor &visitor);

    /**
     * @brief Module classes accept visit
     * @param visitor ClassVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool ClassesAccept(ClassVisitor &visitor);

private:
    template <typename T>
    bool TypedObjectsAccept(T &visitor);

    void InitForObject(Object *object) override;

    ObjectPool &objectPool_;
    std::unordered_map<std::string, Child> typedObjectTable_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_MODULE_H
