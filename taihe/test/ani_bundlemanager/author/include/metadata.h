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
#ifndef METADATA_H
#define METADATA_H

#include "stdexcept"
#include <string>

using namespace taihe;

class MetadataImpl {
public:
    std::string name = "metadate.name";
    std::string value = "metadate.value";
    std::string resource = "metadate.resource";
    int32_t metadataImpl = 21474;
    MetadataImpl() {}

    std::string GetName()
    {
        return name;
    }

    void SetName(string_view name)
    {
        this->name = name;
    }

    std::string GetValue()
    {
        return value;
    }

    void SetValue(string_view value)
    {
        this->value = value;
    }

    std::string GetResource()
    {
        return resource;
    }

    void SetResource(string_view resource)
    {
        this->resource = resource;
    }

    int32_t GetValueId()
    {
        return metadataImpl;
    }
};

#endif