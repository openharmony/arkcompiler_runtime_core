/*
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

#ifndef CPP_ABCKIT_CORE_FIELD_H
#define CPP_ABCKIT_CORE_FIELD_H

#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief ModuleField
 */
class ModuleField : public View<AbckitCoreModuleField *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief abckit::DefaultHash<ModuleField>
    friend class abckit::DefaultHash<ModuleField>;

protected:
    /// @brief Core API View type
    using CoreViewT = ModuleField;

public:
    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(const ModuleField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ModuleField&
     */
    ModuleField &operator=(const ModuleField &other) = default;

    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(ModuleField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ModuleField&
     */
    ModuleField &operator=(ModuleField &&other) = default;

    /**
     * @brief Destroy the ModuleField object
     */
    ~ModuleField() override = default;

private:
    ModuleField(AbckitCoreModuleField *field, const ApiConfig *conf) : View(field), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief ClassField
 */
class ClassField : public View<AbckitCoreClassField *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief abckit::DefaultHash<ClassField>
    friend class abckit::DefaultHash<ClassField>;

protected:
    /// @brief Core API View type
    using CoreViewT = ClassField;

public:
    /**
     * @brief Construct a new ClassField object
     * @param other
     */
    ClassField(const ClassField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ClassField&
     */
    ClassField &operator=(const ClassField &other) = default;

    /**
     * @brief Construct a new Field object
     * @param other
     */
    ClassField(ClassField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ClassField&
     */
    ClassField &operator=(ClassField &&other) = default;

    /**
     * @brief Destroy the ClassField object
     */
    ~ClassField() override = default;

private:
    ClassField(AbckitCoreClassField *field, const ApiConfig *conf) : View(field), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief InterfaceField
 */
class InterfaceField : public View<AbckitCoreInterfaceField *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<InterfaceField>
    friend class abckit::DefaultHash<InterfaceField>;

protected:
    /// @brief Core API View type
    using CoreViewT = InterfaceField;

public:
    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(const InterfaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return InterfaceField&
     */
    InterfaceField &operator=(const InterfaceField &other) = default;

    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(InterfaceField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return InterfaceField&
     */
    InterfaceField &operator=(InterfaceField &&other) = default;

    /**
     * @brief Destroy the InterfaceField object
     */
    ~InterfaceField() override = default;

private:
    InterfaceField(AbckitCoreInterfaceField *field, const ApiConfig *conf) : View(field), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief EnumField
 */
class EnumField : public View<AbckitCoreEnumField *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<EnumField>
    friend class abckit::DefaultHash<EnumField>;

protected:
    /// @brief Core API View type
    using CoreViewT = EnumField;

public:
    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(const EnumField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return EnumField&
     */
    EnumField &operator=(const EnumField &other) = default;

    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(EnumField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return EnumField&
     */
    EnumField &operator=(EnumField &&other) = default;

    /**
     * @brief Destroy the EnumField object
     */
    ~EnumField() override = default;

private:
    EnumField(AbckitCoreEnumField *field, const ApiConfig *conf) : View(field), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FIELD_H
