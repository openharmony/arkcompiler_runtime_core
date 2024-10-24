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

#ifndef CPP_ABCKIT_LITERAL_ARRAY_H
#define CPP_ABCKIT_LITERAL_ARRAY_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "libabckit/include/c/metadata_core.h"

#include <vector>

namespace abckit {

class LiteralArray : public View<AbckitLiteralArray *> {
    friend class abckit::File;
    friend class abckit::Literal;

public:
    LiteralArray(const LiteralArray &other) = default;
    LiteralArray &operator=(const LiteralArray &other) = default;
    LiteralArray(LiteralArray &&other) = default;
    LiteralArray &operator=(LiteralArray &&other) = default;
    ~LiteralArray() override = default;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    LiteralArray(AbckitLiteralArray *lita, const ApiConfig *conf) : View(lita), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_ARRAY_H
