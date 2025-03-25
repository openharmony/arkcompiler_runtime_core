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
#include <string>
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/tests/native/native_test_helper.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
namespace ark::ets::test {

constexpr int ELEVEN = 11;
constexpr int TWELVE = 12;
constexpr int TWENTY_FOUR = 24;
constexpr int TWENTY_FIVE = 25;
constexpr int FOURTY_TWO = 42;
constexpr int FOURTY_THREE = 43;
constexpr int HUNDRED = 100;
constexpr int HUNDRED_ELEVEN = 111;
constexpr int HUNDRED_TWENTY_FOUR = 124;

void TestFinalizer(void *data, [[maybe_unused]] void *finalizerHint)
{
    delete[] reinterpret_cast<int8_t *>(data);
}

class ArrayBufferNativeManagedTest : public EtsNapiTestBaseClass {};

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", arrayBuffer);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferCreateWithLength)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", arrayBuffer);
    ASSERT_EQ(length, TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    data[ELEVEN] = HUNDRED_TWENTY_FOUR;
    data[TWELVE] = FOURTY_TWO;

    ets_int value = 0;
    CallEtsFuntion(&value, "getByte", arrayBuffer, ELEVEN);
    ASSERT_EQ(value, HUNDRED_TWENTY_FOUR);
    CallEtsFuntion(&value, "getByte", arrayBuffer, TWELVE);
    ASSERT_EQ(value, FOURTY_TWO);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    int8_t *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int value = 0;
    CallEtsFuntion(&value, "setByte", arrayBuffer, ELEVEN, -HUNDRED_TWENTY_FOUR);
    ASSERT_EQ(data[ELEVEN], -HUNDRED_TWENTY_FOUR);
    CallEtsFuntion(&value, "setByte", arrayBuffer, TWELVE, -FOURTY_TWO);
    ASSERT_EQ(data[TWELVE], -FOURTY_TWO);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSlice)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(HUNDRED_TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_arraybuffer sliced {};
    CallEtsFuntion(&sliced, "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", sliced);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    auto *data = new int8_t[0];
    auto status = env_->ArrayBufferCreateExternal(data, 0, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", arrayBuffer);
    ASSERT_EQ(length, 0);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalCreateWithLength)
{
    ets_arraybuffer arrayBuffer = nullptr;
    auto *data = new int8_t[TWENTY_FOUR];
    auto status = env_->ArrayBufferCreateExternal(data, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", arrayBuffer);
    ASSERT_EQ(length, TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    auto *data = new int8_t[TWENTY_FOUR];
    auto status = env_->ArrayBufferCreateExternal(data, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    data[ELEVEN] = HUNDRED_TWENTY_FOUR;
    data[TWELVE] = FOURTY_TWO;

    ets_int value = 0;
    CallEtsFuntion(&value, "getByte", arrayBuffer, ELEVEN);
    ASSERT_EQ(value, HUNDRED_TWENTY_FOUR);
    CallEtsFuntion(&value, "getByte", arrayBuffer, TWELVE);
    ASSERT_EQ(value, FOURTY_TWO);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    auto *data = new int8_t[TWENTY_FOUR];
    auto status = env_->ArrayBufferCreateExternal(data, TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int value = 0;
    CallEtsFuntion(&value, "setByte", arrayBuffer, ELEVEN, -HUNDRED_TWENTY_FOUR);
    ASSERT_EQ(data[ELEVEN], -HUNDRED_TWENTY_FOUR);
    CallEtsFuntion(&value, "setByte", arrayBuffer, TWELVE, -FOURTY_TWO);
    ASSERT_EQ(data[TWELVE], -FOURTY_TWO);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferExternalSlice)
{
    ets_arraybuffer arrayBuffer = nullptr;
    auto *data = new int8_t[HUNDRED_TWENTY_FOUR];
    auto status = env_->ArrayBufferCreateExternal(data, HUNDRED_TWENTY_FOUR, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    ets_arraybuffer sliced {};
    CallEtsFuntion(&sliced, "createSlicedArrayBuffer", arrayBuffer, TWENTY_FOUR, HUNDRED);
    ets_int length = 0;
    CallEtsFuntion(&length, "getLength", sliced);
    ASSERT_EQ(length, HUNDRED - TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedCreate)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", 0);

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
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", TWENTY_FOUR);

    void *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBufferGetInfo(arrayBuffer, &data, &length);
    ASSERT_EQ(status, ETS_OKAY);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedGet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", TWENTY_FOUR);

    int8_t *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
    ASSERT_EQ(status, ETS_OKAY);

    data[ELEVEN] = HUNDRED_ELEVEN;
    data[TWELVE] = HUNDRED_TWENTY_FOUR;

    ets_int value = 0;
    CallEtsFuntion(&value, "getByte", arrayBuffer, ELEVEN);
    ASSERT_EQ(value, HUNDRED_ELEVEN);
    CallEtsFuntion(&value, "getByte", arrayBuffer, TWELVE);
    ASSERT_EQ(value, HUNDRED_TWENTY_FOUR);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedSet)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", TWENTY_FOUR);

    int8_t *data = nullptr;
    size_t length = 0;
    auto status = env_->ArrayBufferGetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
    ASSERT_EQ(status, ETS_OKAY);

    ets_int value = 0;
    CallEtsFuntion(&value, "setByte", arrayBuffer, ELEVEN, FOURTY_TWO);
    CallEtsFuntion(&value, "setByte", arrayBuffer, TWELVE, FOURTY_THREE);

    ASSERT_EQ(data[ELEVEN], FOURTY_TWO);
    ASSERT_EQ(data[TWELVE], FOURTY_THREE);
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferManagedDetachEmpty)
{
    ets_arraybuffer arrayBuffer = nullptr;
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", 0);

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
    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", TWENTY_FOUR);

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
    auto coro = Coroutine::GetCurrent();

    CallEtsFuntion(&arrayBuffer, "createArrayBuffer", -TWENTY_FOUR);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferGetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    auto coro = Coroutine::GetCurrent();
    ets_int value = 0;
    CallEtsFuntion(&value, "getByte", arrayBuffer, -1);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();

    CallEtsFuntion(&value, "getByte", arrayBuffer, TWENTY_FOUR);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();

    CallEtsFuntion(&value, "getByte", arrayBuffer, TWENTY_FIVE);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();
}

TEST_F(ArrayBufferNativeManagedTest, ArrayBufferSetInvalidArgs)
{
    ets_arraybuffer arrayBuffer = nullptr;
    void *data = nullptr;
    auto status = env_->ArrayBufferCreate(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ETS_OKAY);

    auto coro = Coroutine::GetCurrent();
    ets_int value = 0;
    CallEtsFuntion(&value, "setByte", arrayBuffer, -1, 1);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();

    CallEtsFuntion(&value, "setByte", arrayBuffer, TWENTY_FOUR, 1);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();

    CallEtsFuntion(&value, "setByte", arrayBuffer, TWENTY_FIVE, 1);
    ASSERT_TRUE(coro->HasPendingException());
    coro->ClearException();
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
