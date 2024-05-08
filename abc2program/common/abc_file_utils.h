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

#ifndef ABC2PROGRAM_ABC_FILE_UTILS_H
#define ABC2PROGRAM_ABC_FILE_UTILS_H

#include <string>
#include <assembly-program.h>
#include "file.h"

namespace panda::abc2program {

// function_kind_mask
constexpr uint32_t MASK_9_TO_16_BITS = 0x0000FF00;
constexpr uint32_t LOWEST_8_BIT_MASK = 0x000000FF;
constexpr uint32_t BITS_8_SHIFT = 8;

// type name
constexpr std::string_view GLOBAL_TYPE_NAME = "_GLOBAL";
constexpr std::string_view ARRAY_TYPE_PREFIX = "[";
constexpr std::string_view ANY_TYPE_NAME = "any";
constexpr std::string_view ES_TYPE_ANNOTATION_NAME = "_ESTypeAnnotation";
constexpr std::string_view ES_MODULE_RECORD = "_ESModuleRecord";
constexpr std::string_view MODULE_RECORD_IDX = "moduleRecordIdx";
constexpr std::string_view CONCURRENT_MODULE_REQUEST_ANN_RECORD_TYPE_DESCRIPTOR =
    "L_ESConcurrentModuleRequestsAnnotation;";
constexpr std::string_view CONCURRENT_MODULE_REQUEST_RECORD_NAME = "_ESConcurrentModuleRequestsAnnotation";
constexpr std::string_view SLOT_NUMBER_ANN_RECORD_TYPE_DESCRIPTOR = "L_ESSlotNumberAnnotation;";
constexpr std::string_view SLOT_NUMBER_RECORD_NAME = "_ESSlotNumberAnnotation";
constexpr std::string_view DOT = ".";

// attribute constant
constexpr std::string_view ABC_ATTR_EXTERNAL = "external";
constexpr std::string_view ABC_ATTR_STATIC = "static";

// dump constant
constexpr std::string_view DUMP_TITLE_SOURCE_BINARY = "# source binary: ";
constexpr std::string_view DUMP_TITLE_LANGUAGE = ".language ";
constexpr std::string_view DUMP_TITLE_LITERALS = "# LITERALS";
constexpr std::string_view DUMP_TITLE_RECORDS = "# RECORDS";
constexpr std::string_view DUMP_TITLE_RECORD = ".record ";
constexpr std::string_view DUMP_TITLE_RECORD_SOURCE_FILE = ".record.source_file ";
constexpr std::string_view DUMP_TITLE_METHODS = "# METHODS";
constexpr std::string_view DUMP_TITLE_FUNCTION = ".function ";
constexpr std::string_view DUMP_TITLE_CATCH_ALL = ".catchall :";
constexpr std::string_view DUMP_TITLE_CATCH = ".catch :";
constexpr std::string_view DUMP_TITLE_STRING = "# STRING\n";
constexpr std::string_view DUMP_TITLE_SEPARATOR = "# ====================\n";
constexpr std::string_view DUMP_TITLE_LOCAL_VAR_TABLE = "# LOCAL_VARIABLE_TABLE:";
constexpr std::string_view DUMP_CONTENT_ECMASCRIPT = "ECMAScript";
constexpr std::string_view DUMP_CONTENT_PANDA_ASSEMBLY = "PandaAssembly";
constexpr std::string_view DUMP_CONTENT_SPACE = " ";
constexpr std::string_view DUMP_CONTENT_DOUBLE_SPACES = "  ";
constexpr std::string_view DUMP_CONTENT_TRIPLE_SPACES = "   ";
constexpr std::string_view DUMP_CONTENT_NONUPLE_SPACES = "         ";
constexpr std::string_view DUMP_CONTENT_TAB = "\t";
constexpr std::string_view DUMP_CONTENT_SINGLE_ENDL = "\n";
constexpr std::string_view DUMP_CONTENT_DOUBLE_ENDL = "\n\n";
constexpr std::string_view DUMP_CONTENT_ATTR_EXTERNAL = "<external>";
constexpr std::string_view DUMP_CONTENT_ATTR_STATIC = "<static>";
constexpr std::string_view DUMP_CONTENT_FUNCTION_PARAM_NAME_PREFIX = "a";
constexpr std::string_view DUMP_CONTENT_TRY_BEGIN_LABEL = "try_begin_label : ";
constexpr std::string_view DUMP_CONTENT_TRY_END_LABEL = "try_end_label : ";
constexpr std::string_view DUMP_CONTENT_CATCH_BEGIN_LABEL = "catch_begin_label : ";
constexpr std::string_view DUMP_CONTENT_CATCH_END_LABEL = "catch_end_label : ";
constexpr std::string_view DUMP_CONTENT_LOCAL_VAR_TABLE = "#\t Start   Length  Register           Name   Signature\n";
constexpr std::string_view DUMP_CONTENT_LINE_NUMBER = " # line: ";
constexpr std::string_view DUMP_CONTENT_COLUMN_NUMBER = " # column: ";
constexpr std::string_view DUMP_CONTENT_NESTED_LITERALARRAY = "$$NESTED-LITERALARRAY$$";

// literal array name
constexpr std::string_view UNDERLINE = "_";

// width constant
constexpr uint32_t LINE_WIDTH = 40;
constexpr uint32_t COLUMN_WIDTH = 10;
constexpr uint32_t START_WIDTH = 5;
constexpr uint32_t END_WIDTH = 6;
constexpr uint32_t REG_WIDTH = 8;
constexpr uint32_t NAME_WIDTH = 13;

// ins constant
constexpr std::string_view LABEL_PREFIX = "label@";

// module literal
constexpr uint8_t LITERAL_NUM_OF_REGULAR_IMPORT = 3;
constexpr uint8_t LITERAL_NUM_OF_NAMESPACE_IMPORT = 2;
constexpr uint8_t LITERAL_NUM_OF_LOCAL_EXPORT = 2;
constexpr uint8_t LITERAL_NUM_OF_INDIRECT_EXPORT = 3;
constexpr uint8_t LITERAL_NUM_OF_STAR_EXPORT = 1;
constexpr uint8_t LITERAL_NUMS[] = {
    LITERAL_NUM_OF_REGULAR_IMPORT,
    LITERAL_NUM_OF_NAMESPACE_IMPORT,
    LITERAL_NUM_OF_LOCAL_EXPORT,
    LITERAL_NUM_OF_INDIRECT_EXPORT,
    LITERAL_NUM_OF_STAR_EXPORT
};

class AbcFileUtils {
public:
    static bool IsGlobalTypeName(const std::string &type_name);
    static bool IsArrayTypeName(const std::string &type_name);
    static bool IsSystemTypeName(const std::string &type_name);
    static bool IsESTypeAnnotationName(const std::string &type_name);
    static bool IsLiteralTagArray(const panda_file::LiteralTag &tag);
    static std::string GetLabelNameByInstIdx(size_t inst_idx);
};  // class AbcFileUtils

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_FILE_UTILS_H
