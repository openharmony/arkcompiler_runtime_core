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

#ifndef ARK_GUARD_OBFUSCATOR_OBFUSCATOR_H
#define ARK_GUARD_OBFUSCATOR_OBFUSCATOR_H

#include "abckit_wrapper/file_view.h"

#include "configuration/configuration.h"

namespace ark::guard {

class Obfuscator {
public:
    explicit Obfuscator(Configuration config) : config_(std::move(config)) {}

    bool Execute(abckit_wrapper::FileView &fileView);

private:
    Configuration config_ {};
};
}  // namespace ark::guard

#endif  // ARK_GUARD_OBFUSCATOR_OBFUSCATOR_H
