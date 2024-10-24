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

#ifndef CPP_ABCKIT_VALUE_H
#define CPP_ABCKIT_VALUE_H

#include "cpp/headers/core/annotation.h"
#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "libabckit/include/c/metadata_core.h"

namespace abckit {

class Value : public View<AbckitValue *> {
    friend class abckit::File;
    friend class abckit::core::Annotation;
    friend class abckit::arkts::Annotation;

public:
    Value(const Value &other) = default;
    Value &operator=(const Value &other) = default;
    Value(Value &&other) = default;
    Value &operator=(Value &&other) = default;
    ~Value() override = default;

    bool GetU1() const
    {
        bool ret = GetApiConfig()->cIapi_->valueGetU1(GetView());
        CheckError(GetApiConfig());
        return ret;
    }

    double GetDouble() const
    {
        double ret = GetApiConfig()->cIapi_->valueGetDouble(GetView());
        CheckError(GetApiConfig());
        return ret;
    }

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    explicit Value(AbckitValue *val, const ApiConfig *conf) : View(val), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_VALUE_H
