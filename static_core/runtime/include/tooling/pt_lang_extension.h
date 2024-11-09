/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_PT_LANG_EXTENSION_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_PT_LANG_EXTENSION_H

#include "libpandabase/macros.h"
#include "libpandafile/file_items.h"
#include "runtime/include/typed_value.h"

#include <optional>
#include <functional>
#include <string>

namespace ark::tooling {
class PtLangExt {
public:
    using PropertyHandler =
        std::function<void(const std::string &name, TypedValue value, bool isFinal, bool isAccessor)>;

public:
    PtLangExt() = default;
    virtual ~PtLangExt() = default;

    NO_COPY_SEMANTIC(PtLangExt);
    NO_MOVE_SEMANTIC(PtLangExt);

    virtual std::string GetClassName(const ObjectHeader *object) = 0;
    virtual std::optional<std::string> GetAsString(const ObjectHeader *object) = 0;
    virtual std::optional<size_t> GetLengthIfArray(const ObjectHeader *object) = 0;
    virtual void EnumerateProperties(const ObjectHeader *object, const PropertyHandler &handler) = 0;
    virtual void EnumerateGlobals(const PropertyHandler &handler) = 0;
    virtual std::string_view GetThisParameterName() const
    {
        return "this";
    }
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_PT_LANG_EXTENSION_H
