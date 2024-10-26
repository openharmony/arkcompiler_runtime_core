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

#ifndef CPP_ABCKIT_TYPE_H
#define CPP_ABCKIT_TYPE_H

#include "cpp/headers/core/annotation.h"
#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "libabckit/include/c/metadata_core.h"

namespace abckit {

class Type : public View<AbckitType *> {
    friend class abckit::File;

public:
    Type(const Type &other) = default;
    Type &operator=(const Type &other) = default;
    Type(Type &&other) = default;
    Type &operator=(Type &&other) = default;
    ~Type() override = default;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    explicit Type(AbckitType *val, const ApiConfig *conf) : View(val), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_TYPE_H
