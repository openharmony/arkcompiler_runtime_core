/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "ani_gtest.h"
#include "plugins/ets/runtime/libani_helpers/external_array_buffer.h"

namespace ark::ets::ani_helpers::testing {

class ExternalArrayBufferTest : public ark::ets::ani::testing::AniTest {
public:
    void CheckArrayBuffer(ani_arraybuffer arrayBuffer, uint8_t *externalData, int size)
    {
        uint8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(length, size);
        ASSERT_EQ(data, externalData);
    }
};

void TestFinalizer([[maybe_unused]] void *data, [[maybe_unused]] void *finalizerHint) {}

TEST_F(ExternalArrayBufferTest, CreateExternalArrayBuffer)
{
    static constexpr int SIZE = 3U;
    std::array<uint8_t, SIZE> buffer = {1U, 2U, 3U};

    ani_arraybuffer arrayBuffer;
    auto status = CreateExternalArrayBuffer(env_, buffer.data(), SIZE, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    CheckArrayBuffer(arrayBuffer, buffer.data(), SIZE);

    ani_arraybuffer finalizableArrayBuffer;
    status = CreateFinalizableArrayBuffer(env_, buffer.data(), SIZE, TestFinalizer, nullptr, &finalizableArrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    CheckArrayBuffer(finalizableArrayBuffer, buffer.data(), SIZE);
}

TEST_F(ExternalArrayBufferTest, CreateExternalArrayBufferInvalid)
{
    static constexpr int SIZE = 3U;
    static constexpr int OFFSET = 1U;
    std::array<uint8_t, SIZE + OFFSET> buffer {};
    uint8_t *unalignedPtr = buffer.data() + OFFSET;

    ani_arraybuffer arrayBuffer;
    auto status = CreateExternalArrayBuffer(env_, unalignedPtr, SIZE, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    status = CreateFinalizableArrayBuffer(env_, unalignedPtr, SIZE, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani_helpers::testing
