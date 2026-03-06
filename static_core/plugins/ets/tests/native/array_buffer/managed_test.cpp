/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <ani.h>
#include <cstdint>

#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"
#include "plugins/ets/runtime/libani_helpers/external_array_buffer.h"

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

void CheckUnhandledError(ani_env *env, ani_boolean shouldExist = ANI_FALSE)
{
    ani_boolean exists = ANI_FALSE;
    ASSERT_EQ(env->ExistUnhandledError(&exists), ANI_OK);
    ASSERT_EQ(exists, shouldExist);
}

class ArrayBufferNativeManagedTest : public ani::testing::AniTest {
public:
    /**
     * @brief Checks that changes in natively-acquired array are visible by managed `at` method
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferAtTest(ani_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(length, TWENTY_FOUR);

        data[ELEVEN] = FOURTY_TWO;
        data[TWELVE] = FOURTY_THREE;

        auto value = CallEtsFunction<ani_int>("ManagedTest", "getByte", arrayBuffer, ELEVEN);
        CheckUnhandledError(env_);
        ASSERT_EQ(value, FOURTY_TWO);
        value = CallEtsFunction<ani_int>("ManagedTest", "getByte", arrayBuffer, TWELVE);
        CheckUnhandledError(env_);
        ASSERT_EQ(value, FOURTY_THREE);
    }

    /**
     * @brief Checks that changes through managed `set` method are visible in natively-acquired array
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferSetTest(ani_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(length, TWENTY_FOUR);

        CallEtsFunction<ani_int>("ManagedTest", "setByte", arrayBuffer, ELEVEN, FOURTY_TWO);
        CheckUnhandledError(env_);
        ASSERT_EQ(data[ELEVEN], FOURTY_TWO);
        CallEtsFunction<ani_int>("ManagedTest", "setByte", arrayBuffer, TWELVE, FOURTY_THREE);
        CheckUnhandledError(env_);
        ASSERT_EQ(data[TWELVE], FOURTY_THREE);
    }

    /**
     * @brief Checks that `at` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferAtInvalidIndexTest(ani_arraybuffer arrayBuffer)
    {
        ani_module mod {};
        auto status = env_->FindModule("ManagedTest", &mod);
        ASSERT_EQ(status, ANI_OK);
        ani_function getByte {};
        status = env_->Module_FindFunction(mod, "getByte", nullptr, &getByte);
        ASSERT_EQ(status, ANI_OK);

        ani_int ignored = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(getByte, &ignored, arrayBuffer, -1);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(getByte, &ignored, arrayBuffer, TWENTY_FOUR);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(getByte, &ignored, arrayBuffer, TWENTY_FIVE);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();
    }

    /**
     * @brief Checks that `set` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferSetInvalidIndexTest(ani_arraybuffer arrayBuffer)
    {
        ani_module mod {};
        auto status = env_->FindModule("ManagedTest", &mod);
        ASSERT_EQ(status, ANI_OK);
        ani_function setByte {};
        status = env_->Module_FindFunction(mod, "setByte", nullptr, &setByte);
        ASSERT_EQ(status, ANI_OK);

        ani_int ignored = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(setByte, &ignored, arrayBuffer, -1, 1);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(setByte, &ignored, arrayBuffer, TWENTY_FOUR, 1);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env_->Function_Call_Int(setByte, &ignored, arrayBuffer, TWENTY_FIVE, 1);
        ASSERT_EQ(status, ANI_PENDING_ERROR);
        CheckUnhandledError(env_, ANI_TRUE);
        env_->ResetError();
    }
};

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreate)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", arrayBuffer);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreateWithLength)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", arrayBuffer);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer via ANI and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGet)
{
    ani_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer via ANI and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSet)
{
    ani_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSlice)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(HUNDRED_TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    auto sliced =
        CallEtsFunction<ani_object>("ManagedTest", "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    CheckUnhandledError(env_);
    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", sliced);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

template <size_t LENGTH>
static void CreateExternalArrayBuffer(ani_env *env, ani_arraybuffer *arrayBuffer)
{
    auto *data = new int8_t[LENGTH];
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto status = CreateFinalizableArrayBuffer(env, data, LENGTH, TestFinalizer, nullptr, arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    CheckUnhandledError(env);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreate)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<0>(env_, &arrayBuffer);

    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", arrayBuffer);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreateWithLength)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", arrayBuffer);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer with external memory via ANI and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalGet)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer with external memory via ANI and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSet)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSlice)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<HUNDRED_TWENTY_FOUR>(env_, &arrayBuffer);

    auto sliced =
        CallEtsFunction<ani_object>("ManagedTest", "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    CheckUnhandledError(env_);
    auto length = CallEtsFunction<ani_int>("ManagedTest", "getLength", sliced);
    CheckUnhandledError(env_);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedCreate)
{
    auto arrayBuffer = CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", 0);
    CheckUnhandledError(env_);

    void *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(arrayBuffer), &data, &length);
    ASSERT_EQ(status, ANI_OK);
    // data is accepted to be an arbitrary pointer even on zero length
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedCreateWithLength)
{
    auto arrayBuffer = CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    CheckUnhandledError(env_);

    void *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(arrayBuffer), &data, &length);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, TWENTY_FOUR);
}

/// @brief Creates ArrayBuffer in managed code and tests its `at` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedGet)
{
    auto arrayBuffer = CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    CheckUnhandledError(env_);

    ArrayBufferAtTest(static_cast<ani_arraybuffer>(arrayBuffer));
}

/// @brief Creates ArrayBuffer in managed code and tests its `set` method
TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedSet)
{
    auto arrayBuffer = CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", TWENTY_FOUR);
    CheckUnhandledError(env_);

    ArrayBufferSetTest(static_cast<ani_arraybuffer>(arrayBuffer));
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedDetachEmpty)
{
    ani_arraybuffer arrayBuffer =
        static_cast<ani_arraybuffer>(CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", 0));
    CheckUnhandledError(env_);

    ani_boolean result = ANI_FALSE;
    auto status = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    auto status1 = DetachArrayBuffer(env_, arrayBuffer);
    ASSERT_EQ(status1, ANI_INVALID_ARGS);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedDetach)
{
    ani_arraybuffer arrayBuffer =
        static_cast<ani_arraybuffer>(CallEtsFunction<ani_object>("ManagedTest", "createArrayBuffer", TWENTY_FOUR));
    CheckUnhandledError(env_);

    ani_boolean result = ANI_FALSE;
    auto status = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    auto status1 = DetachArrayBuffer(env_, arrayBuffer);
    ASSERT_EQ(status1, ANI_INVALID_ARGS);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedInvalid)
{
    ani_module mod {};
    auto status = env_->FindModule("ManagedTest", &mod);
    ASSERT_EQ(status, ANI_OK);
    ani_function createArrayBuffer {};
    status = env_->Module_FindFunction(mod, "createArrayBuffer", nullptr, &createArrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ani_ref ignored {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env_->Function_Call_Ref(createArrayBuffer, &ignored, -TWENTY_FOUR);
    ASSERT_EQ(status, ANI_PENDING_ERROR);
    CheckUnhandledError(env_, ANI_TRUE);
    env_->ResetError();
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGetInvalidArgs)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferAtInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalGetInvalidArgs)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferAtInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSetInvalidArgs)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferSetInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSetInvalidArgs)
{
    ani_arraybuffer arrayBuffer = nullptr;
    CreateExternalArrayBuffer<TWENTY_FOUR>(env_, &arrayBuffer);

    ArrayBufferSetInvalidIndexTest(arrayBuffer);
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
