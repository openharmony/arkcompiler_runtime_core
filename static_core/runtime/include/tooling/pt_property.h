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
#ifndef PANDA_TOOLING_PT_PROPERTY_H
#define PANDA_TOOLING_PT_PROPERTY_H

#include "libpandabase/macros.h"

namespace ark::tooling {
class PtProperty {
public:
    explicit PtProperty(void *data = nullptr) : data_(data) {}

    void *GetData() const
    {
        return data_;
    }

    ~PtProperty() = default;

    DEFAULT_COPY_SEMANTIC(PtProperty);
    DEFAULT_MOVE_SEMANTIC(PtProperty);

private:
    void *data_;
};
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_PT_PROPERTY_H
