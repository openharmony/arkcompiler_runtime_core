/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file EXPECT in compliance with the License.
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

#include <gtest/gtest.h>
#include <iostream>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "field_data_accessor-inl.h"
#include "file_items.h"
#include "lexer.h"
#include "method_data_accessor-inl.h"
#include "param_annotations_data_accessor.h"
#include "proto_data_accessor-inl.h"
#include "utils/span.h"
#include "utils/leb128.h"
#include "utils/utf.h"
#include "bytecode_instruction-inl.h"

using namespace testing::ext;

namespace panda::pandasm {
class AssemblyEmitterTest : public testing::Test {
};

static const uint8_t *GetTypeDescriptor(const std::string &name, std::string *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

uint8_t GetSpecialOpcode(uint32_t pc_inc, int32_t line_inc)
{
    return static_cast<uint8_t>(line_inc - panda_file::LineNumberProgramItem::LINE_BASE) +
           static_cast<uint8_t>(pc_inc * panda_file::LineNumberProgramItem::LINE_RANGE) +
           panda_file::LineNumberProgramItem::OPCODE_BASE;
}

/**
 * @tc.name: assembly_emitter_test_001
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_001, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .record R {}
        .function void R.foo(R a0) <ctor> {}
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        auto *name = utf::Mutf8AsCString(pf->GetStringData(mda.GetNameId()).data);
        EXPECT_STREQ(name, ".ctor");
    });
}

/**
 * @tc.name: assembly_emitter_test_002
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_002, TestSize.Level1)
{
    Parser p;

    auto source = R"(            # 1
        .record R {              # 2
            i32 sf <static>      # 3
            i8  if               # 4
        }                        # 5
                                 # 6
        .function void main() {  # 7
            return               # 8
        }                        # 9
    )";

    std::string source_filename = "source.pa";
    auto res = p.Parse(source, source_filename);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    {
        std::string descriptor;
        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());
        EXPECT_FALSE(pf->IsExternal(class_id));

        panda_file::ClassDataAccessor cda(*pf, class_id);
        EXPECT_EQ(cda.GetSuperClassId().GetOffset(), 0U);
        EXPECT_EQ(cda.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_EQ(cda.GetFieldsNumber(), 0U);
        EXPECT_EQ(cda.GetMethodsNumber(), 1U);
        EXPECT_EQ(cda.GetIfacesNumber(), 0U);

        EXPECT_FALSE(cda.GetSourceFileId().has_value());

        cda.EnumerateAnnotations([](panda_file::File::EntityId) { EXPECT_TRUE(false); });

        cda.EnumerateFields([](panda_file::FieldDataAccessor &) { EXPECT_TRUE(false); });

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            EXPECT_FALSE(mda.IsExternal());
            EXPECT_EQ(mda.GetClassId(), class_id);
            EXPECT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("main")),
                      0);

            EXPECT_EQ(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16);

            EXPECT_EQ(mda.GetAccessFlags(), ACC_STATIC);
            EXPECT_TRUE(mda.GetCodeId().has_value());

            panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
            EXPECT_EQ(cdacc.GetNumVregs(), 0U);
            EXPECT_EQ(cdacc.GetNumArgs(), 0U);
            EXPECT_EQ(cdacc.GetCodeSize(), 1U);
            EXPECT_EQ(cdacc.GetTriesSize(), 0U);

            EXPECT_FALSE(mda.GetRuntimeParamAnnotationId().has_value());
            EXPECT_FALSE(mda.GetParamAnnotationId().has_value());
            EXPECT_TRUE(mda.GetDebugInfoId().has_value());

            panda_file::DebugInfoDataAccessor dda(*pf, mda.GetDebugInfoId().value());
            EXPECT_EQ(dda.GetLineStart(), 8U);
            EXPECT_EQ(dda.GetNumParams(), 0U);

            mda.EnumerateAnnotations([](panda_file::File::EntityId) {});
        });
    }
    {
        std::string descriptor;
        auto class_id = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        EXPECT_TRUE(class_id.IsValid());
        EXPECT_FALSE(pf->IsExternal(class_id));

        panda_file::ClassDataAccessor cda(*pf, class_id);
        EXPECT_EQ(cda.GetSuperClassId().GetOffset(), 0U);
        EXPECT_EQ(cda.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_EQ(cda.GetFieldsNumber(), 2U);
        EXPECT_EQ(cda.GetMethodsNumber(), 0U);
        EXPECT_EQ(cda.GetIfacesNumber(), 0U);

        // We emit SET_FILE in debuginfo
        EXPECT_TRUE(cda.GetSourceFileId().has_value());

        EXPECT_EQ(std::string(reinterpret_cast<const char *>(pf->GetStringData(cda.GetSourceFileId().value()).data)),
                  source_filename);

        struct FieldData {
            std::string name;
            panda_file::Type::TypeId type_id;
            uint32_t access_flags;
        };

        std::vector<FieldData> fields {{"sf", panda_file::Type::TypeId::I32, ACC_STATIC},
                                       {"if", panda_file::Type::TypeId::I8, 0}};

        size_t i = 0;
        cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
            EXPECT_FALSE(fda.IsExternal());
            EXPECT_EQ(fda.GetClassId(), class_id);
            EXPECT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(fda.GetNameId()).data,
                                               utf::CStringAsMutf8(fields[i].name.c_str())),
                      0);

            EXPECT_EQ(fda.GetType(), panda_file::Type(fields[i].type_id).GetFieldEncoding());
            EXPECT_EQ(fda.GetAccessFlags(), fields[i].access_flags);

            fda.EnumerateAnnotations([](panda_file::File::EntityId) { EXPECT_TRUE(false); });

            ++i;
        });

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &) { EXPECT_TRUE(false); });
    }
}

/**
 * @tc.name: assembly_emitter_test_003
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_003, TestSize.Level1)
{
    Parser p;

    auto source = R"(
        .function void main() {
            ldai 0      # line 3, pc 0
                        # line 4
                        # line 5
                        # line 6
                        # line 7
                        # line 8
                        # line 9
                        # line 10
                        # line 11
                        # line 12
                        # line 13
                        # line 14
            ldai 1      # line 15, pc 9
            return # line 16, pc 18
        }
    )";

    std::string source_filename = "source.pa";
    auto res = p.Parse(source, source_filename);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;
    auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
        EXPECT_TRUE(mda.GetDebugInfoId().has_value());

        panda_file::DebugInfoDataAccessor dda(*pf, mda.GetDebugInfoId().value());
        EXPECT_EQ(dda.GetLineStart(), 3U);
        EXPECT_EQ(dda.GetNumParams(), 0U);

        dda.GetLineNumberProgram();
        Span<const uint8_t> constant_pool = dda.GetConstantPool();

        std::vector<uint8_t> opcodes {static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::SET_FILE),
                                      static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::ADVANCE_PC),
                                      static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::ADVANCE_LINE),
                                      GetSpecialOpcode(0, 0),
                                      GetSpecialOpcode(9, 1),
                                      static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::END_SEQUENCE)};

        size_t size {};
        bool is_full {};
        size_t constant_pool_offset = 0;

        uint32_t offset {};

        std::tie(offset, size, is_full) = leb128::DecodeUnsigned<uint32_t>(&constant_pool[constant_pool_offset]);
        constant_pool_offset += size;
        EXPECT_TRUE(is_full);
        EXPECT_EQ(
            std::string(reinterpret_cast<const char *>(pf->GetStringData(panda_file::File::EntityId(offset)).data)),
            source_filename);

        uint32_t pc_inc;
        std::tie(pc_inc, size, is_full) = leb128::DecodeUnsigned<uint32_t>(&constant_pool[constant_pool_offset]);
        constant_pool_offset += size;
        EXPECT_TRUE(is_full);
        EXPECT_EQ(pc_inc, 0U);

        int32_t line_inc;
        std::tie(line_inc, size, is_full) = leb128::DecodeSigned<int32_t>(&constant_pool[constant_pool_offset]);
        constant_pool_offset += size;
        EXPECT_TRUE(is_full);
        EXPECT_EQ(line_inc, 5);

        EXPECT_EQ(constant_pool_offset + 1, constant_pool.size());
    });
}

/**
 * @tc.name: assembly_emitter_test_004
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_004, TestSize.Level1)
{
    Parser p;

    auto source = R"(
        .record Exception1 {}
        .record Exception2 {}

        .function any myFunction(any a0, any a1, any a2) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            ldundefined
            sta v4
        try_begin:
            jmp handler_begin1
        handler_begin1:
            ldhole
            sta v5
            jmp handler_end2
        handler_begin2:
            sta v5
        handler_end2:
            lda v4
            sta v6
            ldundefined
            eq 0x0, v6
            jeqz jump_label_4
            sta v4
        jump_label_4:
            lda v5
            sta v6
            ldhole
            sta v7
            lda v6
            noteq 0x1, v7
            jeqz jump_label_5
            lda v6
            throw
        jump_label_5:
            ldundefined
            returnundefined

        .catchall try_begin, try_begin, handler_begin1
        .catchall try_begin, handler_begin1, handler_begin2, handler_end2
        }
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    size_t i = 0;
    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
        // The NumVregs of arguments is 8U
        // The NumArgs of arguments is 3U
        // The tries size is 2U
        EXPECT_EQ(cdacc.GetNumVregs(), 8U);
        EXPECT_EQ(cdacc.GetNumArgs(), 3U);
        EXPECT_EQ(cdacc.GetTriesSize(), 2U);

        cdacc.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
            // Try start Pc is 9U
            // Catches number is 1U
            EXPECT_EQ(try_block.GetStartPc(), 9U);
            EXPECT_EQ(try_block.GetNumCatches(), 1U);

            struct CatchInfo {
                panda_file::File::EntityId type_id;
                uint32_t handler_pc;
            };
            // Exception1 class ID is 11.
            // Exception2 class ID is 16.
            // The file entity ID is 6 * 9.
            std::vector<CatchInfo> catch_infos {{pf->GetClassId(GetTypeDescriptor("Exception1", &descriptor)), 11},
                                                {pf->GetClassId(GetTypeDescriptor("Exception2", &descriptor)), 16},
                                                {panda_file::File::EntityId(), 6 * 9}};

            try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
                EXPECT_EQ(catch_block.GetHandlerPc(), catch_infos[i].handler_pc);
                ++i;

                return true;
            });

            return true;
        });
    });
}

/**
 * @tc.name: assembly_emitter_test_005
 * @tc.desc: Verify the AsmEmitter::GetLastError() function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_005, TestSize.Level1)
{
    {
        Parser p;
        auto source = R"(
            .record A {
                B b
            }
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_EQ(pf, nullptr);
        EXPECT_EQ(AsmEmitter::GetLastError(), "Field A.b has undefined type");
    }

    {
        Parser p;
        auto source = R"(
            .function void A.b() {}
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_EQ(pf, nullptr);
        EXPECT_EQ(AsmEmitter::GetLastError(), "Function A.b is bound to undefined record A");
    }

    {
        Parser p;
        auto source = R"(
            .function void a(b a0) {}
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_EQ(pf, nullptr);
        EXPECT_EQ(AsmEmitter::GetLastError(), "Argument 0 of function a has undefined type");
    }

    {
        Parser p;
        auto source = R"(
            .record A <external>
            .function void A.x() {}
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_EQ(pf, nullptr);
        EXPECT_EQ(AsmEmitter::GetLastError(), "Non-external function A.x is bound to external record");
    }
}

/**
 * @tc.name: assembly_emitter_test_006
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_006, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .function void foo() {}
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    EXPECT_FALSE(cda.GetSourceLang());

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) { EXPECT_FALSE(mda.GetSourceLang()); });
}

/**
 * @tc.name: assembly_emitter_test_007
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_007, TestSize.Level1)
{
    {
        Parser p;
        auto source = R"(
            .record R {}
            .function void R.foo(R a0) <ctor> {}
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_NE(pf, nullptr);

        std::string descriptor;

        auto class_id = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            auto *name = utf::Mutf8AsCString(pf->GetStringData(mda.GetNameId()).data);
            EXPECT_STREQ(name, ".ctor");
        });
    }

    {
        Parser p;
        auto source = R"(
            .record R {}
            .function void R.foo(R a0) <cctor> {}
        )";

        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        EXPECT_NE(pf, nullptr);

        std::string descriptor;

        auto class_id = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            auto *name = utf::Mutf8AsCString(pf->GetStringData(mda.GetNameId()).data);
            EXPECT_STREQ(name, ".cctor");
        });
    }
}

/**
 * @tc.name: assembly_emitter_test_008
 * @tc.desc: Verify the EnumerateFields function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_008, TestSize.Level1)
{
    Parser p;

    auto source = R"(
        .record panda.String <external>

        .record R {
            u1 f_u1 <value=1>
            i8 f_i8 <value=2>
            u8 f_u8 <value=128>
            i16 f_i16 <value=256>
            u16 f_u16 <value=32768>
            i32 f_i32 <value=65536>
            u32 f_u32 <value=2147483648>
            i64 f_i64 <value=4294967296>
            u64 f_u64 <value=9223372036854775808>
            f32 f_f32 <value=1.0>
            f64 f_f64 <value=2.0>
            panda.String f_str <value="str">
        }
    )";

    struct FieldData {
        std::string name;
        panda_file::Type::TypeId type_id;
        std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double, std::string> value;
    };

    uint32_t f_u1 = 1;
    int32_t f_i8 = 2;
    uint32_t f_u8 = 128;
    int32_t f_i16 = 256;
    uint32_t f_u16 = 32768;
    int32_t f_i32 = 65536;
    uint32_t f_u32 = 2147483648;
    int64_t f_i64 = 4294967296;
    uint64_t f_u64 = 9223372036854775808ULL;
    float f_f32 = 1.0;
    double f_f64 = 2.0;

    std::vector<FieldData> data {
        {"f_u1", panda_file::Type::TypeId::U1, f_u1},    {"f_i8", panda_file::Type::TypeId::I8, f_i8},
        {"f_u8", panda_file::Type::TypeId::U8, f_u8},    {"f_i16", panda_file::Type::TypeId::I16, f_i16},
        {"f_u16", panda_file::Type::TypeId::U16, f_u16}, {"f_i32", panda_file::Type::TypeId::I32, f_i32},
        {"f_u32", panda_file::Type::TypeId::U32, f_u32}, {"f_i64", panda_file::Type::TypeId::I64, f_i64},
        {"f_u64", panda_file::Type::TypeId::U64, f_u64}, {"f_f32", panda_file::Type::TypeId::F32, f_f32},
        {"f_f64", panda_file::Type::TypeId::F64, f_f64}, {"f_str", panda_file::Type::TypeId::REFERENCE, "str"}};

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;
    auto class_id = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
    EXPECT_TRUE(class_id.IsValid());
    EXPECT_FALSE(pf->IsExternal(class_id));

    panda_file::ClassDataAccessor cda(*pf, class_id);
    EXPECT_EQ(cda.GetFieldsNumber(), data.size());

    auto panda_string_id = pf->GetClassId(GetTypeDescriptor("panda.String", &descriptor));

    size_t idx = 0;
    cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
        const FieldData &field_data = data[idx];

        EXPECT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(fda.GetNameId()).data,
                                           utf::CStringAsMutf8(field_data.name.c_str())),
                  0);

        panda_file::Type type(field_data.type_id);
        uint32_t type_value;
        if (type.IsReference()) {
            type_value = panda_string_id.GetOffset();
        } else {
            type_value = type.GetFieldEncoding();
        }

        switch (field_data.type_id) {
            case panda_file::Type::TypeId::U1: {
                auto result = fda.GetValue<uint8_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<uint32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::I8: {
                auto result = fda.GetValue<int8_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<int32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::U8: {
                auto result = fda.GetValue<uint8_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<uint32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::I16: {
                auto result = fda.GetValue<int16_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<int32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::U16: {
                auto result = fda.GetValue<uint16_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<uint32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::I32: {
                auto result = fda.GetValue<int32_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<int32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::U32: {
                auto result = fda.GetValue<uint32_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<uint32_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::I64: {
                auto result = fda.GetValue<int64_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<int64_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::U64: {
                auto result = fda.GetValue<uint64_t>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<uint64_t>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::F32: {
                auto result = fda.GetValue<float>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<float>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::F64: {
                auto result = fda.GetValue<double>();
                EXPECT_TRUE(result);
                EXPECT_EQ(result.value(), std::get<double>(field_data.value));
                break;
            }
            case panda_file::Type::TypeId::REFERENCE: {
                auto result = fda.GetValue<uint32_t>();
                EXPECT_TRUE(result);

                panda_file::File::EntityId string_id(result.value());
                auto val = std::get<std::string>(field_data.value);

                EXPECT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(string_id).data, utf::CStringAsMutf8(val.c_str())),
                          0);
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }

        ++idx;
    });
}

/**
 * @tc.name: assembly_emitter_test_009
 * @tc.desc: Verify the EnumerateMethods function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_009, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .function any foo(any a0) <noimpl>
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    int32_t num_methods = 0;
    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        EXPECT_FALSE(mda.IsExternal());
        EXPECT_EQ(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16);
        EXPECT_TRUE(mda.GetCodeId().has_value());
        panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
        EXPECT_EQ(1u, cdacc.GetNumArgs());

        ++num_methods;
    });
    EXPECT_EQ(1, num_methods);
}

/**
 * @tc.name: assembly_emitter_test_010
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_010, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .record Test {
            any foo
        }
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("Test", &descriptor));
    EXPECT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    size_t num_fields = 0;
    const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
    cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
        uint32_t type = fda.GetType();
        EXPECT_EQ(tagged.GetFieldEncoding(), type);

        ++num_fields;
    });
    EXPECT_EQ(1, num_fields);
}

/**
 * @tc.name: assembly_emitter_test_011
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_011, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Test {
            any foo
        }
    )";
    Parser p2;
    auto source2 = R"(
        .function void main() {
            ldai 0
            ldai 1
            return
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto pf = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(pf);
}

/**
 * @tc.name: assembly_emitter_test_012
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_012, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .record Math <external>
        .function i64 Math.minI64(i64 a0) <external>
        .function i64 main(i64 a0, i64 a1) {
            ldglobalvar 0x9, "Math.minI64"
            callarg1 0x4, a0
            return
        }
    )";
    auto ret = par.Parse(source);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);
    auto pf = AsmEmitter::Emit(ret.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptors;

    auto class_id = pf->GetClassId(GetTypeDescriptor("Math", &descriptors));
    EXPECT_TRUE(class_id.IsValid());
}

/**
 * @tc.name: assembly_emitter_test_013
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_013, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .array array {
            u1 1
            u8 2
            i8 -30
            u16 400
            i16 -5000
            u32 60000
            i32 -700000
            u64 8000000
            i64 -90000000
            f32 12
            f64 12
        }
    )";
    std::string source_filename = "source.pa";
    auto program = par.Parse(source, source_filename);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);
    auto pf = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_014
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_014, TestSize.Level1)
{
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    Parser par;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("panda.String \"a\"").first);
    v.push_back(l.TokenizeString("panda.String \"ab\"").first);
    v.push_back(l.TokenizeString("panda.String \"abc\"").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".array array_static panda.String 3 { \"a\" \"ab\" \"abc\" }").first);
    auto program = par.Parse(v);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);
    auto pf = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_015
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_015, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .record R {
            R[][] f
        }
        .function any f(i8 a0) {
            sta a0
        }
        .array array {
            u1 1
        }
    )";
    std::string source_filename = "source.pa";
    auto program = par.Parse(source, source_filename);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);

    panda::pandasm::Type type;
    ScalarValue insn_order(ScalarValue::Create<panda::pandasm::Value::Type::RECORD>(type));
    program.Value().record_table.at("R").field_list[0].metadata->SetValue(insn_order);

    auto pf = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf, nullptr);

    program.Value().literalarray_table.at("array").literals_[0].tag_ = panda_file::LiteralTag::LITERALARRAY;
    program.Value().literalarray_table.at("array").literals_[0].value_.emplace<std::string>("array");
    ScalarValue insn_order_lite(ScalarValue::Create<panda::pandasm::Value::Type::LITERALARRAY>("array"));
    program.Value().record_table.at("R").field_list[0].metadata->SetValue(insn_order_lite);
    auto pf1 = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf1, nullptr);

    ScalarValue insn_order_enum(ScalarValue::Create<panda::pandasm::Value::Type::ENUM>("R.f"));
    program.Value().record_table.at("R").field_list[0].metadata->SetValue(insn_order_enum);
    program.Value().record_table.at("R").field_list[0].metadata->SetAttributeValue("external", "enum");
    auto pf2 = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf2, nullptr);

    ScalarValue insn_order_method(ScalarValue::Create<panda::pandasm::Value::Type::METHOD>("f:(i8)"));
    program.Value().record_table.at("R").field_list[0].metadata->SetValue(insn_order_method);
    auto pf3 = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf3, nullptr);

    ScalarValue insn_orders(ScalarValue::Create<panda::pandasm::Value::Type::I32>(1));
    std::vector<panda::pandasm::ScalarValue> elements;
    elements.emplace_back(std::move(insn_orders));

    ArrayValue array_value(panda::pandasm::Value::Type::I32, elements);
    AnnotationElement anno_element("_TypeOfInstruction", std::make_unique<ArrayValue>(array_value));
    AnnotationData annotation("_ESSlotNumberAnnotation");
    annotation.AddElement(std::move(anno_element));

    ScalarValue insn_order_anno(ScalarValue::Create<panda::pandasm::Value::Type::ANNOTATION>(annotation));

    static const std::string SLOT_NUMBER = "_ESSlotNumberAnnotation";
    pandasm::Record record(SLOT_NUMBER, pandasm::extensions::Language::ECMASCRIPT);
    record.metadata->AddAccessFlags(panda::ACC_ANNOTATION);
    program.Value().record_table.emplace(SLOT_NUMBER, std::move(record));
    program.Value().record_table.at("R").field_list[0].metadata->SetValue(insn_order_anno);
    auto pf4 = AsmEmitter::Emit(program.Value());
    EXPECT_NE(pf4, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_016
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_016, TestSize.Level1)
{
    Parser par;
    std::vector<std::vector<panda::pandasm::Token>> v;
    Lexer l;
    v.push_back(l.TokenizeString(".array array {").first);
    v.push_back(l.TokenizeString("u1 1").first);
    v.push_back(l.TokenizeString("u8 2").first);
    v.push_back(l.TokenizeString("i8 -30").first);
    v.push_back(l.TokenizeString("u16 400").first);
    v.push_back(l.TokenizeString("i16 -5000").first);
    v.push_back(l.TokenizeString("u32 60000").first);
    v.push_back(l.TokenizeString("i32 -700000").first);
    v.push_back(l.TokenizeString("u64 8000000").first);
    v.push_back(l.TokenizeString("i64 -90000000").first);
    v.push_back(l.TokenizeString("}").first);
    v.push_back(l.TokenizeString(".function any f(i8 a0) { sta a0 }").first);
    std::string source_filename = "source.pa";
    auto item = par.Parse(v, source_filename);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);

    item.Value().literalarray_table.at("array").literals_[0].tag_ = panda_file::LiteralTag::BOOL;
    item.Value().literalarray_table.at("array").literals_[0].value_.emplace<0>(true);

    item.Value().literalarray_table.at("array").literals_[1].tag_ = panda_file::LiteralTag::METHODAFFILIATE;
    item.Value().literalarray_table.at("array").literals_[1].value_.emplace<uint16_t>(1);

    item.Value().literalarray_table.at("array").literals_[2].tag_ = panda_file::LiteralTag::FLOAT;
    item.Value().literalarray_table.at("array").literals_[2].value_.emplace<float>(1.0);

    item.Value().literalarray_table.at("array").literals_[3].tag_ = panda_file::LiteralTag::DOUBLE;
    item.Value().literalarray_table.at("array").literals_[3].value_.emplace<double>(1.0);

    item.Value().literalarray_table.at("array").literals_[4].tag_ = panda_file::LiteralTag::STRING;
    item.Value().literalarray_table.at("array").literals_[4].value_.emplace<std::string>("1.0");

    item.Value().literalarray_table.at("array").literals_[5].tag_ = panda_file::LiteralTag::ASYNCGENERATORMETHOD;
    item.Value().literalarray_table.at("array").literals_[5].value_.emplace<std::string>("f:(i8)");

    item.Value().literalarray_table.at("array").literals_[6].tag_ = panda_file::LiteralTag::LITERALARRAY;
    item.Value().literalarray_table.at("array").literals_[6].value_.emplace<std::string>("array");

    auto pf = AsmEmitter::Emit(item.Value());
    EXPECT_NE(pf, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_017
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_017, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .record Asm1 {
            i64 asm1
            void asm2
            i32 asm3
        }
    )";
    std::string source_filename = "source.pa";
    auto item = par.Parse(source, source_filename);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);

    EXPECT_EQ(item.Value().record_table.at("Asm1").name, "Asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].name, "asm1");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[0].type.GetId(), panda::panda_file::Type::TypeId::I64);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].name, "asm2");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[1].type.GetId(), panda::panda_file::Type::TypeId::VOID);
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].name, "asm3");
    EXPECT_EQ(item.Value().record_table.at("Asm1").field_list[2].type.GetId(), panda::panda_file::Type::TypeId::I32);

    auto pf = AsmEmitter::Emit(item.Value());
    EXPECT_NE(pf, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_018
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_018, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .function any f(i8 a0) {
            sta a0
        }
    )";
    std::string source_filename = "source.pa";
    AsmEmitter::PandaFileToPandaAsmMaps *maps = nullptr;
    auto items = panda::panda_file::ItemContainer {};
    auto program = par.Parse(source, source_filename);
    program.Value().function_table.at("f:(i8)").metadata->SetAttribute("external");
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);
    auto pf = AsmEmitter::Emit(&items, program.Value(), maps, false);
    EXPECT_EQ(pf, true);
}

/**
 * @tc.name: assembly_emitter_test_019
 * @tc.desc: Verify the AsmEmitter::Emit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_019, TestSize.Level1)
{
    Parser par;
    auto source = R"(
        .function any f(i8 a0) {
            sta a0
        }
    )";
    std::string source_filename = "source.pa";

    auto program = par.Parse(source, source_filename);
    program.Value().function_table.at("f:(i8)").metadata->SetAttribute("external");
    panda::pandasm::debuginfo::LocalVariable local;
    local.name = "test";
    program.Value().function_table.at("f:(i8)").local_variable_debug.push_back(local);
    EXPECT_EQ(par.ShowError().err, Error::ErrorType::ERR_NONE);
    auto success = AsmEmitter::Emit(program.Value());
    EXPECT_NE(success, nullptr);
}

/**
 * @tc.name: assembly_emitter_test_020
 * @tc.desc: Verify the EmitPrograms function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_020, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Test {
            any foo
        }
    )";
    Parser p2;
    auto source2 = R"(
        .function void main() {
            ldai 0
            ldai 1
            return
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto success = AsmEmitter::EmitPrograms(filename, progs, true);
    EXPECT_TRUE(success);
}

/**
 * @tc.name: assembly_emitter_test_021
 * @tc.desc: Verify the AsmEmitter::EmitPrograms function with different API version.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_021, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .function any foo(any a0) <noimpl>
    )";

    // api 11
    {
        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        std::vector<Program *> progs;
        progs.push_back(&res.Value());

        std::string descriptor;
        const std::string filename_api11 = "source_021_api11.abc";
        EmitterConfig emitConfig {11};
        auto is_emitted = AsmEmitter::EmitPrograms(filename_api11, progs, false, emitConfig);
        EXPECT_TRUE(is_emitted);
        auto pf = panda_file::OpenPandaFile(filename_api11);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            EXPECT_NE(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16);
            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            EXPECT_EQ(tagged, pda.GetReturnType());
            EXPECT_EQ(1u, pda.GetNumArgs());
            EXPECT_EQ(tagged, pda.GetArgType(0));
        });
    }

    // api 12
    {
        auto res = p.Parse(source);
        EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        std::vector<Program *> progs;
        progs.push_back(&res.Value());

        std::string descriptor;
        const std::string filename_api12 = "source_021_api12.abc";
        EmitterConfig emitConfig {12};
        auto is_emitted = AsmEmitter::EmitPrograms(filename_api12, progs, false, emitConfig);
        EXPECT_TRUE(is_emitted);
        auto pf = panda_file::OpenPandaFile(filename_api12);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        cda.EnumerateMethods(
            [&](panda_file::MethodDataAccessor &mda) { EXPECT_EQ(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16); });
    }
}

/**
 * @tc.name: assembly_emitter_test_022
 * @tc.desc: Verify the AsmEmitter::Emit function with different API version.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_022, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .function any foo(any a0) <noimpl>
    )";
    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    // api 11
    {
        std::string descriptor;
        const std::string filename_api11 = "source_022_api11.abc";
        auto writer = panda_file::FileWriter(filename_api11);
        auto is_emitted = AsmEmitter::Emit(&writer, res.Value(), nullptr, nullptr, false, nullptr, 11);
        EXPECT_TRUE(is_emitted);
        auto pf = panda_file::OpenPandaFile(filename_api11);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            EXPECT_NE(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16);
            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            EXPECT_EQ(tagged, pda.GetReturnType());
            EXPECT_EQ(1u, pda.GetNumArgs());
            EXPECT_EQ(tagged, pda.GetArgType(0));
        });
    }

    // api 12
    {
        std::string descriptor;
        const std::string filename_api12 = "source_022_api12.abc";
        auto writer = panda_file::FileWriter(filename_api12);
        auto is_emitted = AsmEmitter::Emit(&writer, res.Value(), nullptr, nullptr, false, nullptr, 12);
        EXPECT_TRUE(is_emitted);
        auto pf = panda_file::OpenPandaFile(filename_api12);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        cda.EnumerateMethods(
            [&](panda_file::MethodDataAccessor &mda) { EXPECT_EQ(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16); });
    }
}

/**
 * @tc.name: assembly_emitter_test_023
 * @tc.desc: Verify the AsmEmitter::Emit function with different API version.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_023, TestSize.Level1)
{
    Parser p;
    auto source = R"(
        .function any foo(any a0) <noimpl>
    )";

    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    // api 11
    {
        std::string descriptor;
        auto pf = AsmEmitter::Emit(res.Value(), nullptr, 11);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            EXPECT_NE(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16);
            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            EXPECT_EQ(tagged, pda.GetReturnType());
            EXPECT_EQ(1u, pda.GetNumArgs());
            EXPECT_EQ(tagged, pda.GetArgType(0));
        });
    }

    // api 12
    {
        std::string descriptor;
        auto pf = AsmEmitter::Emit(res.Value(), nullptr, 12);
        EXPECT_NE(pf, nullptr);

        auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        EXPECT_TRUE(class_id.IsValid());

        panda_file::ClassDataAccessor cda(*pf, class_id);

        EXPECT_FALSE(cda.GetSourceLang());

        cda.EnumerateMethods(
            [&](panda_file::MethodDataAccessor &mda) { EXPECT_EQ(mda.GetProtoIdx(), panda_file::INVALID_INDEX_16); });
    }
}

/**
 * @tc.name: assembly_emitter_test_024
 * @tc.desc: Verify the AsmEmitter::Emit Handle Same Record.
 * @tc.type: FUNC
 * @tc.require: I9OWSZ
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_024, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Rec {
            u8 rec <value=0>
        }
    )";
    Parser p2;
    auto source2 = R"(
        .record Rec {
            u8 rec <value=0>
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto pf = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(pf);
}

/**
 * @tc.name: assembly_emitter_test_025
 * @tc.desc: Verify the AsmEmitter::Emit Handle Same Record.
 * @tc.type: FUNC
 * @tc.require: I9OWSZ
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_025, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Rec {
            u8 rec <value=0>
        }
    )";
    Parser p2;
    auto source2 = R"(
        .record Rec {
            u8 rec <value=1>
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto pf = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_FALSE(pf);
    EXPECT_EQ(AsmEmitter::GetLastError(), "Field {Rec.rec} has different value.");
}

/**
 * @tc.name: assembly_emitter_test_026
 * @tc.desc: Verify the AsmEmitter::Emit Handle Same Record.
 * @tc.type: FUNC
 * @tc.require: I9OWSZ
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_026, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Rec {
            u8 rec1 <value=0>
        }
    )";
    Parser p2;
    auto source2 = R"(
        .record Rec {
            u8 rec2 <value=0>
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto pf = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(pf);
}

/**
 * @tc.name: assembly_emitter_test_027
 * @tc.desc: Verify the AsmEmitter::EmitProgram Handle Function.
 * @tc.type: FUNC
 * @tc.require: I9OWSZ
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_027, TestSize.Level1)
{
    Parser p1;
    auto source1 = R"(
        .record Test {
            any foo
        }
        .function any foo() {
            ldai 0
            ldai 1
            return
        }
        .function any foo1() {
            ldai 0
            ldai 1
            return
        }
    )";
    Parser p2;
    auto source2 = R"(
        .function any main() {
            ldai 0
            ldai 1
            return
        }
        .function any func1() {
            ldai 0
            ldai 1
            return
        }
        .function any func2() {
            ldai 0
            ldai 1
            return
        }
    )";
    auto res1 = p1.Parse(source1);
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);

    auto res2 = p2.Parse(source2);
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    std::vector<Program *> progs;
    progs.push_back(&res1.Value());
    progs.push_back(&res2.Value());
    const std::string filename = "source.pa";
    ;
    auto pf = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(pf);
}

// --- Helpers for dedup-literal-array tests ---

static LiteralArray MakeU32LiteralArray(uint32_t v)
{
    LiteralArray la;
    la.literals_.resize(1);
    la.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U32;
    la.literals_[0].value_.emplace<uint32_t>(v);
    return la;
}

// Attach a function with a CREATEARRAYWITHBUFFER instruction referencing the given literal-array id.
// The dedup whitelist requires arrays to be referenced by this instruction type to participate.
static void AttachArrayBuffer(Program &prog, const std::string &funcName, const std::string &litArrayId)
{
    std::vector<std::uint16_t> regs;
    std::vector<IType> imms = {static_cast<int64_t>(0)};
    std::vector<std::string> ids = {litArrayId};
    Function fn(funcName, panda_file::SourceLang::ECMASCRIPT);
    fn.ins.push_back(InsPtr(Ins::CreateIns(Opcode::CREATEARRAYWITHBUFFER, regs, imms, ids)));
    prog.function_table.emplace(funcName, std::move(fn));
}

// Attach a function with a CREATEOBJECTWITHBUFFER instruction referencing the given literal-array id.
static void AttachObjectBuffer(Program &prog, const std::string &funcName, const std::string &litArrayId)
{
    std::vector<std::uint16_t> regs;
    std::vector<IType> imms = {static_cast<int64_t>(0)};
    std::vector<std::string> ids = {litArrayId};
    Function fn(funcName, panda_file::SourceLang::ECMASCRIPT);
    fn.ins.push_back(InsPtr(Ins::CreateIns(Opcode::CREATEOBJECTWITHBUFFER, regs, imms, ids)));
    prog.function_table.emplace(funcName, std::move(fn));
}

static AnnotationData MakeLiteralArrayAnnotation(const std::string &litArrayId)
{
    AnnotationElement elem("data", std::make_unique<ScalarValue>(
                                       ScalarValue::Create<Value::Type::LITERALARRAY>(std::string_view(litArrayId))));
    std::vector<AnnotationElement> elements;
    elements.push_back(std::move(elem));
    return AnnotationData("Anno", std::move(elements));
}

/**
 * @tc.name: assembly_emitter_test_028
 * @tc.desc: Verify AsmEmitter::DeduplicateLiteralArrays merges identical literal arrays across
 *           programs.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_028, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["arr_0"] = MakeU32LiteralArray(1);
    AttachArrayBuffer(res1.Value(), "s1.func_main_0", "arr_0");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    // Same content as arr_0 but a different id.
    res2.Value().literalarray_table["arr_1"] = MakeU32LiteralArray(1);
    AttachArrayBuffer(res2.Value(), "s2.func_main_0", "arr_1");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // Canonical entry is kept in the first program.
    EXPECT_EQ(res1.Value().literalarray_table.size(), 1U);
    EXPECT_TRUE(res1.Value().literalarray_table.count("arr_0") > 0);
    // The duplicate entry is removed from the second program.
    EXPECT_EQ(res2.Value().literalarray_table.size(), 0U);
    EXPECT_FALSE(res2.Value().literalarray_table.count("arr_1") > 0);
}

/**
 * @tc.name: assembly_emitter_test_029
 * @tc.desc: Verify AsmEmitter::DeduplicateLiteralArrays keeps arrays with distinct content.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_029, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);

    auto la1 = LiteralArray();
    la1.literals_.resize(1);
    la1.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U32;
    la1.literals_[0].value_.emplace<uint32_t>(1);
    res1.Value().literalarray_table["arr_0"] = la1;
    AttachArrayBuffer(res1.Value(), "s1.func_main_0", "arr_0");

    auto la2 = LiteralArray();
    la2.literals_.resize(1);
    la2.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U32;
    la2.literals_[0].value_.emplace<uint32_t>(2);
    res2.Value().literalarray_table["arr_1"] = la2;
    AttachArrayBuffer(res2.Value(), "s2.func_main_0", "arr_1");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    EXPECT_EQ(res1.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 1U);
    EXPECT_TRUE(res1.Value().literalarray_table.count("arr_0") > 0);
    EXPECT_TRUE(res2.Value().literalarray_table.count("arr_1") > 0);
}

/**
 * @tc.name: assembly_emitter_test_030
 * @tc.desc: Verify DeduplicateLiteralArrays deduplicates arrays that reference nested arrays with
 *          identical content but different ids (nested references are compared by content, not by id).
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_030, TestSize.Level1)
{
    auto makeU32 = [](uint32_t v) {
        LiteralArray la;
        la.literals_.resize(1);
        la.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U32;
        la.literals_[0].value_.emplace<uint32_t>(v);
        return la;
    };
    auto makeRef = [](const std::string &nestedId) {
        LiteralArray la;
        la.literals_.resize(1);
        la.literals_[0].tag_ = panda_file::LiteralTag::LITERALARRAY;
        la.literals_[0].value_.emplace<std::string>(nestedId);
        return la;
    };

    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["n0"] = makeU32(1);
    res1.Value().literalarray_table["o0"] = makeRef("n0");
    AttachArrayBuffer(res1.Value(), "s1.func_main_0", "n0");
    AttachArrayBuffer(res1.Value(), "s1.func_main_1", "o0");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    res2.Value().literalarray_table["n1"] = makeU32(1);     // same content as n0, different id
    res2.Value().literalarray_table["o1"] = makeRef("n1");  // structurally equal to o0
    AttachArrayBuffer(res2.Value(), "s2.func_main_0", "n1");
    AttachArrayBuffer(res2.Value(), "s2.func_main_1", "o1");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // The canonical arrays stay in program 1.
    EXPECT_EQ(res1.Value().literalarray_table.size(), 2U);
    EXPECT_TRUE(res1.Value().literalarray_table.count("n0") > 0);
    EXPECT_TRUE(res1.Value().literalarray_table.count("o0") > 0);
    // Both duplicates (nested and the referencing array that is equal through nesting) are removed.
    EXPECT_EQ(res2.Value().literalarray_table.size(), 0U);
    EXPECT_FALSE(res2.Value().literalarray_table.count("n1") > 0);
    EXPECT_FALSE(res2.Value().literalarray_table.count("o1") > 0);
    // The surviving referencing array still points at its canonical nested array.
    EXPECT_EQ(std::get<std::string>(res1.Value().literalarray_table.at("o0").literals_[0].value_), "n0");
}

/**
 * @tc.name: assembly_emitter_test_031
 * @tc.desc: Verify EmitPrograms integrates DeduplicateLiteralArrays: two programs carrying
 *          identical-content literal arrays produce a merged abc with a single literal array entry.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_031, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["arr_0"] = MakeU32LiteralArray(1);
    res1.Value().array_types.insert(Type("u32", 1));
    AttachArrayBuffer(res1.Value(), "R1.func_main_0", "arr_0");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    // Same content as arr_0 but a different id — should be deduplicated by EmitPrograms.
    res2.Value().literalarray_table["arr_1"] = MakeU32LiteralArray(1);
    res2.Value().array_types.insert(Type("u32", 1));
    AttachArrayBuffer(res2.Value(), "R2.func_main_0", "arr_1");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    const std::string filename = "source.pa";
    bool ok = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(ok);
}

/**
 * @tc.name: assembly_emitter_test_032
 * @tc.desc: Verify DeduplicateLiteralArrays handles edge cases: empty input is a no-op, and
 *          duplicate arrays within a single program are also deduplicated.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_032, TestSize.Level1)
{
    // Empty progs vector — must not crash and must be a no-op.
    std::vector<Program *> emptyProgs;
    AsmEmitter::DeduplicateLiteralArrays(emptyProgs);
    SUCCEED();

    // Single program with two identical-content arrays under different ids.
    Parser p;
    auto res = p.Parse(R"(.record R { u32 f })", "s.pa");
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);
    res.Value().literalarray_table["a0"] = MakeU32LiteralArray(7);
    res.Value().literalarray_table["a1"] = MakeU32LiteralArray(7);  // duplicate of a0
    AttachArrayBuffer(res.Value(), "s.func_main_0", "a0");
    AttachArrayBuffer(res.Value(), "s.func_main_1", "a1");

    std::vector<Program *> progs {&res.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // Only the canonical (first) array survives.
    EXPECT_EQ(res.Value().literalarray_table.size(), 1U);
    EXPECT_TRUE(res.Value().literalarray_table.count("a0") > 0);
    EXPECT_FALSE(res.Value().literalarray_table.count("a1") > 0);
}

// --- Helpers for test_034 (order/type sensitivity) ---

static LiteralArray MakeU32PairLiteralArray(uint32_t a, uint32_t b)
{
    constexpr size_t pairSize = 2;
    LiteralArray la;
    la.literals_.resize(pairSize);
    la.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U32;
    la.literals_[0].value_.emplace<uint32_t>(a);
    la.literals_[1].tag_ = panda_file::LiteralTag::ARRAY_U32;
    la.literals_[1].value_.emplace<uint32_t>(b);
    return la;
}

static LiteralArray MakeU16LiteralArray(uint16_t v)
{
    LiteralArray la;
    la.literals_.resize(1);
    la.literals_[0].tag_ = panda_file::LiteralTag::ARRAY_U16;
    la.literals_[0].value_.emplace<uint16_t>(v);
    return la;
}

/**
 * @tc.name: assembly_emitter_test_033
 * @tc.desc: Verify DeduplicateLiteralArrays does NOT merge arrays whose content differs by
 *          element order or by element type — only byte-identical content is deduplicated.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_033, TestSize.Level1)
{
    // --- Element order matters: [1, 2] != [2, 1] ---
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["ord0"] = MakeU32PairLiteralArray(1, 2);
    AttachArrayBuffer(res1.Value(), "s1.func_main_0", "ord0");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    res2.Value().literalarray_table["ord1"] = MakeU32PairLiteralArray(2, 1);  // reversed order
    AttachArrayBuffer(res2.Value(), "s2.func_main_0", "ord1");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    EXPECT_EQ(res1.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 1U);
    EXPECT_TRUE(res1.Value().literalarray_table.count("ord0") > 0);
    EXPECT_TRUE(res2.Value().literalarray_table.count("ord1") > 0);

    // --- Element type matters: [u32 1] != [u16 1] ---
    Parser p3;
    auto res3 = p3.Parse(R"(.record R3 { u32 f })", "s3.pa");
    EXPECT_EQ(p3.ShowError().err, Error::ErrorType::ERR_NONE);
    res3.Value().literalarray_table["t0"] = MakeU32LiteralArray(1);
    AttachArrayBuffer(res3.Value(), "s3.func_main_0", "t0");

    Parser p4;
    auto res4 = p4.Parse(R"(.record R4 { u32 f })", "s4.pa");
    EXPECT_EQ(p4.ShowError().err, Error::ErrorType::ERR_NONE);
    res4.Value().literalarray_table["t1"] = MakeU16LiteralArray(1);
    AttachArrayBuffer(res4.Value(), "s4.func_main_0", "t1");

    std::vector<Program *> progs2 {&res3.Value(), &res4.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs2);

    EXPECT_EQ(res3.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res4.Value().literalarray_table.size(), 1U);
    EXPECT_TRUE(res3.Value().literalarray_table.count("t0") > 0);
    EXPECT_TRUE(res4.Value().literalarray_table.count("t1") > 0);
}

/**
 * @tc.name: assembly_emitter_test_034
 * @tc.desc: Verify EmitPrograms succeeds when annotation element values reference literal arrays
 *           that are deduplicated across merged programs — the original crash scenario.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_034, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["arr_0"] = MakeU32LiteralArray(1);
    res1.Value().array_types.insert(Type("u32", 1));
    res1.Value().isGeneratedFromMergedAbc = true;
    AttachArrayBuffer(res1.Value(), "R1.func_main_0", "arr_0");
    // Create the annotation record so the emitter can find it in the ClassMap.
    Record annoRecord("Anno", panda::panda_file::SourceLang::ECMASCRIPT);
    annoRecord.metadata->SetAccessFlags(panda::ACC_ANNOTATION);
    res1.Value().record_table.emplace("Anno", std::move(annoRecord));

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    // Same content as arr_0 — duplicate will be removed during EmitPrograms.
    res2.Value().literalarray_table["arr_1"] = MakeU32LiteralArray(1);
    res2.Value().array_types.insert(Type("u32", 1));
    res2.Value().isGeneratedFromMergedAbc = true;
    AttachArrayBuffer(res2.Value(), "R2.func_main_0", "arr_1");

    // Annotation on the record references the canonical id — per the id-per-purpose contract,
    // annotations never share ids with buffer instructions, so dedup cannot dangle them.
    std::vector<AnnotationData> recAnnos;
    recAnnos.push_back(MakeLiteralArrayAnnotation("arr_0"));
    res2.Value().record_table.at("R2").metadata->SetAnnotations(std::move(recAnnos));

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    const std::string filename = "anno_dedup.pa";
    bool ok = AsmEmitter::EmitPrograms(filename, progs, false);
    EXPECT_TRUE(ok);
}

// Attach a class-definition instruction (layout id at slot 1); such ids are excluded
// from deduplication.
static void AttachClassDefFunction(Program &prog, const std::string &funcName, Opcode op, const std::string &methodId,
                                   const std::string &litArrayId)
{
    std::vector<std::uint16_t> regs = {0};
    std::vector<IType> imms = {static_cast<int64_t>(0), static_cast<int64_t>(0)};
    std::vector<std::string> ids = {methodId, litArrayId};
    Function fn(funcName, panda_file::SourceLang::ECMASCRIPT);
    fn.ins.push_back(InsPtr(Ins::CreateIns(op, regs, imms, ids)));
    prog.function_table.emplace(funcName, std::move(fn));
}

/**
 * @tc.name: assembly_emitter_test_035
 * @tc.desc: Verify DeduplicateLiteralArrays does NOT merge class layout arrays (referenced by
 *           DEFINECLASSWITHBUFFER / CALLRUNTIME_DEFINESENDABLECLASS) with other arrays of identical
 *           content, because the runtime may mutate those layout buffers at class-definition time.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_035, TestSize.Level1)
{
    // Program 1 has three arrays with identical content:
    //   cls_buf    — referenced by DEFINECLASSWITHBUFFER (class layout, must not be deduped)
    //   send_buf   — referenced by CALLRUNTIME_DEFINESENDABLECLASS (sendable class layout, same)
    //   arr_canon  — a plain array (eligible for dedup, becomes the canonical id)
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["cls_buf"] = MakeU32LiteralArray(1);
    res1.Value().literalarray_table["send_buf"] = MakeU32LiteralArray(1);
    res1.Value().literalarray_table["arr_canon"] = MakeU32LiteralArray(1);
    AttachClassDefFunction(res1.Value(), "s1.func_main_0", Opcode::DEFINECLASSWITHBUFFER, "s1.C", "cls_buf");
    AttachClassDefFunction(res1.Value(), "s1.func_main_1", Opcode::CALLRUNTIME_DEFINESENDABLECLASS, "s1.SC",
                           "send_buf");
    AttachArrayBuffer(res1.Value(), "s1.func_main_2", "arr_canon");

    // Program 2 has one plain array with the same content — it IS a duplicate of arr_canon.
    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    res2.Value().literalarray_table["arr_dup"] = MakeU32LiteralArray(1);
    AttachArrayBuffer(res2.Value(), "s2.func_main_0", "arr_dup");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // Class layout buffers survive (not merged with plain arrays).
    EXPECT_TRUE(res1.Value().literalarray_table.count("cls_buf") > 0);
    EXPECT_TRUE(res1.Value().literalarray_table.count("send_buf") > 0);
    // The canonical plain array survives.
    EXPECT_TRUE(res1.Value().literalarray_table.count("arr_canon") > 0);
    EXPECT_EQ(res1.Value().literalarray_table.size(), 3U);
    // The duplicate plain array is removed and its instruction/field references would be rewritten.
    EXPECT_FALSE(res2.Value().literalarray_table.count("arr_dup") > 0);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 0U);
    // The class-definition instructions still reference the original (un-rewritten) ids.
    EXPECT_EQ(res1.Value().function_table.at("s1.func_main_0").ins[0]->GetId(1), "cls_buf");
    EXPECT_EQ(res1.Value().function_table.at("s1.func_main_1").ins[0]->GetId(1), "send_buf");
}

/**
 * @tc.name: assembly_emitter_test_036
 * @tc.desc: Verify DeduplicateLiteralArrays does not crash on cyclic LITERALARRAY references
 *           (A→B→A). The cycle is detected and the arrays are not merged.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_036, TestSize.Level1)
{
    auto makeRef = [](const std::string &nestedId) {
        LiteralArray la;
        la.literals_.resize(1);
        la.literals_[0].tag_ = panda_file::LiteralTag::LITERALARRAY;
        la.literals_[0].value_.emplace<std::string>(nestedId);
        return la;
    };

    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    // A → B → A (mutual cycle within one program)
    res1.Value().literalarray_table["a"] = makeRef("b");
    res1.Value().literalarray_table["b"] = makeRef("a");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    // Same cycle shape, different ids — should NOT be merged with the first pair.
    res2.Value().literalarray_table["c"] = makeRef("d");
    res2.Value().literalarray_table["d"] = makeRef("c");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    // Must not crash (cycle detection falls back to using the refId as key).
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // All four cyclic arrays survive — the cycle fallback produces id-unique keys, so no merging.
    EXPECT_EQ(res1.Value().literalarray_table.size(), 2U);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 2U);
}

/**
 * @tc.name: assembly_emitter_test_037
 * @tc.desc: Verify DeduplicateLiteralArrays merges identical arrays referenced by
 *           CREATEOBJECTWITHBUFFER with each other (same instruction-type group).
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_037, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["obj_canon"] = MakeU32LiteralArray(1);
    AttachObjectBuffer(res1.Value(), "s1.func_main_0", "obj_canon");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    res2.Value().literalarray_table["obj_dup"] = MakeU32LiteralArray(1);
    AttachObjectBuffer(res2.Value(), "s2.func_main_0", "obj_dup");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    EXPECT_TRUE(res1.Value().literalarray_table.count("obj_canon") > 0);
    EXPECT_FALSE(res2.Value().literalarray_table.count("obj_dup") > 0);
    EXPECT_EQ(res1.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 0U);
    EXPECT_EQ(res2.Value().function_table.at("s2.func_main_0").ins[0]->GetId(0), "obj_canon");
}

/**
 * @tc.name: assembly_emitter_test_038
 * @tc.desc: Verify DeduplicateLiteralArrays does NOT merge a CREATEARRAYWITHBUFFER array with
 *           a CREATEOBJECTWITHBUFFER array even when content is identical — cross-type merging
 *           causes runtime constant pool type confusion (JSArray vs JSObject).
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblyEmitterTest, assembly_emitter_test_038, TestSize.Level1)
{
    Parser p1;
    auto res1 = p1.Parse(R"(.record R1 { u32 f })", "s1.pa");
    EXPECT_EQ(p1.ShowError().err, Error::ErrorType::ERR_NONE);
    res1.Value().literalarray_table["arr_buf"] = MakeU32LiteralArray(1);
    AttachArrayBuffer(res1.Value(), "s1.func_main_0", "arr_buf");

    Parser p2;
    auto res2 = p2.Parse(R"(.record R2 { u32 f })", "s2.pa");
    EXPECT_EQ(p2.ShowError().err, Error::ErrorType::ERR_NONE);
    res2.Value().literalarray_table["obj_buf"] = MakeU32LiteralArray(1);
    AttachObjectBuffer(res2.Value(), "s2.func_main_0", "obj_buf");

    std::vector<Program *> progs {&res1.Value(), &res2.Value()};
    AsmEmitter::DeduplicateLiteralArrays(progs);

    // Both survive — different instruction-type groups, no cross-merge.
    EXPECT_TRUE(res1.Value().literalarray_table.count("arr_buf") > 0);
    EXPECT_TRUE(res2.Value().literalarray_table.count("obj_buf") > 0);
    EXPECT_EQ(res1.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res2.Value().literalarray_table.size(), 1U);
    EXPECT_EQ(res2.Value().function_table.at("s2.func_main_0").ins[0]->GetId(0), "obj_buf");
}
}  // namespace panda::pandasm
