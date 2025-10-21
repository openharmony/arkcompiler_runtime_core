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

#ifndef ARK_GUARD_OBFUSCATOR_OBFUSCATOR_DATA_MANAGER_H
#define ARK_GUARD_OBFUSCATOR_OBFUSCATOR_DATA_MANAGER_H

#include <memory>
#include <string>

#include "abckit_wrapper/file_view.h"

#include "name_cache/name_cache_constants.h"

namespace ark::guard {
/**
 * @brief ObfuscatorData
 */
class ObfuscatorData {
public:
    /**
     * @brief Is Keep
     * @return true: keep, false: not keep
     */
    [[nodiscard]] bool IsKept() const
    {
        return this->isKept_;
    }

    /**
     * @brief set is keep
     */
    void SetKept()
    {
        this->isKept_ = true;
    }

    /**
     * @brief set name
     * @param name origin name
     */
    void SetName(const std::string &name)
    {
        this->name_ = name;
    }

    /**
     * @brief get name
     * @return name origin name
     */
    [[nodiscard]] std::string GetName() const
    {
        return this->name_;
    }

    /**
     * @brief set obfuscated name
     * @param obfName obfuscated name
     */
    void SetObfName(const std::string &obfName)
    {
        this->obfName_ = obfName;
    }

    /**
     * @brief get obfuscated name
     * @return std::string obfuscated name
     */
    [[nodiscard]] std::string GetObfName() const
    {
        return this->obfName_;
    }

    /**
     * @brief set linked member
     * @param member linked member
     */
    void SetMember(abckit_wrapper::Member *member)
    {
        this->member_ = member;
    }

    /**
     * @brief get linked member
     * @return abckit_wrapper::Member * linked member
     */
    [[nodiscard]] abckit_wrapper::Member *GetMember() const
    {
        return this->member_;
    }

    /**
     * @brief set object name cache
     * @param objectNameCache object name cache
     */
    void SetObjectNameCache(const std::shared_ptr<ObjectNameCache> &objectNameCache)
    {
        this->objectNameCache_ = objectNameCache;
    }

    /**
     * @brief get object name cache
     * @return abckit_wrapper::Member * linked member
     */
    [[nodiscard]] std::shared_ptr<ObjectNameCache> GetObjectNameCache() const
    {
        return this->objectNameCache_;
    }

    /**
     * @brief set indent
     * @param indent indent
     */
    void SetIndent(const std::string &indent)
    {
        this->indent_ = indent;
    }

    /**
     * @brief get indent
     * @return std::string indent
     */
    [[nodiscard]] std::string GetIndent() const
    {
        return this->indent_;
    }

    /**
     * @brief set kept path
     * @param path kept path
     */
    void SetKeptPath(const std::string &path)
    {
        this->keptPath_ = path;
    }

    /**
     * @brief get kept path
     * @return std::string kept path
     */
    [[nodiscard]] std::string GetKeptPath() const
    {
        return this->keptPath_;
    }

private:
    // Mark whether the object has been kept, with a default value of false, indicating that it has not been kept.
    // This field is used in the obfuscation phase
    bool isKept_ = false;

    // Store the name of the object before obfuscation
    // This field is used for name cache dump after obfuscation
    std::string name_;

    // Store the obfuscated name of the object,
    // This field is used for the obfuscation phase and the obfuscated name_cache dump
    std::string obfName_;

    // When an object is a member, store the next member object associated with that object
    // This field is used in the obfuscation phase
    std::string indent_;

    // When an object is a module, store its kept path part; end with a '.'
    // This field is used in the obfuscation/nameMapping phase
    std::string keptPath_;

    // This field is used in the obfuscation phase
    abckit_wrapper::Member *member_ = nullptr;

    // Identify the indentation before the output of this object
    // This field is used to obfuscate the post print seeds stage
    std::shared_ptr<ObjectNameCache> objectNameCache_ = nullptr;
};

/**
 * @brief ObfuscatorDataManager
 * Manage ObfuscatorData and provide interfaces for operating ObfuscatorData
 */
class ObfuscatorDataManager {
public:
    /**
     * @brief constructor, initializes object
     * @param object the constructor will set ObfuscatorData to object->data_
     */
    explicit ObfuscatorDataManager(abckit_wrapper::Object *object);

    /**
     * @brief Get Object Data
     * @return std::shared_ptr<ObfuscatorData>  return a shared point to ObfuscatorData
     */
    std::shared_ptr<ObfuscatorData> GetData();

    /**
     * @brief Get Object Parent Data
     * @return std::shared_ptr<ObfuscatorData>  return a shared point to the object parent ObfuscatorData
     */
    std::shared_ptr<ObfuscatorData> GetParentData();

    /**
     * @brief Set Object Name and Obfuscated Name
     * @param name object name
     * @param obfName object obfuscated name
     */
    void SetNameAndObfuscatedName(const std::string &name, const std::string &obfName);

private:
    abckit_wrapper::Object *object_ = nullptr;
};
} // namespace ark::guard

#endif  // ARK_GUARD_OBFUSCATOR_OBFUSCATOR_DATA_MANAGER_H