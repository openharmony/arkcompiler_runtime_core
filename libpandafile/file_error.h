/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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


#ifndef FILE_ERRORS_H
#define FILE_ERRORS_H

#include <string_view>

namespace panda::panda_file {

class FileError {
public:
    // File-related error messages
    static constexpr std::string_view INVALID_FILE_OFFSET = "Invalid file offset";
    static constexpr std::string_view INVALID_OFFSET_WITHIN_HEADER =
        "Invalid file offset: offset falls within the header";
    static constexpr std::string_view OFFSET_OUT_OF_FILE = "Invalid file offset: offset out of file";
    static constexpr std::string_view NOT_ENOUGH_SP_SIZE = "Invalid file offset: not enough sp size";

    // Index header related error messages
    static constexpr std::string_view NULL_INDEX_HEADER = "index_header is null";
    static constexpr std::string_view INVALID_INDEX_HEADER = "index_header is invalid";
    static constexpr std::string_view INDEX_OFFSET_OUT_OF_BOUNDS = "index_header is invalid: index offset out of file";
    static constexpr std::string_view INDEX_SIZE_OUT_OF_FILE_BOUNDS =
        "index_header is invalid: index size out of file size";
    static constexpr std::string_view INDEX_OUT_OF_BOUNDS = "index_header is invalid: index out of file";

    // Span related error messages
    static constexpr std::string_view INVALID_SPAN_OFFSET = "Invalid span offset";
    static constexpr std::string_view SPAN_SIZE_IS_ZERO = "Invalid span offset: span size is zero";
    static constexpr std::string_view INVALID_LEB128_ENCODING =
        "Invalid span offset: byte does not conform to the LEB128 encoding rules";
    static constexpr std::string_view NOT_ENOUGH_SPAN_SIZE = "Invalid span offset: span size is less than expected";

    static constexpr std::string_view GET_CLASS_INDEX = "GetClassIndex";
    static constexpr std::string_view GET_METHOD_INDEX = "GetMethodIndex";
    static constexpr std::string_view GET_FIELD_INDEX = "GetFieldIndex";
    static constexpr std::string_view GET_PROTO_INDEX = "GetProtoIndex";

    static constexpr std::string_view ANNOTATION_DATA_ACCESSOR = "AnnotationDataAccessor";
    static constexpr std::string_view CLASS_DATA_ACCESSOR = "ClassDataAccessor";
    static constexpr std::string_view CODE_DATA_ACCESSOR = "CodeDataAccessor";
    static constexpr std::string_view FIELD_DATA_ACCESSOR = "FieldDataAccessor";
    static constexpr std::string_view GET_SPAN_FROM_ID = "GetSpanFromId";

    static constexpr std::string_view READ = "Read";
    static constexpr std::string_view READULEB128 = "ReadULEB128";
};
} // namespace FileErrors

#endif // FILE_ERRORS_H