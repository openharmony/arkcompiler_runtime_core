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

#include "assembler/meta.h"
#include "assembler/assembly-parser.h"
#include "bytecode_optimizer/optimize_bytecode.h"

namespace panda::bytecodeopt::test {
using ArrayValue = panda::pandasm::ArrayValue;
using ScalarValue = panda::pandasm::ScalarValue;
using AnnotationData = panda::pandasm::AnnotationData;
using AnnotationElement = panda::pandasm::AnnotationElement;

class TestBase {
public:
    template <typename T1, typename T2>
    inline void TestAssertEqual(const T1 &left, const T2 &right) const
    {
        if (left != static_cast<T1>(right)) {
            std::cout << "assertion equal failed." << std::endl;
            UNREACHABLE();
        }
    }

    template <typename T1, typename T2>
    inline void TestAssertNotEqual(const T1 &left, const T2 &right) const
    {
        if (left == static_cast<T1>(right)) {
            std::cout << "assertion not equal failed." << std::endl;
            UNREACHABLE();
        }
    }

    inline void TestAssertTrue(bool val) const
    {
        if (!val) {
            UNREACHABLE();
        }
    }

    inline void TestAssertFalse(bool val) const
    {
        if (val) {
            UNREACHABLE();
        }
    }
};

class BytecodeOptimizerTypeAdaptionTest : TestBase {
public:
    std::unordered_map<int32_t, int32_t> ExtractTypeinfo(const panda::pandasm::Function &fun) const
    {
        const auto *ele = fun.metadata->GetAnnotations()[0].GetElements()[0].GetValue();
        const auto &values = ele->GetAsArray()->GetValues();
        std::unordered_map<int32_t, int32_t> type_info;
        const size_t PAIR_GAP = 2;
        TestAssertEqual(values.size() % PAIR_GAP, 0);  // must be even as it consits of pairs
        for (size_t i = 0; i < values.size(); i += PAIR_GAP) {
            type_info.emplace(values[i].GetValue<int32_t>(), values[i + 1].GetValue<int32_t>());
        }
        return type_info;
    }

    void CheckTypeExist(const std::unordered_map<int32_t, int32_t> &typeinfo, int32_t order, int32_t type) const
    {
        auto type_it = typeinfo.find(order);
        TestAssertNotEqual(type_it, typeinfo.end());
        TestAssertEqual(type_it->second, type);
    }

    void AddTypeinfo(std::vector<ScalarValue> *elements, int32_t order, int32_t type) const
    {
        ScalarValue insn_order(ScalarValue::Create<panda::pandasm::Value::Type::I32>(order));
        elements->emplace_back(std::move(insn_order));
        ScalarValue insn_type(ScalarValue::Create<panda::pandasm::Value::Type::I32>(type));
        elements->emplace_back(std::move(insn_type));
    }

    void SetTypeAnnotationForFunc(const std::vector<ScalarValue> &elements, panda::pandasm::Function &func, 
                                  panda::pandasm::Program &program) const
    {
        ArrayValue array_value(panda::pandasm::Value::Type::I32, elements);
        AnnotationElement anno_element(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_value));
        AnnotationData annotation(TSTYPE_ANNO_RECORD_NAME);
        annotation.AddElement(std::move(anno_element));
        std::vector<AnnotationData> annos;
        annos.emplace_back(std::move(annotation));
        func.metadata->SetAnnotations(std::move(annos));
        const auto iterator = program.record_table.find(TSTYPE_ANNO_RECORD_NAME.data());
        TestAssertNotEqual(iterator, program.record_table.end());
        iterator->second.metadata->SetAccessFlags(panda::ACC_ANNOTATION);
        TestAssertTrue(program.record_table.find(TSTYPE_ANNO_RECORD_NAME.data())->second.metadata->IsAnnotation());
    }

    void EmitAndOptimize(const std::string &abc_file_name, panda::pandasm::Program &program) const
    {
        std::map<std::string, size_t> *statp = nullptr;
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = &maps;
        TestAssertTrue(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, statp, mapsp, false));
        TestAssertTrue(panda::bytecodeopt::OptimizeBytecode(&program, mapsp, abc_file_name, true));
    }

    void TypeAdaptionTest() const;
    void TypeAdaptionTest_UnconditionalJump() const;

    static constexpr int32_t NUM_TYPE = 1;
    static constexpr int32_t STR_TYPE = 4;
};

void BytecodeOptimizerTypeAdaptionTest::TypeAdaptionTest() const
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
        .function any foo(any a0, any a1, any a2) <static> {
            mov.dyn v2, a2
            mov.dyn v1, a1
            mov.dyn v0, a0
            ecma.ldlexenvdyn
            sta.dyn v6
            ldai.dyn 0x64
            sta.dyn v3
            lda.dyn v0
            sta.dyn v5
            lda.dyn v3
            ecma.greaterdyn v5
            jeqz jump_label_0
            lda.dyn v1
            sta.dyn v5
            lda.dyn v5
            return.dyn
        jump_label_0:
            lda.dyn v0
            sta.dyn v5
            lda.dyn v3
            ecma.lessdyn v5
            jeqz jump_label_1
            lda.dyn v2
            sta.dyn v5
            lda.dyn v5
            return.dyn
        jump_label_1:
            lda.dyn v1
            sta.dyn v5
            lda.dyn v2
            ecma.add2dyn v5
            sta.dyn v4
            lda.dyn v4
            sta.dyn v5
            lda.dyn v5
            return.dyn
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "foo:(any,any,any)";
    auto it = program.function_table.find(fun_name);
    TestAssertNotEqual(it, program.function_table.end());

    auto &func = it->second;
    std::vector<panda::pandasm::ScalarValue> elements;
    // set arg type
    AddTypeinfo(&elements, -1, NUM_TYPE);  // -1: first arg
    AddTypeinfo(&elements, -2, STR_TYPE);  // -2: second arg
    AddTypeinfo(&elements, -3, STR_TYPE);  // -3: third arg
    // set ins type
    const size_t LDAI_IDX = 5;
    TestAssertEqual(func.ins[LDAI_IDX].opcode, panda::pandasm::Opcode::LDAI_DYN);
    TestAssertEqual(func.ins[LDAI_IDX + 1].opcode, panda::pandasm::Opcode::STA_DYN);
    AddTypeinfo(&elements, static_cast<int32_t>(LDAI_IDX + 1), NUM_TYPE);
    const size_t ADD_IDX = 30;
    TestAssertEqual(func.ins[ADD_IDX].opcode, panda::pandasm::Opcode::ECMA_ADD2DYN);
    TestAssertEqual(func.ins[ADD_IDX + 1].opcode, panda::pandasm::Opcode::STA_DYN);
    int32_t num_invalid = std::count_if(func.ins.begin(), func.ins.begin() + ADD_IDX,
                                        [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    AddTypeinfo(&elements, ADD_IDX + 1 - num_invalid, STR_TYPE);  // exclude invalid insns because they do not emit

    SetTypeAnnotationForFunc(elements, func, program);

    EmitAndOptimize("TypeAdaptionTest.abc", program);

    // check typeinfo after optimization
    it = program.function_table.find(fun_name);
    TestAssertNotEqual(it, program.function_table.end());
    const auto &foo = it->second;
    const auto typeinfo = ExtractTypeinfo(foo);
    CheckTypeExist(typeinfo, -1, NUM_TYPE);  // -1: means first arg
    CheckTypeExist(typeinfo, -2, STR_TYPE);  // -2: means second arg
    CheckTypeExist(typeinfo, -3, STR_TYPE);  // -3: means third arg
    auto ldai_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::LDAI_DYN; });
    TestAssertNotEqual(ldai_it, foo.ins.end());
    const auto opt_ldai_idx = static_cast<size_t>(ldai_it - foo.ins.begin());
    TestAssertEqual(foo.ins[opt_ldai_idx].opcode, panda::pandasm::Opcode::LDAI_DYN);
    TestAssertTrue(opt_ldai_idx + 1 < foo.ins.size());
    TestAssertEqual(foo.ins[opt_ldai_idx + 1].opcode, panda::pandasm::Opcode::STA_DYN);

    num_invalid = std::count_if(foo.ins.begin(), ldai_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t ldai_type_idx = opt_ldai_idx - num_invalid;  // exclude invalid insns because they do not emit

    CheckTypeExist(typeinfo, ldai_type_idx + 1, NUM_TYPE);  // type is on sta.dyn

    auto add_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                               [](const auto &in) { return in.opcode == panda::pandasm::Opcode::ECMA_ADD2DYN; });
    TestAssertNotEqual(add_it, foo.ins.end());
    const auto opt_add_idx = static_cast<size_t>(add_it - foo.ins.begin());
    TestAssertEqual(foo.ins[opt_add_idx].opcode, panda::pandasm::Opcode::ECMA_ADD2DYN);
    TestAssertTrue(opt_add_idx + 1 < foo.ins.size());
    TestAssertNotEqual(foo.ins[opt_add_idx + 1].opcode, panda::pandasm::Opcode::STA_DYN);

    num_invalid = std::count_if(foo.ins.begin(), add_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t add_type_idx = opt_add_idx - num_invalid;  // exclude invalid insns because they do not emit
    CheckTypeExist(typeinfo, add_type_idx, STR_TYPE);  // type is on ecma.add2dyn as it does not have sta.dyn
}

void BytecodeOptimizerTypeAdaptionTest::TypeAdaptionTest_UnconditionalJump() const
{
    /* ts source code
    function processResults(results: number) {
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
        .function any foo(any a0) <static> {
            mov.dyn v0, a0
            ecma.ldlexenvdyn
            sta.dyn v6
            ldai.dyn 0x0
            sta.dyn v1
        jump_label_1:
            lda.dyn v1
            sta.dyn v4
            ldai.dyn 0x1
            ecma.lessdyn v4
            jeqz jump_label_0
            lda.dyn v0
            sta.dyn v4
            lda.dyn v1
            ecma.mul2dyn v4
            sta.dyn v0
            lda.dyn v1
            sta.dyn v4
            ecma.incdyn v4
            sta.dyn v1
            ecma.tonumeric v4
            jmp jump_label_1
        jump_label_0:
            lda.dyn v0
            sta.dyn v3
            ldai.dyn 0x2
            ecma.add2dyn v3
            sta.dyn v2
            lda.dyn v2
            sta.dyn v3
            lda.dyn v3
            return.dyn
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "foo:(any)";
    auto it = program.function_table.find(fun_name);
    TestAssertNotEqual(it, program.function_table.end());

    auto &func = it->second;
    std::vector<panda::pandasm::ScalarValue> elements;
    // set arg type
    AddTypeinfo(&elements, -1, NUM_TYPE);  // -1: first arg
    // set ins type
    const size_t ADD_IDX = 26;
    TestAssertEqual(func.ins[ADD_IDX].opcode, panda::pandasm::Opcode::ECMA_ADD2DYN);
    TestAssertEqual(func.ins[ADD_IDX + 1].opcode, panda::pandasm::Opcode::STA_DYN);
    int32_t num_invalid = std::count_if(func.ins.begin(), func.ins.begin() + ADD_IDX,
                                        [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    AddTypeinfo(&elements, ADD_IDX + 1 - num_invalid, STR_TYPE);  // exclude invalid insns because they do not emit

    SetTypeAnnotationForFunc(elements, func, program);

    EmitAndOptimize("TypeAdaptionTest_UnconditionalJump.abc", program);

    // check typeinfo after optimization
    it = program.function_table.find(fun_name);
    TestAssertNotEqual(it, program.function_table.end());
    const auto &foo = it->second;
    const auto typeinfo = ExtractTypeinfo(foo);
    CheckTypeExist(typeinfo, -1, NUM_TYPE);  // -1: first arg
    auto add_it = std::find_if(foo.ins.begin(), foo.ins.end(),
                               [](const auto &in) { return in.opcode == panda::pandasm::Opcode::ECMA_ADD2DYN; });
    TestAssertNotEqual(add_it, foo.ins.end());
    const auto opt_add_idx = static_cast<size_t>(add_it - foo.ins.begin());
    TestAssertEqual(foo.ins[opt_add_idx].opcode, panda::pandasm::Opcode::ECMA_ADD2DYN);
    TestAssertTrue(opt_add_idx + 1 < foo.ins.size());
    TestAssertNotEqual(foo.ins[opt_add_idx + 1].opcode, panda::pandasm::Opcode::STA_DYN);

    num_invalid = std::count_if(foo.ins.begin(), add_it,
                                [](const auto &in) { return in.opcode == panda::pandasm::Opcode::INVALID; });
    int32_t add_type_idx = opt_add_idx - num_invalid;  // exclude invalid insns because they do not emit
    CheckTypeExist(typeinfo, add_type_idx, STR_TYPE);  // type is on ecma.add2dyn as it does not have sta.dyn
}
}  // namespace panda::bytecodeopt::test

int main()
{
    panda::bytecodeopt::test::BytecodeOptimizerTypeAdaptionTest test;
    std::cout << "BytecodeOptimizerTypeAdaptionTest: " << std::endl;
    test.TypeAdaptionTest();
    std::cout << "PASS!" << std::endl;
    std::cout << "BytecodeOptimizerTypeAdaptionTest_UnconditionalJump: " << std::endl;
    test.TypeAdaptionTest_UnconditionalJump();
    std::cout << "PASS!" << std::endl;
    return 0;
}
