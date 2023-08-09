/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "assembler/meta.h"
#include "bytecode_optimizer/optimize_bytecode.h"

namespace panda::bytecodeopt {
using ArrayValue = panda::pandasm::ArrayValue;
using ScalarValue = panda::pandasm::ScalarValue;
using AnnotationData = panda::pandasm::AnnotationData;
using AnnotationElement = panda::pandasm::AnnotationElement;
using TypeInfoMap = std::unordered_map<int32_t, TypeInfoIndex>;

class TypeAdaptionTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    TypeInfoMap ExtractTypeinfo(const panda::pandasm::Function &fun, const panda::pandasm::Program &prog) const
    {
        const auto &annos = fun.metadata->GetAnnotations();
        EXPECT_FALSE(annos.empty());
        const auto &eles = annos[0].GetElements();
        EXPECT_FALSE(eles.empty());
        const auto *ele = eles[0].GetValue();
        EXPECT_NE(ele, nullptr);
        const auto key = ele->GetAsScalar()->GetValue<std::string>();
        auto array_liter = prog.literalarray_table.find(key);
        EXPECT_NE(array_liter, prog.literalarray_table.end());
        const auto &array = array_liter->second.literals_;
        // 4: size must be multiple of 4 because values consits of tuple of tag, order, tag, type
        EXPECT_EQ(array.size() % 4u, 0u);
        TypeInfoMap type_info;
        size_t i = 1;  // 1: skip tag of order, so start from 1
        while (i < array.size()) {
            auto order = bit_cast<int32_t>(std::get<uint32_t>(array[i].value_));
            i += 2;  // 2: skip tag between order and type
            TypeInfoIndex type;
            if (array[i].tag_ == panda_file::LiteralTag::LITERALARRAY) {
                type = std::get<std::string>(array[i].value_);
            } else {
                EXPECT_EQ(array[i].tag_, panda_file::LiteralTag::BUILTINTYPEINDEX);
                type = std::get<BuiltinIndexType>(array[i].value_);
            }
            type_info.emplace(order, type);
            i += 2;  // 2: skip tag between order and type
        }
        return type_info;
    }

    void CheckTypeExist(const TypeInfoMap &typeinfo, int32_t order, const TypeInfoIndex &type) const
    {
        auto type_it = typeinfo.find(order);
        EXPECT_NE(type_it, typeinfo.end());
        EXPECT_EQ(type_it->second, type);
    }

    void AddTypeinfo(panda::pandasm::LiteralArray &lit_arr, int32_t order, TypeInfoIndex type) const
    {
        auto &arr = lit_arr.literals_;

        panda::pandasm::LiteralArray::Literal order_tag;
        order_tag.tag_ = panda::panda_file::LiteralTag::TAGVALUE;
        order_tag.value_ = static_cast<uint8_t>(panda::panda_file::LiteralTag::INTEGER);
        arr.emplace_back(order_tag);

        panda::pandasm::LiteralArray::Literal order_val;
        order_val.tag_ = panda::panda_file::LiteralTag::INTEGER;
        order_val.value_ = static_cast<uint32_t>(order);
        arr.emplace_back(order_val);

        panda::pandasm::LiteralArray::Literal type_tag;
        panda::pandasm::LiteralArray::Literal type_val;
        type_tag.tag_ = panda::panda_file::LiteralTag::TAGVALUE;

        if (std::holds_alternative<uint8_t>(type)) {
            type_tag.value_ = static_cast<uint8_t>(panda::panda_file::LiteralTag::BUILTINTYPEINDEX);
            type_val.tag_ = panda::panda_file::LiteralTag::BUILTINTYPEINDEX;
            type_val.value_ = std::get<uint8_t>(type);
        } else {
            EXPECT_TRUE(std::holds_alternative<std::string>(type));
            type_tag.value_ = static_cast<uint8_t>(panda::panda_file::LiteralTag::LITERALARRAY);
            type_val.tag_ = panda::panda_file::LiteralTag::LITERALARRAY;
            type_val.value_ = std::get<std::string>(type);
        }
        arr.emplace_back(type_tag);
        arr.emplace_back(type_val);
    }

    void SetTypeAnnotationForFunc(const panda::pandasm::LiteralArray &arr, panda::pandasm::Function &func,
                                  panda::pandasm::Program &program) const
    {
        auto id = std::to_string(program.literalarray_table.size());
        program.literalarray_table.emplace(id, arr);

        AnnotationElement element(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ScalarValue>(
            ScalarValue::Create<panda::pandasm::Value::Type::LITERALARRAY>(id)));
        AnnotationData annotation(TSTYPE_ANNO_RECORD_NAME);
        annotation.AddElement(std::move(element));
        std::vector<AnnotationData> annos;
        annos.emplace_back(annotation);
        func.metadata->SetAnnotations(std::move(annos));
        const auto iterator = program.record_table.find(TSTYPE_ANNO_RECORD_NAME.data());
        EXPECT_NE(iterator, program.record_table.end());
        iterator->second.metadata->SetAccessFlags(panda::ACC_ANNOTATION);
        EXPECT_TRUE(program.record_table.find(TSTYPE_ANNO_RECORD_NAME.data())->second.metadata->IsAnnotation());
    }

    // add a literalarray as a type
    TypeInfoIndex AddAnTypeLiteralArray(panda::pandasm::Program &program) const
    {
        panda::pandasm::LiteralArray arr;
        panda::pandasm::LiteralArray::Literal tag;
        tag.tag_ = panda::panda_file::LiteralTag::TAGVALUE;
        tag.value_ = static_cast<uint8_t>(panda::panda_file::LiteralTag::BUILTINTYPEINDEX);
        arr.literals_.emplace_back(tag);
        panda::pandasm::LiteralArray::Literal val;
        val.tag_ = panda::panda_file::LiteralTag::BUILTINTYPEINDEX;
        val.value_ = static_cast<uint8_t>(0u);
        arr.literals_.emplace_back(val);
        const std::string litKey = std::to_string(program.literalarray_table.size());
        program.literalarray_table.emplace(litKey, arr);
        TypeInfoIndex ret = litKey;
        return ret;
    }

    void EmitAndOptimize(const std::string &abcFileName, panda::pandasm::Program &program) const
    {
        std::map<std::string, size_t> *statp = nullptr;
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = &maps;
        EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abcFileName, program, statp, mapsp, false));
        EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, mapsp, abcFileName, true));
    }
};

HWTEST_F(TypeAdaptionTest, type_adaption_test_001, testing::ext::TestSize.Level1)
{
    /* ts source code
    function foo(a:number, b:string, c:string):string
    {
        let t:number = 100;
        if (a > t) {
            return b;
        }
        if (a < t) {
            return c;
        }
        let ret:string = b + c;
        return ret;
    }
    */
    const auto source = R"(
        .language ECMAScript
        .record _ESTypeAnnotation <external>
        .function any foo(any a0, any a1, any a2, any a3, any a4, any a5) <static> {
            mov v5, a5
            mov v4, a4
            mov v3, a3
            mov v2, a2
            mov v1, a1
            mov v0, a0
            ldai 0x64
            sta v6
            lda v3
            sta v9
            lda v6
            greater 0x0, v9
            jeqz jump_label_0
            lda v4
            return
        jump_label_0:
            lda v3
            sta v9
            lda v6
            less 0x1, v9
            jeqz jump_label_1
            lda v5
            return
        jump_label_1:
            lda v4
            sta v9
            lda v5
            add2 0x2, v9
            sta v7
            lda v7
            return
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "foo:(any,any,any,any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());

    auto &func = it->second;
    panda::pandasm::LiteralArray lit_arr;
    static const TypeInfoIndex NUM_TYPE = static_cast<uint8_t>(1);
    static const TypeInfoIndex STR_TYPE = static_cast<uint8_t>(4);
    // set arg type
    const auto THIS_TYPE = AddAnTypeLiteralArray(program);
    AddTypeinfo(lit_arr, -3, THIS_TYPE);  // -3: the arg "this"
    AddTypeinfo(lit_arr, -4, NUM_TYPE);   // -4: the first arg
    AddTypeinfo(lit_arr, -5, STR_TYPE);   // -5: the second arg
    AddTypeinfo(lit_arr, -6, STR_TYPE);   // -6: the third arg
    // set ins type
    const size_t LDAI_IDX = 6;
    EXPECT_EQ(func.ins[LDAI_IDX].opcode, panda::pandasm::Opcode::LDAI);
    EXPECT_EQ(func.ins[LDAI_IDX + 1].opcode, panda::pandasm::Opcode::STA);
    AddTypeinfo(lit_arr, static_cast<int32_t>(LDAI_IDX + 1), NUM_TYPE);
    const size_t ADD_IDX = 27;
    EXPECT_EQ(func.ins[ADD_IDX].opcode, panda::pandasm::Opcode::ADD2);
    EXPECT_EQ(func.ins[ADD_IDX + 1].opcode, panda::pandasm::Opcode::STA);
    int32_t num_invalid = std::count_if(func.ins.begin(), func.ins.begin() + ADD_IDX,
                                        [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    AddTypeinfo(lit_arr, ADD_IDX + 1 - num_invalid, STR_TYPE);  // exclude invalid insns because they do not emit

    SetTypeAnnotationForFunc(lit_arr, func, program);

    EmitAndOptimize("TypeAdaptionTest.abc", program);

    // check typeinfo after optimization
    it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    const auto &foo = it->second;
    const auto typeinfo = ExtractTypeinfo(foo, program);
    CheckTypeExist(typeinfo, -3, THIS_TYPE);  // -3: the arg "this"
    CheckTypeExist(typeinfo, -4, NUM_TYPE);   // -4: the first arg
    CheckTypeExist(typeinfo, -5, STR_TYPE);   // -5: the second arg
    CheckTypeExist(typeinfo, -6, STR_TYPE);   // -6: the third arg
    auto ldai_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::LDAI; });
    EXPECT_NE(ldai_it, foo.ins.end());
    const auto opt_ldai_idx = static_cast<size_t>(ldai_it - foo.ins.begin());
    EXPECT_EQ(foo.ins[opt_ldai_idx].opcode, panda::pandasm::Opcode::LDAI);
    EXPECT_LT(opt_ldai_idx + 1, foo.ins.size());
    EXPECT_EQ(foo.ins[opt_ldai_idx + 1].opcode, panda::pandasm::Opcode::STA);

    num_invalid = std::count_if(foo.ins.begin(), ldai_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t ldai_type_idx = opt_ldai_idx - num_invalid;  // exclude invalid insns because they do not emit

    CheckTypeExist(typeinfo, ldai_type_idx + 1, NUM_TYPE);  // type is on sta

    auto add_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                               [](const auto &in) { return in.opcode == panda::pandasm::Opcode::ADD2; });
    EXPECT_NE(add_it, foo.ins.end());
    const auto opt_add_idx = static_cast<size_t>(add_it - foo.ins.begin());
    EXPECT_EQ(foo.ins[opt_add_idx].opcode, panda::pandasm::Opcode::ADD2);
    EXPECT_LT(opt_add_idx + 1, foo.ins.size());
    EXPECT_NE(foo.ins[opt_add_idx + 1].opcode, panda::pandasm::Opcode::STA);

    num_invalid = std::count_if(foo.ins.begin(), add_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t add_type_idx = opt_add_idx - num_invalid;  // exclude invalid insns because they do not emit
    CheckTypeExist(typeinfo, add_type_idx, STR_TYPE);  // type is on add2 as it does not have sta
}

HWTEST_F(TypeAdaptionTest, unconditional_jump_case_test_001, testing::ext::TestSize.Level1)
{
    /* ts source code
    function foo(results: number) {
        for (let i = 0; i < 1; i++) {
            results *= i;
        }
        let s:number = results + 2;
        return s;
    }
    */
    const auto source = R"(
        .language ECMAScript
        .record _ESTypeAnnotation <external>
        .function any foo(any a0, any a1, any a2, any a3) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            mov v3, a3
            ldai 0x0
            sta v6
        jump_label_1:
            lda v6
            sta v7
            ldai 0x1
            less 0x0, v7
            jeqz jump_label_0
            lda v3
            sta v7
            lda v6
            mul2 0x1, v7
            sta v3
            lda v6
            sta v7
            lda v7
            inc 0x2
            sta v6
            lda v7
            tonumeric 0x3
            jmp jump_label_1
        jump_label_0:
            lda v3
            sta v6
            ldai 0x2
            add2 0x4, v6
            sta v4
            lda v4
            return
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "foo:(any,any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());

    auto &func = it->second;
    panda::pandasm::LiteralArray lit_arr;
    static const TypeInfoIndex NUM_TYPE = static_cast<uint8_t>(1);
    static const TypeInfoIndex STR_TYPE = static_cast<uint8_t>(4);
    // set arg type
    const auto THIS_TYPE = AddAnTypeLiteralArray(program);
    AddTypeinfo(lit_arr, -3, THIS_TYPE);  // -3: the arg "this"
    AddTypeinfo(lit_arr, -4, NUM_TYPE);   // -4: the first arg
    // set ins type
    const size_t ADD_IDX = 29;
    EXPECT_EQ(func.ins[ADD_IDX].opcode, panda::pandasm::Opcode::ADD2);
    EXPECT_EQ(func.ins[ADD_IDX + 1].opcode, panda::pandasm::Opcode::STA);
    int32_t num_invalid = std::count_if(func.ins.begin(), func.ins.begin() + ADD_IDX,
                                        [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    AddTypeinfo(lit_arr, ADD_IDX + 1 - num_invalid, STR_TYPE);  // exclude invalid insns because they do not emit

    SetTypeAnnotationForFunc(lit_arr, func, program);

    EmitAndOptimize("TypeAdaptionTest_UnconditionalJump.abc", program);

    // check typeinfo after optimization
    it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    const auto &foo = it->second;
    const auto typeinfo = ExtractTypeinfo(foo, program);
    CheckTypeExist(typeinfo, -3, THIS_TYPE);  // -3: the arg "this"
    CheckTypeExist(typeinfo, -4, NUM_TYPE);   // -4: the first arg
    auto add_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                               [](const auto &in) { return in.opcode == panda::pandasm::Opcode::ADD2; });
    EXPECT_NE(add_it, foo.ins.end());
    const auto opt_add_idx = static_cast<size_t>(add_it - foo.ins.begin());
    EXPECT_EQ(foo.ins[opt_add_idx].opcode, panda::pandasm::Opcode::ADD2);
    EXPECT_TRUE(opt_add_idx + 1 < foo.ins.size());
    EXPECT_NE(foo.ins[opt_add_idx + 1].opcode, panda::pandasm::Opcode::STA);

    num_invalid = std::count_if(foo.ins.begin(), add_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t add_type_idx = opt_add_idx - num_invalid;  // exclude invalid insns because they do not emit
    CheckTypeExist(typeinfo, add_type_idx, STR_TYPE);  // type is on add2 as it does not have sta
}

}  // namespace panda::bytecodeopt::test
