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

#ifndef ARK_GUARD_OBFUSCATOR_MEMBER_DESCRIPTOR_UTIL_H
#define ARK_GUARD_OBFUSCATOR_MEMBER_DESCRIPTOR_UTIL_H

#include <string>
#include <unordered_map>

namespace ark::guard {

/**
 * @brief MemberDescriptorUtil
 */
class MemberDescriptorUtil {
public:
    /**
     * @brief get or create new member descriptor
     * @return new member descriptor
     */
    static std::string GetOrCreateNewMemberDescriptor(const std::string &descriptor);

private:
    static std::unordered_map<std::string, std::string> memberDescriptorCache_;
};
}  // namespace ark::guard

#endif  // ARK_GUARD_OBFUSCATOR_MEMBER_DESCRIPTOR_UTIL_H
