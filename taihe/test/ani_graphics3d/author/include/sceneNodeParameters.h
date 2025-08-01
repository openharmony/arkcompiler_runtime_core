/*
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
#ifndef SCENENODEPARAMETERS_H
#define SCENENODEPARAMETERS_H

#include "stdexcept"
#include <string>

using namespace taihe;

class SceneNodeParametersImpl {
public:
    string name = "name";
    optional<string> path;
    SceneNodeParametersImpl() {}

    string GetName()
    {
        return name;
    }

    void SetName(string_view name)
    {
        this->name = name;
    }

    optional<string> GetPath()
    {
        return path;
    }

    void SetPath(optional_view<string> name)
    {
        this->path = path;
    }
};
#endif