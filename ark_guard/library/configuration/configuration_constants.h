/**
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

#ifndef ARK_GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H
#define ARK_GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H

#include <string_view>

namespace ark::guard {

namespace AccessFlagsConstants {
constexpr std::string_view PUBLIC = "public";
constexpr std::string_view PROTECTED = "protected";
constexpr std::string_view INTERNAL = "internal";
constexpr std::string_view PRIVATE = "private";
constexpr std::string_view STATIC = "static";
constexpr std::string_view READONLY = "readonly";
constexpr std::string_view ASYNC = "async";
constexpr std::string_view ABSTRACT = "abstract";
constexpr std::string_view NATIVE = "native";
constexpr std::string_view FINAL = "final";
constexpr std::string_view OVERRIDE = "override";
constexpr std::string_view ANNOTATION = "@";
constexpr std::string_view DECLARE = "declare";
}  // namespace AccessFlagsConstants

namespace TypeDeclarationsConstants {
constexpr std::string_view INTERFACE = "interface";
constexpr std::string_view CLASS = "class";
constexpr std::string_view ENUM = "enum";
constexpr std::string_view TYPE = "type";
constexpr std::string_view NAMESPACE = "namespace";
constexpr std::string_view PACKAGE = "package";
}  // namespace TypeDeclarationsConstants

namespace ExtensionTypesConstants {
constexpr std::string_view EXTENDS = "extends";
constexpr std::string_view IMPLEMENTS = "implements";
}  // namespace ExtensionTypesConstants

namespace KeepOptionsConstants {
constexpr std::string_view KEEP_OPTION = "-keep";
constexpr std::string_view KEEP_CLASS_MEMBERS = "-keepclassmembers";
constexpr std::string_view KEEP_CLASS_WITH_MEMBERS = "-keepclasswithmembers";
}  // namespace KeepOptionsConstants

namespace PrimitiveTypesConstants {
constexpr std::string_view INT = "int";
constexpr std::string_view BYTE = "byte";
constexpr std::string_view SHORT = "short";
constexpr std::string_view NUMBER = "number";
constexpr std::string_view LONG = "long";
constexpr std::string_view FLOAT = "float";
constexpr std::string_view DOUBLE = "double";
constexpr std::string_view CHAR = "char";
constexpr std::string_view BOOLEAN = "boolean";
constexpr std::string_view STRING = "string";
constexpr std::string_view BIGINT = "bigint";
constexpr std::string_view NULL_TYPE = "null";
constexpr std::string_view NEVER = "never";
constexpr std::string_view UNDEFINED = "undefined";
constexpr std::string_view ARRAY = "[]";
}  // namespace PrimitiveTypesConstants

namespace PandaFileTypesConstants {
constexpr std::string_view INT = "i32";
constexpr std::string_view BYTE = "i8";
constexpr std::string_view SHORT = "i16";
constexpr std::string_view LONG = "i64";
constexpr std::string_view NUMBER = "f64";
constexpr std::string_view FLOAT = "f32";
constexpr std::string_view DOUBLE = "f64";
constexpr std::string_view CHAR = "u16";
constexpr std::string_view BOOLEAN = "u1";
constexpr std::string_view STRING = "std.core.String";
constexpr std::string_view BIGINT = "std.core.BigInt";
constexpr std::string_view NULL_TYPE = "std.core.Null";
constexpr std::string_view NEVER = "std.core.Object";
constexpr std::string_view UNDEFINED = "std.core.Object";
constexpr std::string_view ARRAY = "escompat.Array";
}  // namespace PandaFileTypesConstants

namespace PandaFileUnionTypesConstants {
constexpr std::string_view INT = "std.core.Int";
constexpr std::string_view BYTE = "std.core.Byte";
constexpr std::string_view SHORT = "std.core.Short";
constexpr std::string_view LONG = "std.core.Long";
constexpr std::string_view NUMBER = "std.core.Double";
constexpr std::string_view FLOAT = "std.core.Float";
constexpr std::string_view DOUBLE = "std.core.Double";
constexpr std::string_view CHAR = "std.core.Char";
constexpr std::string_view BOOLEAN = "std.core.Boolean";
constexpr std::string_view STRING = "std.core.String";
constexpr std::string_view BIGINT = "std.core.BigInt";
constexpr std::string_view NULL_TYPE = "std.core.Null";
constexpr std::string_view NEVER = "";
constexpr std::string_view UNDEFINED = "";
constexpr std::string_view ARRAY = "escompat.Array";
}  // namespace PandaFileTypesConstants

namespace ConfigurationConstants {
constexpr std::string_view KEEP_OPTION_PREFIX = "-";
constexpr std::string_view AT_DIRECTIVE = "@";

constexpr std::string_view NEGATOR_KEYWORD = "!";
constexpr std::string_view OPEN_KEYWORD = "{";
constexpr std::string_view CLOSE_KEYWORD = "}";

constexpr std::string_view ANNOTATION_KEYWORD = "@";

constexpr std::string_view ANY_CLASS_KEYWORD = "*";
constexpr std::string_view ANY_CLASS_MEMBER_KEYWORD = "*";
constexpr std::string_view ANY_CONSTRUCTOR_KEYWORD = "<constructor>";
constexpr std::string_view ANY_FIELD_KEYWORD = "<fields>";
constexpr std::string_view ANY_METHOD_KEYWORD = "<methods>";

constexpr std::string_view OPEN_ARGUMENTS_KEYWORD = "(";
constexpr std::string_view CLOSE_ARGUMENTS_KEYWORD = ")";

constexpr std::string_view EQUAL_KEYWORD = "=";
constexpr std::string_view RETURN_KEYWORD = "return";

constexpr std::string_view COMMA_KEYWORD = ",";

constexpr std::string_view COLON_KEYWORD = ":";

constexpr std::string_view SEMICOLON_KEYWORD = ";";

constexpr char PIPE_KEYWORD = '|';
constexpr std::string_view WHITE_SPACE_KEYWORD = " \t";

constexpr std::string_view TYPE_STD_CORE_OBJECT = "std.core.Object";

}  // namespace ConfigurationConstants

}  // namespace ark::guard

#endif  // ARK_GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H
