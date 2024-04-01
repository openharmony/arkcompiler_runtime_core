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

#include "abc_file_utils.h"
#include <iostream>
#include "os/file.h"

namespace panda::abc2program {

bool AbcFileUtils::IsGlobalTypeName(const std::string &type_name)
{
    return (type_name == GLOBAL_TYPE_NAME);
}

bool AbcFileUtils::IsArrayTypeName(const std::string &type_name)
{
    return (type_name.find(ARRAY_TYPE_PREFIX) != std::string::npos);
}

bool AbcFileUtils::IsESTypeAnnotationName(const std::string &type_name)
{
    return (type_name == ES_TYPE_ANNOTATION_NAME);
}

bool AbcFileUtils::IsSystemTypeName(const std::string &type_name)
{
    return (IsGlobalTypeName(type_name) || IsArrayTypeName(type_name));
}

bool AbcFileUtils::IsLiteralTagArray(const panda_file::LiteralTag &tag)
{
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_F32:
        case panda_file::LiteralTag::ARRAY_F64:
        case panda_file::LiteralTag::ARRAY_STRING:
            return true;
        default:
            return false;
    }
}

std::string AbcFileUtils::GetLabelNameByInstIdx(size_t inst_idx)
{
    std::stringstream name;
    name << LABEL_PREFIX << inst_idx;
    return name.str();
}

}  // namespace panda::abc2program
