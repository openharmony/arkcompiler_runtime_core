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

#ifndef PANDA_GUARD_OBFUSCATE_NODE_H
#define PANDA_GUARD_OBFUSCATE_NODE_H

#include "class.h"
#include "entity.h"
#include "file_path.h"
#include "function.h"
#include "module_record.h"
#include "object.h"

namespace panda::guard {

class Node final : public Entity {
public:
    Node(Program *program, const std::string &name, std::string pkgName)
        : Entity(program, name),
          moduleRecord_(program, name),
          pkgName_(std::move(pkgName)),
          sourceName_(name),
          obfSourceName_(name)
    {
    }

    void Build() override;

    /**
     * For Each Function In Node
     * 1. Functions
     * 2. For Each Function In Classes
     */
    void ForEachFunction(const std::function<FunctionTraver> &callback);

    /**
     * Update file name references in this node
     */
    void UpdateFileNameReferences();

    /**
     * Update SourceFile with obf file name
     * SourceFile: file path in "abc record's source_file"
     */
    void UpdateSourceFile(const std::string &file);

    /**
     * update record scopeNames
     */
    void UpdateScopeNames();

    /**
     * Find PkgName field in pandasm::Record
     */
    static bool FindPkgName(const pandasm::Record &record, std::string &pkgName);

    /**
     * record is json file
     * @param record record
     * @return true, false
     */
    static bool IsJsonFile(const pandasm::Record &record);

protected:
    void RefreshNeedUpdate() override;

    void Update() override;

private:
    void ForEachIns(const InstructionInfo &info, Scope scope);

    void CreateFunction(const InstructionInfo &info, Scope scope);

    void CreateClass(const InstructionInfo &info, Scope scope);

    /**
     * e.g. method in class
     * class A { get b() {return 1;}}
     * bytecode:
     * defineclasswithbuffer
     * definemethod
     *
     * e.g. method in object
     * let obj = { get b() {return 1;}}
     * bytecode:
     * createobjectwithbuffer
     * definemethod
     */
    void CreateOuterMethod(const InstructionInfo &info);

    void CreateObject(const InstructionInfo &info, Scope scope);

    /**
     * when obj field is array, outer ins definepropertybyname will add an property for object
     * let obj = { arr:[] }
     * bytecode:
     * createobjectwithbuffer
     * sta v0
     * definepropertybyname arr v0
     * @param info instruction info
     */
    void CreateObjectOuterProperty(const InstructionInfo &info);

    /**
     * let obj = new ClassA();
     * obj.field = 1; // field not defined in ClassA, field is a outer property
     * @param info instruction info
     */
    void CreateOuterProperty(const InstructionInfo &info);

    /*
     * this instruction crosses functions and cannot be analyzed by graph, therefore it has been added to the whitelist
     * e.g.
     * class A { ['field'] = 1; }
     * bytecode:
     *  func1: stlexvar
     *  func2: defineclasswithbuffer
     */
    static void FindStLexVarName(const InstructionInfo &info);

    void CreateFilePath();

    void CreateFilePathForDefaultMode();

    void CreateFilePathForNormalizedMode();

    void ExtractNames();

    void WriteFileCache(const std::string &filePath) override;

    void UpdateRecordTable();

    void UpdateFileNameDefine();

    static void GetMethodNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo);

public:
    ModuleRecord moduleRecord_;
    FilePath filepath_;
    std::unordered_map<std::string, Function> functionTable_ {};  // key: Function idx
    std::unordered_map<std::string, Class> classTable_ {};        // key: class literalArray idx
    std::unordered_map<std::string, Object> objectTable_ {};      // key: object literalArray idx
    std::vector<Property> outerProperties_ {};
    std::set<std::string> strings_ {};
    std::string pkgName_;
    bool fileNameNeedUpdate_ = true;
    bool contentNeedUpdate_ = true;
    bool isNormalizedOhmUrl_ = false;
    std::string sourceName_;  // file path in "nameCache key" format
    std::string obfSourceName_;
    std::string sourceFile_;  // file path in "abc record's source_file"
    std::string obfSourceFile_;
    bool sourceFileUpdated_ = false;  // is sourceFile updated
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_NODE_H
