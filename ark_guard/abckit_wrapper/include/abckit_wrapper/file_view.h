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

#ifndef ABCKIT_WRAPPER_FILE_VIEW_H
#define ABCKIT_WRAPPER_FILE_VIEW_H

#include <memory>
#include <string_view>

#include "libabckit/cpp/abckit_cpp.h"

namespace abckit_wrapper {
/**
 * @brief FileView
 * abc file view
 */
class FileView {
public:
    /**
     * @brief FileView Constructor
     */
    explicit FileView() = default;

    /**
     * @brief Init fileView with abc file path
     * @param path abc file path
     * @return int
     */
    int Init(const std::string_view &path);

private:
    /**
     * abckit::File ptr
     */
    std::unique_ptr<abckit::File> file_ = nullptr;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_FILE_VIEW_H
