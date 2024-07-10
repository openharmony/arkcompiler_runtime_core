/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cmath>

#include "arkplatform_jsnapi.h"
#include "libpandabase/utils/bit_utils.h"
#include <gtest/gtest.h>

namespace arkplatform {
namespace test {
class JSNapiTest :  public ::testing::Test {
public:

    JSTaggedType ToJSTaggedValue(JSValueRef *obj)
    {
        return *reinterpret_cast<JSTaggedType*>(obj);
    }
 
    JSTaggedType ConvertDouble(double val)
    {
        return panda::bit_cast<JSTaggedType>(val) + JSValueRefInternals::DOUBLE_ENCODE_OFFSET;
    }

    template<typename T>
    void TestNumberRef(T val, JSTaggedType expected)
    {
        LocalScope scope(vm_);
        Local<NumberRef> obj = NumberRef::New(vm_, val);
        ASSERT_TRUE(obj->IsNumber());
        JSTaggedType res = ToJSTaggedValue(*obj);
        ASSERT_EQ(res, expected);
        if constexpr (std::is_floating_point_v<T>) {
            if (std::isnan(val)) {
                ASSERT_TRUE(std::isnan(obj->Value()));
            } else {
                ASSERT_EQ(obj->Value(), val);
            }
        } else if constexpr (sizeof(T) >= sizeof(int32_t)) {
            ASSERT_EQ(obj->IntegerValue(vm_), val);
        } else if constexpr (std::is_signed_v<T>) {
            ASSERT_EQ(obj->Int32Value(vm_), val);
        } else {
            ASSERT_EQ(obj->Uint32Value(vm_), val);
        }
    }

    void SetUp() override
    {
        panda::RuntimeOption option;
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        vm_ = JSNApi::CreateJSVM(option);
        ASSERT_TRUE(vm_ != nullptr) << "Cannot create Runtime";
    }

    void TearDown() override
    {
        JSNApi::DestroyJSVM(vm_);
    }

protected:
    panda::JSThread *thread_ = nullptr;
    panda::ecmascript::EcmaVM *vm_ = nullptr;
};

// If this test failed, please contact with Chernykh Sergey
TEST_F(JSNapiTest, ArkplatformJSNApiTest_NumberRef)
{
    // double
    double dinput = 0.;
    TestNumberRef(dinput, JSValueRefInternals::DOUBLE_ENCODE_OFFSET);
    static constexpr double NAN_VALUE = std::numeric_limits<double>::quiet_NaN();
    TestNumberRef(NAN_VALUE, ConvertDouble(NAN_VALUE));

    // int32_t
    TestNumberRef(static_cast<int32_t>(0), JSValueRefInternals::TAG_INT);
    TestNumberRef(INT32_MIN, static_cast<JSTaggedType>(INT32_MIN) | JSValueRefInternals::TAG_INT);
    TestNumberRef(INT32_MAX, static_cast<JSTaggedType>(INT32_MAX) | JSValueRefInternals::TAG_INT);

    // uint32_t
    TestNumberRef(static_cast<uint32_t>(0), JSValueRefInternals::TAG_INT);
    auto u32input = static_cast<uint32_t>(INT32_MAX);
    TestNumberRef(u32input, u32input | JSValueRefInternals::TAG_INT);
    u32input = static_cast<uint32_t>(INT32_MAX + 1UL);
    TestNumberRef(u32input, ConvertDouble(static_cast<double>(u32input)));
    TestNumberRef(UINT32_MAX, ConvertDouble(static_cast<double>(UINT32_MAX)));

    // int64_t
    auto i64input = static_cast<int64_t>(INT32_MIN);
    TestNumberRef(i64input, i64input | JSValueRefInternals::TAG_INT);
    i64input = static_cast<int64_t>(INT32_MIN);
    TestNumberRef(i64input, i64input | JSValueRefInternals::TAG_INT);
    TestNumberRef(INT64_MIN, ConvertDouble(static_cast<double>(INT64_MIN)));
    TestNumberRef(INT64_MAX, ConvertDouble(static_cast<double>(INT64_MAX)));
}

// If this test failed, please contact with Chernykh Sergey
TEST_F(JSNapiTest, ArkplatformJSNApiTest_BooleanRef)
{
    bool input = true;
    Local<BooleanRef> res = BooleanRef::New(vm_, input);
    EXPECT_TRUE(res->IsBoolean());
    EXPECT_TRUE(res->BooleaValue(vm_));
    ASSERT_EQ(ToJSTaggedValue(*res), JSValueRefInternals::VALUE_TRUE);

    input = false;
    res = BooleanRef::New(vm_, input);
    EXPECT_TRUE(res->IsBoolean());
    EXPECT_FALSE(res->BooleaValue(vm_));
    ASSERT_EQ(ToJSTaggedValue(*res), JSValueRefInternals::VALUE_FALSE);
}

// If this test failed, please contact with Chernykh Sergey
TEST_F(JSNapiTest, ArkplatformJSNApiTest_NullUndefined)
{
    LocalScope scope(vm_);
    Local<JSValueRef> null = JSValueRef::Null(vm_);
    ASSERT_TRUE(null->IsNull());
    ASSERT_EQ(ToJSTaggedValue(*null), JSValueRefInternals::VALUE_NULL);

    Local<JSValueRef> undefined = JSValueRef::Undefined(vm_);
    ASSERT_TRUE(undefined->IsUndefined());
    ASSERT_EQ(ToJSTaggedValue(*undefined), JSValueRefInternals::VALUE_UNDEFINED);
}

} // namespace test
} // namespace arkplatform
