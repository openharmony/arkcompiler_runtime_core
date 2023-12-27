/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "macros.h"
#include "mem/pool_manager.h"
#include "target/amd64/target.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 20;
#else
const uint64_t ITERATION = 0xffffff;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
static inline auto g_randomGen = std::mt19937_64(SEED);

namespace panda::compiler {
class Register64Test : public ::testing::Test {
public:
    Register64Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);  // NOLINT(readability-magic-numbers)
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }
    ~Register64Test() override
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(Register64Test);
    NO_MOVE_SEMANTIC(Register64Test);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_;
};

TEST_F(Register64Test, TempRegisters)
{
    amd64::Amd64Encoder encoder(GetAllocator());
    ASSERT_TRUE(encoder.InitMasm());

    amd64::Amd64RegisterDescription regfile(GetAllocator());
    encoder.SetRegfile(&regfile);

    auto floatType = FLOAT64_TYPE;

    auto initialCount = encoder.GetScratchRegistersCount();
    auto initialFpCount = encoder.GetScratchFPRegistersCount();
    ASSERT_NE(initialCount, 0);
    ASSERT_NE(initialFpCount, 0);

    std::vector<Reg> regs;
    for (size_t i = 0; i < initialCount; i++) {
        regs.push_back(encoder.AcquireScratchRegister(INT64_TYPE));
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), 0);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);

    regs.clear();
    for (size_t i = 0; i < initialFpCount; i++) {
        regs.push_back(encoder.AcquireScratchRegister(floatType));
    }

    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), 0);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);

    {
        ScopedTmpRegRef reg(&encoder);
        ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 1);
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
        if (encoder.GetScratchRegistersCount() != 0) {
            ScopedTmpRegU32 reg2(&encoder);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 2);
        }
        {
            ScopedTmpReg reg2(&encoder, floatType);
            ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount - 1);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 1);
        }
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
}
}  // namespace panda::compiler
