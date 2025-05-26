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

#include <gtest/gtest.h>
#include <cstdint>
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/tests/native/native_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
namespace ark::ets::test {

constexpr int ELEVEN = 11;
constexpr int TWELVE = 12;
constexpr int TWENTY_FOUR = 24;
constexpr int TWENTY_FIVE = 25;
constexpr int FOURTY_TWO = 42;
constexpr int FOURTY_THREE = 43;
constexpr int HUNDRED = 100;
constexpr int HUNDRED_TWENTY_FOUR = 124;

void TestFinalizer(void *data, [[maybe_unused]] void *finalizerHint)
{
    delete[] reinterpret_cast<int8_t *>(data);
}

class ArrayBufferNativeManagedTest : public EtsNapiTestBaseClass {
public:
    /**
     * @brief Checks that changes in natively-acquired array are visible by managed `at` method
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferAtTest(ets_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ETS_OKAY);
        ASSERT_EQ(length, TWENTY_FOUR);

        data[ELEVEN] = FOURTY_TWO;
        data[TWELVE] = FOURTY_THREE;

        ets_int value = 0;
        CallEtsFunction(&value, "ManagedTest", "getByte", arrayBuffer, ELEVEN);
        ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
        ASSERT_EQ(value, FOURTY_TWO);
        CallEtsFunction(&value, "ManagedTest", "getByte", arrayBuffer, TWELVE);
        ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
        ASSERT_EQ(value, FOURTY_THREE);
    }

    /**
     * @brief Checks that changes through managed `set` method are visible in natively-acquired array
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferSetTest(ets_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ETS_OKAY);
        ASSERT_EQ(length, TWENTY_FOUR);

        ets_int value = 0;
        CallEtsFunction(&value, "ManagedTest", "setByte", arrayBuffer, ELEVEN, FOURTY_TWO);
        ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
        ASSERT_EQ(data[ELEVEN], FOURTY_TWO);
        CallEtsFunction(&value, "ManagedTest", "setByte", arrayBuffer, TWELVE, FOURTY_THREE);
        ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
        ASSERT_EQ(data[TWELVE], FOURTY_THREE);
    }

    /**
     * @brief Checks that `at` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferAtInvalidIndexTest(ets_arraybuffer arrayBuffer)
    {
        ets_int value = 0;
        CallEtsFunction(&value, "ManagedTest", "getByte", arrayBuffer, -1);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();

        CallEtsFunction(&value, "ManagedTest", "getByte", arrayBuffer, TWENTY_FOUR);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();

        CallEtsFunction(&value, "ManagedTest", "getByte", arrayBuffer, TWENTY_FIVE);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();
    }

    /**
     * @brief Checks that `set` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferSetInvalidIndexTest(ets_arraybuffer arrayBuffer)
    {
        ets_int value = 0;
        CallEtsFunction(&value, "ManagedTest", "setByte", arrayBuffer, -1, 1);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();

        CallEtsFunction(&value, "ManagedTest", "setByte", arrayBuffer, TWENTY_FOUR, 1);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();

        CallEtsFunction(&value, "ManagedTest", "setByte", arrayBuffer, TWENTY_FIVE, 1);
        ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
        env_->ErrorClear();
    }
};

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", arrayBuffer);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreateWithLength)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", arrayBuffer);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer via ets_napi and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer via ets_napi and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSlice)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(HUNDRED_TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_arraybuffer sliced {};
    CallEtsFunction(&sliced, "ManagedTest", "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", sliced);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

template <size_t LENGTH>
static void CreateExternalArrayBuffer(EtsEnv *env, ets_arraybuffer *arrayBuffer)
{
    auto *data = new int8_t[LENGTH];
    auto status = env->ArrayBufferCreateExternal(data, LENGTH, TestFinalizer, nullptr, arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_EQ(env->ErrorCheck(), ETS_FALSE);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<0>(env_, &arrayBuffer);

    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", arrayBuffer);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreateWithLength)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", arrayBuffer);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer with external memory via ets_napi and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer with external memory via ets_napi and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSlice)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<HUNDRED_TWENTY_FOUR>(env_, &arrayBuffer);

    ets_arraybuffer sliced {};
    CallEtsFunction(&sliced, "ManagedTest", "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ets_int length = 0;
    CallEtsFunction(&length, "ManagedTest", "getLength", sliced);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", 0);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    void *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBufferGetInfo(arrayBuffer, &data, &length);
    ASSERT_EQ(status, ETS_OKAY);
    // data is accepted to be an arbitrary pointer even on zero length
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedCreateWithLength)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    void *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBufferGetInfo(arrayBuffer, &data, &length);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer in managed code and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer in managed code and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedDetachEmpty)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", 0);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    bool result = false;
    auto status = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_EQ(status, false);

    auto status1 = env_->ArrayBufferDetach(arrayBuffer);
    ASSERT_EQ(status1, ETS_DETACHABLE_ARRAYBUFFER_EXPECTED);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedDetach)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    ASSERT_EQ(env_->ErrorCheck(), ETS_FALSE);

    bool result = false;
    auto status = env_->ArrayBufferIsDetached(arrayBuffer, &result);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_EQ(status, false);

    auto status1 = env_->ArrayBufferDetach(arrayBuffer);
    ASSERT_EQ(status1, ETS_DETACHABLE_ARRAYBUFFER_EXPECTED);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedInvalid)
{
    ets_arraybuffer arrayBuffer = nullptr;

    CallEtsFunction(&arrayBuffer, "ManagedTest", "createArrayBuffer", -TWENTY_FOUR);
    ASSERT_EQ(env_->ErrorCheck(), ETS_TRUE);
    env_->ErrorClear();
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ArrayBufferAtInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalGetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferAtInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ArrayBufferSetInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferSetInvalidIndexTest(arrayBuffer);
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
