/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "disassembler.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

using namespace testing::ext;
namespace panda::disasm {

static const std::string FILE_DECLARATION_3D_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-boolean.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-enum-number.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-enum-string.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-number.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-string.abc";
static const std::string FILE_DECLARATION_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-array-boolean.abc";
static const std::string FILE_DECLARATION_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-array-enum-number.abc";
static const std::string FILE_DECLARATION_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-array-enum-string.abc";
static const std::string FILE_DECLARATION_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-array-number.abc";
static const std::string FILE_DECLARATION_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-array-string.abc";
static const std::string FILE_DECLARATION_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-boolean.abc";
static const std::string FILE_DECLARATION_COMBINATION =
    GRAPH_TEST_ABC_DIR "declaration-combination.abc";
static const std::string FILE_DECLARATION_EMPTY =
    GRAPH_TEST_ABC_DIR "declaration-empty.abc";
static const std::string FILE_DECLARATION_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-enum-number.abc";
static const std::string FILE_DECLARATION_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-enum-string.abc";
static const std::string FILE_DECLARATION_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-number.abc";
static const std::string FILE_DECLARATION_STRING =
    GRAPH_TEST_ABC_DIR "declaration-string.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-number.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-string.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-number.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-string.abc";
static const std::string FILE_DECLARATION_USAGE_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_COMBINATION =
    GRAPH_TEST_ABC_DIR "declaration-usage-combination.abc";
static const std::string FILE_DECLARATION_USAGE_EMPTY =
    GRAPH_TEST_ABC_DIR "declaration-usage-empty.abc";
static const std::string FILE_DECLARATION_USAGE_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-number.abc";
static const std::string FILE_DECLARATION_USAGE_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-string.abc";
static const std::string FILE_EXPORT =
    GRAPH_TEST_ABC_DIR "export.abc";
static const std::string FILE_IMPORT_QUALIFIED =
    GRAPH_TEST_ABC_DIR "import-qualified.abc";
static const std::string FILE_IMPORT_UNQUALIFIED =
    GRAPH_TEST_ABC_DIR "import-unqualified.abc";
static const std::string FILE_MULTIPLE_ANNOTATIONS =
    GRAPH_TEST_ABC_DIR "multiple-annotations.abc";

class DisassemblerUserAnnotationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        disasm = new panda::disasm::Disassembler{};
    };

    void TearDown()
    {
        delete disasm;
        disasm = nullptr;
    };

    void ValidateMethodAnnotation(const std::string &method_name, const std::string &anno_name,
                                  std::string anno_expected)
    {
        auto anno = disasm->GetSerializedMethodAnnotation(method_name, anno_name);
        ASSERT_NE(anno, std::nullopt);
        std::string anno_str = anno.value();
        // remove whitespaces from both strings
        auto anno_str_rem_iter = std::remove_if(anno_str.begin(), anno_str.end(),
                                                [](unsigned char x) { return std::isspace(x); });
        anno_str.erase(anno_str_rem_iter, anno_str.end());
        auto anno_expected_rem_iter = std::remove_if(anno_expected.begin(), anno_expected.end(),
                                                     [](unsigned char x) { return std::isspace(x); });
        anno_expected.erase(anno_expected_rem_iter, anno_expected.end());
        ASSERT_EQ(anno_str, anno_expected);
    }

    void ValidateRecord(const std::string &record_name, std::string record_expected)
    {
        auto record = disasm->GetSerializedRecord(record_name);
        ASSERT_NE(record, std::nullopt);
        std::string record_str = record.value();
        // remove whitespaces from both strings
        auto record_str_rem_iter = std::remove_if(record_str.begin(), record_str.end(),
                                                  [](unsigned char x) { return std::isspace(x); });
        record_str.erase(record_str_rem_iter, record_str.end());
        auto record_expected_rem_iter = std::remove_if(record_expected.begin(), record_expected.end(),
                                                       [](unsigned char x) { return std::isspace(x); });
        record_expected.erase(record_expected_rem_iter, record_expected.end());
        ASSERT_EQ(record_str, record_expected);
    }

    void InitDisasm(std::string file_path)
    {
        disasm->Disassemble(file_path, false, false);
    }

private:
    panda::disasm::Disassembler *disasm = nullptr;
};

/**
* @tc.name: test_declaration_3d_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_BOOLEAN);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            u1[][][] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            u1[][][] a = [[[]]]
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            u1[][][] a = [[[1, 0], [1, 0]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_ENUM_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64[][][] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64[][][] a = [[[]]]
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            f64[][][] a = [[[42, 314, 42]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_ENUM_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String[][][] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String[][][] a = [[[]]]
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            panda.String[][][] a = [[["Hello", "world!", "Hello"]]]
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_3d_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64[][][] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64[][][] a = [[[]]]
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            f64[][][] a = [[[1, 2, 3], [4, 5, 6]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String[][][] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String[][][] a = [[[]]]
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            panda.String[][][] a = [[["hello", "world"], ["hello", "world"]]]
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_BOOLEAN);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            u1[] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            u1[] a = []
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            u1[] a = [1, 0, 1]
        }
    )");
}

/**
* @tc.name: test_declaration_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_ENUM_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64[] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64[] a = []
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            f64[] a = [42, 314, 42]
        }
    )");
}

/**
* @tc.name: test_declaration_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_ENUM_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String[] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String[] a = []
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            panda.String[] a = ["Hello", "world!", "Hello"]
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64[] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64[] a = []
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            f64[] a = [1, 2, 3]
        }
    )");
}

/**
* @tc.name: test_declaration_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String[] a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String[] a = []
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            panda.String[] a = ["Hello", "world", "!"]
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_BOOLEAN);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            u1 a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            u1 a = 1
        }
    )");
}

/**
* @tc.name: test_declaration_combination
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_combination, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_COMBINATION);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64 a
            f64[] b = [13, 10]
            panda.String c
            u1 d
            f64[] e = [1, 2, 3]
            f64[] f
            f64 h
            f64[][][] i
            panda.String j
            panda.String[][][] k
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_empty
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_empty, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_EMPTY);

    ValidateRecord("Anno", R"(
        .record Anno {
        }
    )");
}

/**
* @tc.name: test_declaration_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ENUM_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64 a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64 a = 42
        }
    )");
}

/**
* @tc.name: test_declaration_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ENUM_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String a = "Hello"
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_NUMBER);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64 a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64 a = 42
        }
    )");
}

/**
* @tc.name: test_declaration_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_STRING);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            panda.String a
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String a = "Hello world!"
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_BOOLEAN);

    ValidateRecord("Anno", R"(
        .record Anno {
            u1[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-boolean.#~A=#A", "LAnno", R"(
        LAnno:
            u1[][][] a { [[[1, 0, 1]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-boolean.#~A>#foo", "LAnno", R"(
        LAnno:
            u1[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64[][][] a { [[[1, 2, 1]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String[][][] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String[][][] a { [[["Hello", "world", "Hello"]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64[][][] a { [[[1, 2, 3]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String[][][] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String[][][] a { [[["Hello", "world", "!"]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_BOOLEAN);

    ValidateRecord("Anno", R"(
        .record Anno {
            u1[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-boolean.#~A=#A", "LAnno", R"(
        LAnno:
            u1[] a { [1, 0, 1] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-boolean.#~A>#foo", "LAnno", R"(
        LAnno:
            u1[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_ENUM_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64[] a { [1, 2, 1] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_ENUM_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String[] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String[] a { ["Hello", "world", "Hello"] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64[] a { [1, 2, 3] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String[] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-array-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String[] a { ["hello", "world", "!"] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_BOOLEAN);

    ValidateRecord("Anno", R"(
        .record Anno {
            u1 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-boolean.#~A=#A", "LAnno", R"(
        LAnno:
            u1 a { 1 }
    )");

    ValidateMethodAnnotation("declaration-usage-boolean.#~A>#foo", "LAnno", R"(
        LAnno:
            u1 a { 0 }
    )");
}

/**
* @tc.name: test_declaration_usage_combination
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_combination, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_COMBINATION);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64 a
            f64[] b = [13, 10]
            panda.String c
            u1 d
            f64[] e = [1, 2, 3]
            f64[] f
            f64 h
            f64[][][] i
            panda.String j
            panda.String[][][] k
        }
    )");

    ValidateMethodAnnotation("declaration-usage-combination.#~A=#A", "LAnno", R"(
        LAnno:
            f64 a { 20 }
            f64[] b { [13, 10] }
            panda.String c { "ab" }
            u1 d { 1 }
            f64[] e { [1, 2, 3] }
            f64[] f { [] }
            f64[][][] g { [[[0]]] }
            f64 h { 10 }
            f64[][][] i { [[[]]] }
            panda.String j { "B" }
            panda.String[][][] k { [[[]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-combination.#~A>#foo", "LAnno", R"(
        LAnno:
            f64 a { 10 }
            f64[] b { [1, 2, 3] }
            panda.String c { "cde" }
            u1 d { 1 }
            f64[] e { [1, 2, 3] }
            f64[] f { [1] }
            f64[][][] g { [[[0], [1]]] }
            f64 h { 10 }
            f64[][][] i { [[[10], [20]]] }
            panda.String j { "B" }
            panda.String[][][] k { [[["A"], ["B"]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_empty
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_empty, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_EMPTY);

    ValidateRecord("Anno", R"(
        .record Anno {
        }
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A=#A", "LAnno", R"(
        LAnno:
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A>#foo", "LAnno", R"(
        LAnno:
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A>#bar", "LAnno", R"(
        LAnno:
    )");
}

/**
* @tc.name: test_declaration_usage_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ENUM_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64 a { 1 }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64 a { 2 }
    )");
}

/**
* @tc.name: test_declaration_usage_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ENUM_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-enum-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String a { "Hello" }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String a { "world" }
    )");
}

/**
* @tc.name: test_declaration_usage_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_NUMBER);

    ValidateRecord("Anno", R"(
        .record Anno {
            f64 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-number.#~A=#A", "LAnno", R"(
        LAnno:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("declaration-usage-number.#~A>#foo", "LAnno", R"(
        LAnno:
            f64 a { 314 }
    )");
}

/**
* @tc.name: test_declaration_usage_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_STRING);

    ValidateRecord("Anno", R"(
        .record Anno {
            panda.String a
        }
    )");

    ValidateRecord("panda.String", R"(
        .record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-string.#~A=#A", "LAnno", R"(
        LAnno:
            panda.String a { "Hello" }
    )");

    ValidateMethodAnnotation("declaration-usage-string.#~A>#foo", "LAnno", R"(
        LAnno:
            panda.String a { "world" }
    )");
}

/**
* @tc.name: test_export
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_export, TestSize.Level1)
{
    InitDisasm(FILE_EXPORT);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            f64 a = 0
        }
    )");

    ValidateMethodAnnotation("export.#~A=#A", "LAnno1", R"(
        LAnno1:
    )");

    ValidateMethodAnnotation("export.#~B=#B", "LAnno1", R"(
        LAnno1:
    )");
}

/**
* @tc.name: test_import_qualified
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_import_qualified, TestSize.Level1)
{
    InitDisasm(FILE_IMPORT_QUALIFIED);

    ValidateRecord("Namespace1.Anno", R"(
        .record Namespace1.Anno <external>
    )");

    ValidateRecord("Namespace1.Anno1", R"(
        .record Namespace1.Anno1 <external>
    )");

    ValidateRecord("Namespace1.Namespace2.Namespace3.Anno2", R"(
        .record Namespace1.Namespace2.Namespace3.Anno2 <external>
    )");

    ValidateRecord("Namespace1.Namespace2.Namespace3.Anno3", R"(
        .record Namespace1.Namespace2.Namespace3.Anno3 <external>
    )");

    ValidateMethodAnnotation("import-qualified.#~A=#A", "LNamespace1.Anno", R"(
        LNamespace1.Anno:
    )");

    ValidateMethodAnnotation("import-qualified.#~B=#B", "LNamespace1.Anno", R"(
        LNamespace1.Anno:
    )");

    ValidateMethodAnnotation("import-qualified.#~C=#C", "LNamespace1.Anno1", R"(
        LNamespace1.Anno1:
            f64 a { 1 }
            panda.String b { "string" }
    )");

    ValidateMethodAnnotation("import-qualified.#~D=#D", "LNamespace1.Namespace2.Namespace3.Anno2", R"(
        LNamespace1.Namespace2.Namespace3.Anno2:
    )");

    ValidateMethodAnnotation("import-qualified.#~E=#E", "LNamespace1.Namespace2.Namespace3.Anno2", R"(
        LNamespace1.Namespace2.Namespace3.Anno2:
    )");
}

/**
* @tc.name: test_import_unqualified
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_import_unqualified, TestSize.Level1)
{
    InitDisasm(FILE_IMPORT_UNQUALIFIED);

    ValidateRecord("Anno1", R"(
        .record Anno1 <external>
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 <external>
    )");

    ValidateMethodAnnotation("import-unqualified.#~A=#A", "LAnno1", R"(
        LAnno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~B=#B", "LAnno1", R"(
        LAnno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~C=#C", "LAnno1", R"(
        LAnno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~D=#D", "LAnno2", R"(
        LAnno2:
            f64 a { 1 }
            panda.String b { "string" }
    )");
}

/**
* @tc.name: test_multiple_annotations
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_multiple_annotations, TestSize.Level1)
{
    InitDisasm(FILE_MULTIPLE_ANNOTATIONS);

    ValidateRecord("Anno1", R"(
        .record Anno1 {
            f64 a = 1
        }
    )");

    ValidateRecord("Anno2", R"(
        .record Anno2 {
            panda.String b = "string"
        }
    )");

    ValidateRecord("Anno3", R"(
        .record Anno3 {
            u1[] c = [1, 0]
        }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "LAnno1", R"(
        LAnno1:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "LAnno2", R"(
        LAnno2:
            panda.String b { "abc" }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "LAnno3", R"(
        LAnno3:
            u1[] c { [0, 1] }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "LAnno1", R"(
        LAnno1:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "LAnno2", R"(
        LAnno2:
            panda.String b { "abc" }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "LAnno3", R"(
        LAnno3:
            u1[] c { [0, 1] }
    )");
}

};
