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

#ifndef ARK_GUARD_OBFUSCATOR_MEMBER_PREPROCESSOR_H
#define ARK_GUARD_OBFUSCATOR_MEMBER_PREPROCESSOR_H

#include "abckit_wrapper/file_view.h"

#include "name_mapping/name_mapping_manager.h"

namespace ark::guard {

/**
 * @brief MemberPreProcessor
 */
class MemberPreProcessor final : public abckit_wrapper::ModuleVisitor,
                                 public abckit_wrapper::ChildVisitor,
                                 public abckit_wrapper::MemberVisitor {
public:
    /**
     * @brief constructor, initializes nameMappingManager
     * @param nameMappingManager nameMappingManager
     */
    explicit MemberPreProcessor(NameMappingManager &nameMappingManager) : nameMappingManager_(nameMappingManager) {}

    /**
     * @brief Preparation before obfuscated. collect used member name, init member data
     * @param fileView abc fileView
     * @return true: success, false: failed
     */
    bool Process(abckit_wrapper::FileView &fileView);

private:
    bool Visit(abckit_wrapper::Module *module) override;

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override;

    bool VisitClass(abckit_wrapper::Class *clazz) override;

    bool Visit(abckit_wrapper::Member *member) override;

    NameMappingManager &nameMappingManager_;
};
}  // namespace ark::guard

#endif  // ARK_GUARD_OBFUSCATOR_MEMBER_PREPROCESSOR_H
