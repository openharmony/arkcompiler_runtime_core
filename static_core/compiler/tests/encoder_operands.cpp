/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <random>
#include <gtest/gtest.h>

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 20;
#else
const uint64_t ITERATION = 4000;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
static inline auto RANDOM_GEN = std::mt19937_64(SEED);

// Encoder header
#include "optimizer/code_generator/operands.h"

// NOLINTBEGIN(readability-isolate-declaration)
namespace panda::compiler {
TEST(Operands, TypeInfo)
{
    uint8_t u8 = 0;
    int8_t i8 = 0;
    uint16_t u16 = 0;
    int16_t i16 = 0;
    uint32_t u32 = 0;
    int32_t i32 = 0;
    uint64_t u64 = 0;
    int64_t i64 = 0;

    float f32 = 0.0;
    double f64 = 0.0;

    std::array arr {
        TypeInfo(u8),  // 0
        TypeInfo(u16),
        TypeInfo(u32),
        TypeInfo(u64),
        TypeInfo(i8),  // 4
        TypeInfo(i16),
        TypeInfo(i32),
        TypeInfo(i64),
        TypeInfo(INT8_TYPE),  // 8
        TypeInfo(INT16_TYPE),
        TypeInfo(INT32_TYPE),
        TypeInfo(INT64_TYPE),
        TypeInfo(f32),  // 12
        TypeInfo(f64),
        TypeInfo(FLOAT32_TYPE),  // 14
        TypeInfo(FLOAT64_TYPE),
        TypeInfo(),  // 16
        INVALID_TYPE,
    };

    for (uint64_t i = 0; i < sizeof(arr) / sizeof(TypeInfo); ++i) {
        if (i >= 16U) {  // NOLINT(readability-magic-numbers)
            ASSERT_FALSE(arr[i].IsValid());
        } else {
            ASSERT_TRUE(arr[i].IsValid());
        }
    }

    for (size_t i = 0; i < 4U; ++i) {
        ASSERT_EQ(arr[i], arr[4U + i]);
        ASSERT_EQ(arr[i], arr[8U + i]);
        ASSERT_EQ(arr[4U + i], arr[8U + i]);

        ASSERT_EQ(arr[i].GetSize(), arr[4U + i].GetSize());
        ASSERT_EQ(arr[i].GetSize(), arr[8U + i].GetSize());
        ASSERT_EQ(arr[4U + i].GetSize(), arr[8U + i].GetSize());

        ASSERT_TRUE(arr[i].IsScalar());
        ASSERT_TRUE(arr[4U + i].IsScalar());
        ASSERT_TRUE(arr[8U + i].IsScalar());

        ASSERT_FALSE(arr[i].IsFloat());
        ASSERT_FALSE(arr[4U + i].IsFloat());
        ASSERT_FALSE(arr[8U + i].IsFloat());

        ASSERT_NE(arr[i], arr[12U]);
        ASSERT_NE(arr[i], arr[13U]);
        ASSERT_NE(arr[4U + i], arr[12U]);
        ASSERT_NE(arr[4U + i], arr[13U]);
        ASSERT_NE(arr[8U + i], arr[12U]);
        ASSERT_NE(arr[8U + i], arr[13U]);

        ASSERT_NE(arr[i], arr[14U]);
        ASSERT_NE(arr[i], arr[15U]);
        ASSERT_NE(arr[4U + i], arr[14U]);
        ASSERT_NE(arr[4U + i], arr[15U]);
        ASSERT_NE(arr[8U + i], arr[14U]);
        ASSERT_NE(arr[8U + i], arr[15U]);
        ASSERT_NE(arr[i], arr[16U]);
        ASSERT_NE(arr[i], arr[17U]);
        ASSERT_NE(arr[4U + i], arr[16U]);
        ASSERT_NE(arr[4U + i], arr[17U]);
        ASSERT_NE(arr[8U + i], arr[16U]);
        ASSERT_NE(arr[8U + i], arr[17U]);
    }
    // Float
    ASSERT_EQ(arr[2U].GetSize(), arr[12U].GetSize());
    ASSERT_EQ(arr[2U].GetSize(), arr[14U].GetSize());

    ASSERT_TRUE(arr[12U].IsValid());
    ASSERT_TRUE(arr[14U].IsValid());
    ASSERT_TRUE(arr[12U].IsFloat());
    ASSERT_TRUE(arr[14U].IsFloat());
    // Double
    ASSERT_EQ(arr[3U].GetSize(), arr[13U].GetSize());
    ASSERT_EQ(arr[3U].GetSize(), arr[15U].GetSize());

    // Check sizes:
    ASSERT_EQ(BYTE_SIZE, HALF_SIZE / 2U);
    ASSERT_EQ(HALF_SIZE, WORD_SIZE / 2U);
    ASSERT_EQ(WORD_SIZE, DOUBLE_WORD_SIZE / 2U);

    ASSERT_EQ(arr[0U].GetSize(), BYTE_SIZE);
    ASSERT_EQ(arr[1U].GetSize(), HALF_SIZE);
    ASSERT_EQ(arr[2U].GetSize(), WORD_SIZE);
    ASSERT_EQ(arr[3U].GetSize(), DOUBLE_WORD_SIZE);

    ASSERT_EQ(sizeof(TypeInfo), sizeof(uint8_t));

    ASSERT_EQ(TypeInfo(u8), INT8_TYPE);
    ASSERT_EQ(TypeInfo(u16), INT16_TYPE);
    ASSERT_EQ(TypeInfo(u32), INT32_TYPE);
    ASSERT_EQ(TypeInfo(u64), INT64_TYPE);

    ASSERT_EQ(TypeInfo(f32), FLOAT32_TYPE);
    ASSERT_EQ(TypeInfo(f64), FLOAT64_TYPE);
}

TEST(Operands, Reg)
{
    //  Size of structure
    ASSERT_LE(sizeof(Reg), sizeof(size_t));

    ASSERT_EQ(INVALID_REGISTER.GetId(), INVALID_REG_ID);

    // Check, what it is possible to create all 32 registers
    // for each type

    // Check what special registers are possible to compare with others

    // Check equality between registers

    // Check invalid registers
}

TEST(Operands, Imm)
{
    // Check all possible types:
    //  Imm holds same data (static cast for un-signed)

    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint8_t u8 = RANDOM_GEN(), u8_z = 0U, u8_min = std::numeric_limits<uint8_t>::min(),
                u8_max = std::numeric_limits<uint8_t>::max();
        uint16_t u16 = RANDOM_GEN(), u16_z = 0U, u16_min = std::numeric_limits<uint16_t>::min(),
                 u16_max = std::numeric_limits<uint16_t>::max();
        uint32_t u32 = RANDOM_GEN(), u32_z = 0U, u32_min = std::numeric_limits<uint32_t>::min(),
                 u32_max = std::numeric_limits<uint32_t>::max();
        uint64_t u64 = RANDOM_GEN(), u64_z = 0U, u64_min = std::numeric_limits<uint64_t>::min(),
                 u64_max = std::numeric_limits<uint64_t>::max();

        int8_t i8 = RANDOM_GEN(), i8_z = 0U, i8_min = std::numeric_limits<int8_t>::min(),
               i8_max = std::numeric_limits<int8_t>::max();
        int16_t i16 = RANDOM_GEN(), i16_z = 0U, i16_min = std::numeric_limits<int16_t>::min(),
                i16_max = std::numeric_limits<int16_t>::max();
        int32_t i32 = RANDOM_GEN(), i32_z = 0U, i32_min = std::numeric_limits<int32_t>::min(),
                i32_max = std::numeric_limits<int32_t>::max();
        int64_t i64 = RANDOM_GEN(), i64_z = 0U, i64_min = std::numeric_limits<int64_t>::min(),
                i64_max = std::numeric_limits<int64_t>::max();

        float f32 = RANDOM_GEN(), f32_z = 0.0, f32_min = std::numeric_limits<float>::min(),
              f32_max = std::numeric_limits<float>::max();
        double f64 = RANDOM_GEN(), f64_z = 0.0, f64_min = std::numeric_limits<double>::min(),
               f64_max = std::numeric_limits<double>::max();

        // Unsigned part - check across static_cast

        Imm imm_u8(u8), imm_u8_z(u8_z), imm_u8_min(u8_min), imm_u8_max(u8_max);
        ASSERT_EQ(imm_u8.GetAsInt(), u8);
        ASSERT_EQ(imm_u8_min.GetAsInt(), u8_min);
        ASSERT_EQ(imm_u8_max.GetAsInt(), u8_max);
        ASSERT_EQ(imm_u8_z.GetAsInt(), u8_z);

        TypedImm typed_imm_u8(u8), typed_imm_u8_z(u8_z);
        ASSERT_EQ(typed_imm_u8_z.GetType(), INT8_TYPE);
        ASSERT_EQ(typed_imm_u8.GetType(), INT8_TYPE);
        ASSERT_EQ(typed_imm_u8_z.GetImm().GetAsInt(), u8_z);
        ASSERT_EQ(typed_imm_u8.GetImm().GetAsInt(), u8);

        Imm imm_u16(u16), imm_u16_z(u16_z), imm_u16_min(u16_min), imm_u16_max(u16_max);
        ASSERT_EQ(imm_u16.GetAsInt(), u16);
        ASSERT_EQ(imm_u16_min.GetAsInt(), u16_min);
        ASSERT_EQ(imm_u16_max.GetAsInt(), u16_max);
        ASSERT_EQ(imm_u16_z.GetAsInt(), u16_z);

        TypedImm typed_imm_u16(u16), typed_imm_u16_z(u16_z);
        ASSERT_EQ(typed_imm_u16_z.GetType(), INT16_TYPE);
        ASSERT_EQ(typed_imm_u16.GetType(), INT16_TYPE);
        ASSERT_EQ(typed_imm_u16_z.GetImm().GetAsInt(), u16_z);
        ASSERT_EQ(typed_imm_u16.GetImm().GetAsInt(), u16);

        Imm imm_u32(u32), imm_u32_z(u32_z), imm_u32_min(u32_min), imm_u32_max(u32_max);
        ASSERT_EQ(imm_u32.GetAsInt(), u32);
        ASSERT_EQ(imm_u32_min.GetAsInt(), u32_min);
        ASSERT_EQ(imm_u32_max.GetAsInt(), u32_max);
        ASSERT_EQ(imm_u32_z.GetAsInt(), u32_z);

        TypedImm typed_imm_u32(u32), typed_imm_u32_z(u32_z);
        ASSERT_EQ(typed_imm_u32_z.GetType(), INT32_TYPE);
        ASSERT_EQ(typed_imm_u32.GetType(), INT32_TYPE);
        ASSERT_EQ(typed_imm_u32_z.GetImm().GetAsInt(), u32_z);
        ASSERT_EQ(typed_imm_u32.GetImm().GetAsInt(), u32);

        Imm imm_u64(u64), imm_u64_z(u64_z), imm_u64_min(u64_min), imm_u64_max(u64_max);
        ASSERT_EQ(imm_u64.GetAsInt(), u64);
        ASSERT_EQ(imm_u64_min.GetAsInt(), u64_min);
        ASSERT_EQ(imm_u64_max.GetAsInt(), u64_max);
        ASSERT_EQ(imm_u64_z.GetAsInt(), u64_z);

        TypedImm typed_imm_u64(u64), typed_imm_u64_z(u64_z);
        ASSERT_EQ(typed_imm_u64_z.GetType(), INT64_TYPE);
        ASSERT_EQ(typed_imm_u64.GetType(), INT64_TYPE);
        ASSERT_EQ(typed_imm_u64_z.GetImm().GetAsInt(), u64_z);
        ASSERT_EQ(typed_imm_u64.GetImm().GetAsInt(), u64);

        // Signed part

        Imm imm_i8(i8), imm_i8_z(i8_z), imm_i8_min(i8_min), imm_i8_max(i8_max);
        ASSERT_EQ(imm_i8.GetAsInt(), i8);
        ASSERT_EQ(imm_i8_min.GetAsInt(), i8_min);
        ASSERT_EQ(imm_i8_max.GetAsInt(), i8_max);
        ASSERT_EQ(imm_i8_z.GetAsInt(), i8_z);

        TypedImm typed_imm_i8(i8), typed_imm_i8_z(i8_z);
        ASSERT_EQ(typed_imm_i8_z.GetType(), INT8_TYPE);
        ASSERT_EQ(typed_imm_i8.GetType(), INT8_TYPE);
        ASSERT_EQ(typed_imm_i8_z.GetImm().GetAsInt(), i8_z);
        ASSERT_EQ(typed_imm_i8.GetImm().GetAsInt(), i8);

        Imm imm_i16(i16), imm_i16_z(i16_z), imm_i16_min(i16_min), imm_i16_max(i16_max);
        ASSERT_EQ(imm_i16.GetAsInt(), i16);
        ASSERT_EQ(imm_i16_min.GetAsInt(), i16_min);
        ASSERT_EQ(imm_i16_max.GetAsInt(), i16_max);
        ASSERT_EQ(imm_i16_z.GetAsInt(), i16_z);

        TypedImm typed_imm_i16(i16), typed_imm_i16_z(i16_z);
        ASSERT_EQ(typed_imm_i16_z.GetType(), INT16_TYPE);
        ASSERT_EQ(typed_imm_i16.GetType(), INT16_TYPE);
        ASSERT_EQ(typed_imm_i16_z.GetImm().GetAsInt(), i16_z);
        ASSERT_EQ(typed_imm_i16.GetImm().GetAsInt(), i16);

        Imm imm_i32(i32), imm_i32_z(i32_z), imm_i32_min(i32_min), imm_i32_max(i32_max);
        ASSERT_EQ(imm_i32.GetAsInt(), i32);
        ASSERT_EQ(imm_i32_min.GetAsInt(), i32_min);
        ASSERT_EQ(imm_i32_max.GetAsInt(), i32_max);
        ASSERT_EQ(imm_i32_z.GetAsInt(), i32_z);

        TypedImm typed_imm_i32(i32), typed_imm_i32_z(i32_z);
        ASSERT_EQ(typed_imm_i32_z.GetType(), INT32_TYPE);
        ASSERT_EQ(typed_imm_i32.GetType(), INT32_TYPE);
        ASSERT_EQ(typed_imm_i32_z.GetImm().GetAsInt(), i32_z);
        ASSERT_EQ(typed_imm_i32.GetImm().GetAsInt(), i32);

        Imm imm_i64(i64), imm_i64_z(i64_z), imm_i64_min(i64_min), imm_i64_max(i64_max);
        ASSERT_EQ(imm_i64.GetAsInt(), i64);
        ASSERT_EQ(imm_i64_min.GetAsInt(), i64_min);
        ASSERT_EQ(imm_i64_max.GetAsInt(), i64_max);
        ASSERT_EQ(imm_i64_z.GetAsInt(), i64_z);

        TypedImm typed_imm_i64(i64), typed_imm_i64_z(i64_z);
        ASSERT_EQ(typed_imm_i64_z.GetType(), INT64_TYPE);
        ASSERT_EQ(typed_imm_i64.GetType(), INT64_TYPE);
        ASSERT_EQ(typed_imm_i64_z.GetImm().GetAsInt(), i64_z);
        ASSERT_EQ(typed_imm_i64.GetImm().GetAsInt(), i64);

        // Float test:
        Imm imm_f32(f32), imm_f32_z(f32_z), imm_f32_min(f32_min), imm_f32_max(f32_max);
        ASSERT_EQ(imm_f32.GetAsFloat(), f32);
        ASSERT_EQ(imm_f32_min.GetAsFloat(), f32_min);
        ASSERT_EQ(imm_f32_max.GetAsFloat(), f32_max);
        ASSERT_EQ(imm_f32_z.GetAsFloat(), f32_z);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(imm_f32.GetRawValue())), f32);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(imm_f32_min.GetRawValue())), f32_min);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(imm_f32_max.GetRawValue())), f32_max);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(imm_f32_z.GetRawValue())), f32_z);

        TypedImm typed_imm_f32(f32), typed_imm_f32_z(f32_z);
        ASSERT_EQ(typed_imm_f32_z.GetType(), FLOAT32_TYPE);
        ASSERT_EQ(typed_imm_f32.GetType(), FLOAT32_TYPE);
        ASSERT_EQ(typed_imm_f32_z.GetImm().GetAsFloat(), f32_z);
        ASSERT_EQ(typed_imm_f32.GetImm().GetAsFloat(), f32);

        Imm imm_f64(f64), imm_f64_z(f64_z), imm_f64_min(f64_min), imm_f64_max(f64_max);
        ASSERT_EQ(imm_f64.GetAsDouble(), f64);
        ASSERT_EQ(imm_f64_min.GetAsDouble(), f64_min);
        ASSERT_EQ(imm_f64_max.GetAsDouble(), f64_max);
        ASSERT_EQ(imm_f64_z.GetAsDouble(), f64_z);
        ASSERT_EQ(bit_cast<double>(imm_f64.GetRawValue()), f64);
        ASSERT_EQ(bit_cast<double>(imm_f64_min.GetRawValue()), f64_min);
        ASSERT_EQ(bit_cast<double>(imm_f64_max.GetRawValue()), f64_max);
        ASSERT_EQ(bit_cast<double>(imm_f64_z.GetRawValue()), f64_z);

        TypedImm typed_imm_f64(f64), typed_imm_f64_z(f64_z);
        ASSERT_EQ(typed_imm_f64_z.GetType(), FLOAT64_TYPE);
        ASSERT_EQ(typed_imm_f64.GetType(), FLOAT64_TYPE);
        ASSERT_EQ(typed_imm_f64_z.GetImm().GetAsDouble(), f64_z);
        ASSERT_EQ(typed_imm_f64.GetImm().GetAsDouble(), f64);
    }

#ifndef NDEBUG
    // Imm holds std::variant:
    ASSERT_EQ(sizeof(Imm), sizeof(uint64_t) * 2U);
#else
    // Imm holds 64-bit storage only:
    ASSERT_EQ(sizeof(Imm), sizeof(uint64_t));
#endif  // NDEBUG
}

TEST(Operands, MemRef)
{
    Reg r1(1U, INT64_TYPE), r2(2U, INT64_TYPE), r_i(INVALID_REG_ID, INVALID_TYPE);
    ssize_t i1(0x0U), i2(0x2U);

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
    std::array<MemRef, 3U> arr {MemRef(r1), MemRef(r1, i1), MemRef(r1)};
    // 1. Check constructors
    //  for getters
    //  for validness
    //  for operator ==
    // 2. Create mem with invalid_reg / invalid imm
}
// NOLINTEND(readability-isolate-declaration)

}  // namespace panda::compiler
