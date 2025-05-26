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

#include <cstdint>
#include <limits>
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/tests/native/native_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

namespace ark::ets::test {

static constexpr int TWO = 2;
static constexpr int THREE = 3;
static constexpr int EIGHT = 8;
static constexpr int TWENTY_FOUR = 24;
static constexpr int HUNDRED_TWENTY_THREE = 123;
static constexpr int TWO_HUNDRED_THIRTY_FOUR = 234;
static constexpr int THREE_HUNDRED_FOURTY_FIVE = 345;
static constexpr const char *CHECK_ARRAY_BUFFER_FUNCTION = "checkCreatedArrayBuffer";
static constexpr const char *IS_DETACHED = "isDetached";

static void TestFinalizer(void *data, [[maybe_unused]] void *finalizerHint)
{
    delete[] reinterpret_cast<int64_t *>(data);
}

class EtsNativeInterfaceArrayBufferTest : public EtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateEmpty)
{
    ets_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    void *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBufferGetInfo(arrayBuffer, &resultData, &resultLength);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, 0);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateWithLength)
{
    ets_arraybuffer arrayBuffer;
    int64_t *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    data[0] = HUNDRED_TWENTY_THREE;
    data[1] = TWO_HUNDRED_THIRTY_FOUR;
    data[TWO] = THREE_HUNDRED_FOURTY_FIVE;

    int64_t *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&resultData), &resultLength);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, TWENTY_FOUR);
    ASSERT_EQ(resultData[0], HUNDRED_TWENTY_THREE);
    ASSERT_EQ(resultData[1], TWO_HUNDRED_THIRTY_FOUR);
    ASSERT_EQ(resultData[TWO], THREE_HUNDRED_FOURTY_FIVE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateExternalEmpty)
{
    ets_arraybuffer arrayBuffer;
    void *data = new int64_t[0];
    auto status = env_->ArrayBufferCreateExternal(data, 0, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    void *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBufferGetInfo(arrayBuffer, &resultData, &resultLength);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, 0);

    ets_boolean bufferIsExpected = ETS_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, 0);
    ASSERT_EQ(bufferIsExpected, ETS_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateExternalWithLength)
{
    ets_arraybuffer arrayBuffer;
    auto *data = new int64_t[THREE];
    data[0] = HUNDRED_TWENTY_THREE;
    data[1] = TWO_HUNDRED_THIRTY_FOUR;
    data[TWO] = THREE_HUNDRED_FOURTY_FIVE;

    auto status = env_->ArrayBufferCreateExternal(data, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    int64_t *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&resultData), &resultLength);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, TWENTY_FOUR);
    ASSERT_EQ(resultData[0], HUNDRED_TWENTY_THREE);
    ASSERT_EQ(resultData[1], TWO_HUNDRED_THIRTY_FOUR);
    ASSERT_EQ(resultData[TWO], THREE_HUNDRED_FOURTY_FIVE);

    ets_boolean bufferIsExpected = ETS_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, TWENTY_FOUR);
    ASSERT_EQ(bufferIsExpected, ETS_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferDetachNonDetachable)
{
    ets_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    bool result = false;
    auto resultStatus = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(result, false);

    auto resultstatus1 = env_->ArrayBufferDetach(arrayBuffer);
    ASSERT_EQ(resultstatus1, ETS_DETACHABLE_ARRAYBUFFER_EXPECTED);

    auto resultstatus2 = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(resultstatus2, ETS_OKAY);
    ASSERT_EQ(result, false);

    ets_boolean isDetached = ETS_FALSE;
    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ETS_FALSE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferDetachDetachable)
{
    ets_arraybuffer arrayBuffer;
    auto *data = new int64_t[THREE];

    auto status = env_->ArrayBufferCreateExternal(data, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());

    ets_boolean bufferIsExpected = ETS_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, TWENTY_FOUR);
    ASSERT_EQ(bufferIsExpected, ETS_TRUE);

    bool result = false;
    auto resultStatus = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(resultStatus, ETS_OKAY);
    ASSERT_EQ(result, false);

    auto resultstatus1 = env_->ArrayBufferDetach(arrayBuffer);
    ASSERT_EQ(resultstatus1, ETS_OKAY);

    auto resultstatus2 = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(resultstatus2, ETS_OKAY);
    ASSERT_EQ(result, true);

    ets_boolean isDetached = ETS_FALSE;
    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ETS_TRUE);

    auto resultstatus3 = env_->ArrayBufferDetach(arrayBuffer);
    ASSERT_EQ(resultstatus3, ETS_DETACHABLE_ARRAYBUFFER_EXPECTED);

    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ETS_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferInvalidArgs)
{
    ets_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(-1, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer1;
    void *data1 = nullptr;
    size_t oversize = static_cast<size_t>(std::numeric_limits<int>::max()) + 1U;
    auto status1 = env_->ArrayBufferCreate(oversize, &data1, &arrayBuffer1);
    ASSERT_EQ(status1, ETS_INVALID_ARG);

    void *data2 = nullptr;
    auto status2 = env_->ArrayBufferCreate(TWENTY_FOUR, &data2, nullptr);
    ASSERT_EQ(status2, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer3;
    auto status3 = env_->ArrayBufferCreate(TWENTY_FOUR, nullptr, &arrayBuffer3);
    ASSERT_EQ(status3, ETS_INVALID_ARG);

    auto *externalData = new int64_t[THREE];

    auto status4 = env_->ArrayBufferCreateExternal(externalData, TWENTY_FOUR, TestFinalizer, nullptr, nullptr);
    ASSERT_EQ(status4, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer5;
    auto status5 = env_->ArrayBufferCreateExternal(externalData, -1, TestFinalizer, nullptr, &arrayBuffer5);
    ASSERT_EQ(status5, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer6;
    auto status6 = env_->ArrayBufferCreateExternal(nullptr, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer6);
    ASSERT_EQ(status6, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer7;
    auto status7 = env_->ArrayBufferCreateExternal(externalData, TWENTY_FOUR, nullptr, nullptr, &arrayBuffer7);
    ASSERT_EQ(status7, ETS_INVALID_ARG);

    auto status8 = env_->ArrayBufferCreateExternal(nullptr, 0, nullptr, nullptr, nullptr);
    ASSERT_EQ(status8, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer9;
    void *data9;
    ASSERT_EQ(env_->ArrayBufferCreate(EIGHT, &data9, &arrayBuffer9), ETS_OKAY);
    size_t length9;
    auto status9 = env_->ArrayBufferGetInfo(arrayBuffer9, nullptr, &length9);
    ASSERT_EQ(status9, ETS_INVALID_ARG);

    // No array buffer was created with this memory, so delete it manually to avoid mem leak
    delete[] externalData;

    ASSERT_FALSE(env_->ErrorOccurred());
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferInvalidArgs2)
{
    ets_arraybuffer arrayBuffer10;
    void *data10;
    ASSERT_EQ(env_->ArrayBufferCreate(EIGHT, &data10, &arrayBuffer10), ETS_OKAY);
    void *resultData10;
    auto status10 = env_->ArrayBufferGetInfo(arrayBuffer10, &resultData10, nullptr);
    ASSERT_EQ(status10, ETS_INVALID_ARG);

    size_t length11;
    void *resultData11;
    auto status11 = env_->ArrayBufferGetInfo(nullptr, &resultData11, &length11);
    ASSERT_EQ(status11, ETS_INVALID_ARG);

    bool result12;
    auto status12 = env_->ArrayBufferIsDetached(nullptr, &result12);
    ASSERT_EQ(status12, ETS_INVALID_ARG);

    ets_arraybuffer arrayBuffer13;
    void *data13;
    ASSERT_EQ(env_->ArrayBufferCreate(EIGHT, &data13, &arrayBuffer13), ETS_OKAY);
    auto status13 = env_->ArrayBufferIsDetached(arrayBuffer13, nullptr);
    ASSERT_EQ(status13, ETS_INVALID_ARG);

    auto status14 = env_->ArrayBufferDetach(nullptr);
    ASSERT_EQ(status14, ETS_INVALID_ARG);

    ASSERT_FALSE(env_->ErrorOccurred());
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
