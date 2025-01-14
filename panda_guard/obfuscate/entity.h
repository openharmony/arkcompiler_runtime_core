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

#ifndef PANDA_GUARD_OBFUSCATE_ENTITY_H
#define PANDA_GUARD_OBFUSCATE_ENTITY_H

#include "assembly-program.h"

namespace panda::guard {

enum Scope {
    NONE,
    TOP_LEVEL,
    FUNCTION,
};

class IExtractNames {
public:
    virtual ~IExtractNames() = default;

    /**
     * extract all export names
     * @param strings strings set
     */
    virtual void ExtractNames(std::set<std::string> &strings) const = 0;
};

class Function;

enum class FunctionType;

using FunctionTraver = void(Function &function);

/**
 * Instruction information
 * Due to STL's memory reallocation mechanism, internal pointers are only accessible during traversal
 */
class InstructionInfo {
public:
    explicit InstructionInfo() = default;

    InstructionInfo(Function *func, pandasm::Ins *ins, size_t index) : function_(func), ins_(ins), index_(index) {}

    /**
     * Determine whether the instruction uses registers within the function
     */
    [[nodiscard]] bool IsInnerReg() const;

    /**
     * Determine whether it is valid instruction information
     */
    [[nodiscard]] bool IsValid() const;

    void UpdateInsName(bool generateNewName = true);

    void UpdateInsFileName();

    void WriteNameCache();

    Function *function_ = nullptr;
    pandasm::Ins *ins_ = nullptr;
    size_t index_ = 0;

private:
    std::string origin_ {};
    std::string obfName_ {};
};

class Program;

class IEntity {
public:
    virtual ~IEntity() = default;

    /**
     * Create Entity
     */
    virtual void Create() = 0;

    /**
     * Obfuscate Entity
     */
    virtual void Obfuscate() = 0;
};

class Entity : public IEntity {
public:
    explicit Entity(Program *program) : program_(program) {}

    Entity(Program *program, const std::string &name) : program_(program), name_(name), obfName_(name) {}

    /**
     * get Entity is export
     * @return isExport
     */
    [[nodiscard]] bool IsExport() const;

    /**
     * Create Entity
     * 1. Build Entity
     * 2. RefreshNeedUpdate
     */
    void Create() final;

    /**
     * Obfuscate Entity
     * 1. needUpdate true->2 false->return
     * 2、Update name
     * @return result code
     */
    void Obfuscate() final;

    /**
     * Write NameCacheInfo to NameCacheEntity
     * 1. Write NameCache to FileCache
     * 2. Write NameCache to PropertyCache
     * @param filePath File name before obfuscation
     */
    virtual void WriteNameCache(const std::string &filePath);

    /**
     * origin name of Entity
     */
    [[nodiscard]] virtual std::string GetName() const;

    /**
     * obf name of Entity
     */
    [[nodiscard]] virtual std::string GetObfName() const;

protected:
    /**
     * Build Entity
     */
    virtual void Build();

    /**
     * Refresh needUpdate by configs
     */
    virtual void RefreshNeedUpdate();

    /**
     * Write obfuscation mapping to the IdentifierCache and MemberMethodCache nodes of the obfuscated files in the
     * NameCache file
     * @param filePath File name before obfuscation
     */
    virtual void WriteFileCache(const std::string &filePath);

    /**
     * Write obfuscation mapping to the PropertyCache nodes of the obfuscated files in the NameCache
     */
    virtual void WritePropertyCache();

    /**
     * Update Entity name
     */
    virtual void Update();

    /**
     * set nameCacheScope
     * 1. SCOPE=TOP_LEVEL nameCacheScope: name
     * 2. SCOPE=FUNCTION nameCacheScope: functionScope#name
     * @param name nameCache name
     */
    void SetNameCacheScope(const std::string &name);

    [[nodiscard]] std::string GetNameCacheScope() const;

public:
    Program *program_ = nullptr;
    Scope scope_ = NONE;
    std::string nameCacheScope_;
    std::string name_;
    std::string obfName_;
    bool export_ = false;
    bool needUpdate = true;
    bool obfuscated = false;
    // define instruction list, when entity define in finally block, define ins maybe occurs multiple times
    std::vector<InstructionInfo> defineInsList_ {};
};

class TopLevelOptionEntity : public Entity {
public:
    static bool NeedUpdate(const Entity &entity);

    static void WritePropertyCache(const Entity &entity);

    explicit TopLevelOptionEntity(Program *program) : Entity(program) {}

    void WritePropertyCache() override;

protected:
    void RefreshNeedUpdate() override;
};

class PropertyOptionEntity : public Entity {
public:
    static bool NeedUpdate(const Entity &entity);

    static void WritePropertyCache(const Entity &entity);

    explicit PropertyOptionEntity(Program *program) : Entity(program) {}

    void WritePropertyCache() override;

protected:
    void RefreshNeedUpdate() override;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_ENTITY_H
