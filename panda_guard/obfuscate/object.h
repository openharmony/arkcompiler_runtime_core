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

#ifndef PANDA_GUARD_OBFUSCATE_OBJECT_H
#define PANDA_GUARD_OBFUSCATE_OBJECT_H

#include "entity.h"
#include "method.h"
#include "module_record.h"

namespace panda::guard {

class ObjectProperty final : public PropertyOptionEntity, public IExtractNames {
public:
    ObjectProperty(Program *program, std::string literalArrayIdx)
        : PropertyOptionEntity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void ExtractNames(std::set<std::string> &strings) const override;

    /**
     * 设置 内容是否应该混淆
     */
    void SetContentNeedUpdate(bool toUpdate);

protected:
    void Build() override;

    void Update() override;

public:
    std::string literalArrayIdx_;
    size_t index_ = 0;
    std::shared_ptr<PropertyMethod> method_ = nullptr;
    bool contentNeedUpdate_ = false;
};

class Object final : public Entity, public IExtractNames {
public:
    Object(Program *program, std::string literalArrayIdx)
        : Entity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void Build() override;

    /**
     * 遍历所有方法
     * @param callback 方法回调
     */
    void ForEachMethod(const std::function<FunctionTraver> &callback);

    void ExtractNames(std::set<std::string> &strings) const override;

    void WriteNameCache(const std::string &filePath) override;

    /**
     * 设置 内容是否应该混淆
     */
    void SetContentNeedUpdate(bool toUpdate);

protected:
    void RefreshNeedUpdate() override;

    void Update() override;

private:
    void CreateProperty(const pandasm::LiteralArray &literalArray, size_t index, bool isMethod);

public:
    ModuleRecord *moduleRecord = nullptr;
    bool needUpdateName_ = false;
    bool contentNeedUpdate_ = false;
    std::string literalArrayIdx_;
    std::vector<ObjectProperty> properties_ {};
    std::vector<OuterMethod> outerMethods_ {};
    std::vector<Property> outerProperties_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_OBJECT_H
