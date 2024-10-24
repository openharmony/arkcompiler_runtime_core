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

#ifndef LIBABCKIT_TESTS_VM_HELPERS
#define LIBABCKIT_TESTS_VM_HELPERS

#include <iostream>
#include <sstream>
#include <string>

namespace libabckit::test::helpers {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class CoutRedirect {
public:
    explicit CoutRedirect(std::streambuf *newBuffer) : oldBuffer_(std::cout.rdbuf(newBuffer)) {}

    ~CoutRedirect()
    {
        std::cout.rdbuf(oldBuffer_);
    }

private:
    std::streambuf *oldBuffer_;
};

std::string ExecuteDynamicAbc(const std::string &abcPath, const std::string &entry);
std::string ExecuteStaticAbc(const std::string &abcPath, const std::string &klassName, const std::string &methodName);

}  // namespace libabckit::test::helpers

#endif  // LIBABCKIT_TESTS_ETS_VM_HELPERS
