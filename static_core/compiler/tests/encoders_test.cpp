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

#include <gtest/gtest.h>
#include "macros.h"
#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "target/aarch64/target.h"

#include "vixl_exec_module.h"

namespace panda::compiler::tests {

class EncoderArm64VixlTest : public ::testing::Test {
public:
    EncoderArm64VixlTest()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0U, 0U);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = static_cast<aarch64::Aarch64Encoder *>(Encoder::Create(allocator_, Arch::AARCH64, false));
        encoder_->InitMasm();
        encoder_->SetRegfile(RegistersDescription::Create(allocator_, Arch::AARCH64));

        exec_module_ = allocator_->New<VixlExecModule>(allocator_, nullptr);
    }

    ~EncoderArm64VixlTest() override
    {
        exec_module_->~VixlExecModule();
        Logger::Destroy();
        encoder_->~Aarch64Encoder();
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(EncoderArm64VixlTest);
    NO_MOVE_SEMANTIC(EncoderArm64VixlTest);

    auto GetEncoder()
    {
        return encoder_;
    }

    auto GetAllocator()
    {
        return allocator_;
    }

    void Dump(bool enabled)
    {
        if (enabled) {
            auto size = GetEncoder()->GetCursorOffset();
            for (uint32_t i = 0; i < size;) {
                i = encoder_->DisasmInstr(std::cout, i, 0U);
                std::cout << std::endl;
            }
        }
    }

    template <typename T>
    void TestPcRelativeLoad(size_t data_size, ssize_t offset, bool get_address,
                            std::initializer_list<const char *> insts)
    {
        auto buffer = GetEncoder()->GetMasm()->GetBuffer();
        T *data = reinterpret_cast<T *>(buffer->template GetOffsetAddress<uint8_t *>(data_size) + offset);
        T good_value = GetGoodValue<T>();
        T bad_value = GetBadValue<T>();

        {
            T tmp_value;
            memcpy_s(&tmp_value, sizeof(T), data, sizeof(T));
            ASSERT_EQ(tmp_value, bad_value);
        }

        // We use memcpy here (instead of just assigning), because the data can be aligned by 4 for 64-bit target, and
        // asan will complain about this.
        memcpy_s(data, sizeof(T), &good_value, sizeof(T));

        GetEncoder()->SetCodeOffset(0U);

        buffer->Rewind(data_size);
        Reg reg(0U, TypeInfo(T(0U)));
        if (get_address) {
            Reg addr(1U, INT64_TYPE);
            GetEncoder()->LoadPcRelative(reg, offset, addr);
        } else {
            GetEncoder()->LoadPcRelative(reg, offset);
        }
        GetEncoder()->EncodeReturn();

        auto start_addr = buffer->GetOffsetAddress<const char *>(data_size);
        exec_module_->SetInstructions(start_addr, buffer->GetEndAddress<const char *>());
        exec_module_->Execute();

        {
            T tmp_value;
            memcpy_s(&tmp_value, sizeof(T), data, sizeof(T));
            ASSERT_EQ(tmp_value, good_value);
        }

        {
            auto inst = insts.begin();
            for (uint32_t i = data_size; i < buffer->GetCursorOffset() && inst != insts.end(); ++inst) {
                std::stringstream ss;
                i = encoder_->DisasmInstr(ss, i, 0U);
                auto pos = ss.str().find(*inst);
                EXPECT_NE(pos, std::string::npos) << *inst << " not found";
                if (pos == std::string::npos) {
                    return;
                }
            }
        }

        memcpy_s(data, sizeof(T), &bad_value, sizeof(T));

        if (get_address) {
            EXPECT_EQ(exec_module_->GetSimulator()->ReadXRegister(1U), reinterpret_cast<int64_t>(data));
        }
        EXPECT_EQ(exec_module_->GetRetValue(), GetGoodValue<T>());
    }

    void TestOffset(size_t data_size, ssize_t offset)
    {
        ASSERT((offset & 3U) == 0U);  // NOLINT(hicpp-signed-bitwise)
        if (vixl::IsInt21(offset)) {
            TestPcRelativeLoad<uint64_t>(data_size, offset, false, {"adr", "ldr"});
            TestPcRelativeLoad<uint64_t>(data_size, offset, true, {"adr", "ldr"});
            TestPcRelativeLoad<uint64_t>(data_size, -offset, false, {"adr", "ldr"});
            TestPcRelativeLoad<uint64_t>(data_size, -offset, true, {"adr", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, offset, false, {"adr", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, offset, true, {"adr", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, -offset, false, {"adr", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, -offset, true, {"adr", "ldr"});
        } else {
            if ((offset & 7U) == 0U) {  // NOLINT(hicpp-signed-bitwise)
                TestPcRelativeLoad<uint64_t>(data_size, offset, false, {"adrp", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, offset, true, {"adrp", "add", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, -offset, false, {"adrp", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, -offset, true, {"adrp", "add", "ldr"});
            } else {
                TestPcRelativeLoad<uint64_t>(data_size, offset, false, {"adrp", "mov", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, offset, true, {"adrp", "add", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, -offset, false, {"adrp", "mov", "ldr"});
                TestPcRelativeLoad<uint64_t>(data_size, -offset, true, {"adrp", "add", "ldr"});
            }
            TestPcRelativeLoad<uint32_t>(data_size, offset, false, {"adrp", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, offset, true, {"adrp", "add", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, -offset, false, {"adrp", "ldr"});
            TestPcRelativeLoad<uint32_t>(data_size, -offset, true, {"adrp", "add", "ldr"});
        }
    }

    template <typename T>
    static constexpr T GetBadValue()
    {
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return BAD_VALUE64;
        }
        return BAD_VALUE;
    }

    template <typename T>
    static constexpr T GetGoodValue()
    {
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return GOOD_VALUE64;
        }
        return GOOD_VALUE;
    }

protected:
    static constexpr uint32_t BAD_VALUE = 0xdeadbaad;
    static constexpr uint32_t GOOD_VALUE = 0xcafebabe;
    static constexpr uint64_t BAD_VALUE64 = 0xdeadbaaddeadbaad;
    static constexpr uint64_t GOOD_VALUE64 = 0x1234cafebabe4321;
    static constexpr uint64_t MAX_INT21_VALUE = 0x100000;

private:
    ArenaAllocator *allocator_ {nullptr};
    aarch64::Aarch64Encoder *encoder_ {nullptr};
    VixlExecModule *exec_module_ {nullptr};
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(EncoderArm64VixlTest, LoadPcRelative)
{
    auto encoder = GetEncoder();
    auto masm = encoder->GetMasm();
    auto buffer = masm->GetBuffer();
    static constexpr size_t CODE_OFFSET = 16;
    static constexpr size_t BUF_SIZE = (MAX_INT21_VALUE - vixl::aarch64::kPageSize) * 2U + CODE_OFFSET;

    for (size_t i = 0; i < BUF_SIZE / sizeof(uint32_t); i++) {
        buffer->Emit32(GetBadValue<uint32_t>());
    }
    // Pre-allocate space for the code
    for (size_t i = 0; i < 4U; i++) {
        buffer->Emit32(0U);
    }
    for (size_t i = 0; i < BUF_SIZE / sizeof(uint32_t); i++) {
        buffer->Emit32(GetBadValue<uint32_t>());
    }

    TestOffset(BUF_SIZE, 40U);
    TestOffset(BUF_SIZE, 44U);
    TestOffset(BUF_SIZE, 0x1374b8U);
    // Check for two pages addrp
    TestOffset(BUF_SIZE, MAX_INT21_VALUE + vixl::aarch64::kPageSize + CODE_OFFSET + 8U);
    // Check for one page addrp
    TestOffset(BUF_SIZE, MAX_INT21_VALUE + vixl::aarch64::kPageSize + CODE_OFFSET - 8L);
    TestOffset(BUF_SIZE, 0x100404U);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler::tests
