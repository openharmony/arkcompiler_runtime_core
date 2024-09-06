/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "target/aarch64/target.h"
#include "mem/base_mem_stats.h"

namespace ark::compiler {
class Callconv64Test : public ::testing::Test {
public:
    Callconv64Test()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::AARCH64, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::AARCH64);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::AARCH64);
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
    }
    ~Callconv64Test() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete codeAlloc_;
        delete memStats_;
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(Callconv64Test);
    NO_MOVE_SEMANTIC(Callconv64Test);

    Encoder *GetEncoder()
    {
        return encoder_;
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    RegistersDescription *GetRegfile()
    {
        return regfile_;
    }

    CallingConvention *GetCallconv()
    {
        return callconv_;
    }

private:
    Encoder *encoder_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
};

TEST_F(Callconv64Test, NativeIntParams)
{
    // Test for
    // std::variant<Reg, uint8_t> GetNativeParam(const ArenaVector<TypeInfo>& reg_list,
    //                                           const TypeInfo& type)

    // 8 uint8_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT8_TYPE));

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(INT8_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT8_TYPE));
        }
    }

    // 8 uint32_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT32_TYPE));

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(INT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT32_TYPE));
        }
    }

    // 8 uint64_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT64_TYPE));

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(INT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT64_TYPE));
        }
    }
}

TEST_F(Callconv64Test, NativeFloatParams)
{
    // 8 float params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT32_TYPE));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT32_TYPE));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
        }
    }

    // 8 double params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT64_TYPE));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT64_TYPE));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
        }
    }
}
}  // namespace ark::compiler
