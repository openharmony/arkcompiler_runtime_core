/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"
#include "include/stack_walker.h"
#include "include/stack_walker-inl.h"

#include <vector>
#include <string_view>

namespace ark::ets::ani::testing {

static constexpr std::string_view CHECK_PRIMITIVES_SIGNATURE = "zbcsilfd:i";
static constexpr std::string_view CHECK_REFERENCES_SIGNATURE = "C{std.core.String}i:C{std.core.String}";
static constexpr std::string_view GET_PRIMITIVE_PROP_SIGNATURE = ":i";
static constexpr std::string_view SET_PRIMITIVE_PROP_SIGNATURE = "i:";
static constexpr std::string_view GET_REFERENCE_PROP_SIGNATURE = ":C{std.core.String}";
static constexpr std::string_view SET_REFERENCE_PROP_SIGNATURE = "C{std.core.String}:";

static constexpr ani_boolean BOOLEAN_CONST = ANI_TRUE;
static constexpr ani_byte BYTE_CONST = 12;
static constexpr ani_char CHAR_CONST = 14;
static constexpr ani_short SHORT_CONST = 16;
static constexpr ani_int INT_CONST = 18;
static constexpr ani_long LONG_CONST = 20;
static constexpr ani_float FLOAT_CONST = 22.2F;
static constexpr ani_double DOUBLE_CONST = 24.4;
static constexpr std::string_view STRING_CONST = "TestString";

class ProxyInvokeTest : public AniTest {
public:
    void BindChecker(ani_native_function fn)
    {
        ASSERT_EQ(env_->Module_BindNativeFunctions(mod_, &fn, 1), ANI_OK);
    }

    void GetProxyObject(ani_object &res)
    {
        ani_ref proxyRef {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Function_Call_Ref(createProxyFunction_, &proxyRef), ANI_OK);
        res = static_cast<ani_object>(proxyRef);
    }

protected:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindModule("proxy_invoke_test", &mod_), ANI_OK);
        ASSERT_EQ(env_->Module_FindFunction(mod_, "createProxy", ":C{proxy_invoke_test.I}", &createProxyFunction_),
                  ANI_OK);
        ASSERT_EQ(env_->FindClass("proxy_invoke_test.I", &iClass_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "checkPrimitives", CHECK_PRIMITIVES_SIGNATURE.data(),
                                         &checkPrimitivesMethod_),
                  ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "checkReferences", CHECK_REFERENCES_SIGNATURE.data(),
                                         &checkReferencesMethod_),
                  ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "%%get-primitiveProp", GET_PRIMITIVE_PROP_SIGNATURE.data(),
                                         &getPrimitivePropMethod_),
                  ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "%%set-primitiveProp", SET_PRIMITIVE_PROP_SIGNATURE.data(),
                                         &setPrimitivePropMethod_),
                  ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "%%get-referenceProp", GET_REFERENCE_PROP_SIGNATURE.data(),
                                         &getReferencePropMethod_),
                  ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(iClass_, "%%set-referenceProp", SET_REFERENCE_PROP_SIGNATURE.data(),
                                         &setReferencePropMethod_),
                  ANI_OK);
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ani_class iClass_ {};
    ani_method checkPrimitivesMethod_ {};
    ani_method checkReferencesMethod_ {};
    ani_method getPrimitivePropMethod_ {};
    ani_method setPrimitivePropMethod_ {};
    ani_method getReferencePropMethod_ {};
    ani_method setReferencePropMethod_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    ani_module mod_ {};
    ani_function createProxyFunction_ {};
};

static bool CreateStringReference(ani_env *env, ani_ref &aniRef)
{
    ani_string aniStr = {};
    if (env->String_NewUTF8(STRING_CONST.data(), STRING_CONST.length(), &aniStr) != ANI_OK) {
        return false;
    }
    aniRef = static_cast<ani_ref>(aniStr);
    return true;
}

static bool CheckStringReference(ani_env *env, ani_ref aniRef)
{
    std::string testString;
    AniTest::GetStdString(env, static_cast<ani_string>(aniRef), testString);
    EXPECT_EQ(testString, STRING_CONST);
    return testString == STRING_CONST;
}

template <size_t METHOD_COUNT>
static bool CheckStackWalker(const std::array<const std::string_view, METHOD_COUNT> &methodName)
{
    uint8_t frameIndex = 0;
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    while (stack.HasFrame()) {
        if (frameIndex >= METHOD_COUNT) {
            return false;
        }
        if (methodName[frameIndex] != utf::Mutf8AsCString(stack.GetMethod()->GetName().data)) {
            return false;
        }

        stack.NextFrame();
        frameIndex++;
    }
    return (frameIndex == METHOD_COUNT);
}

static constexpr std::array<const std::string_view, 3> CHECK_PRIMITIVES_METHOD_NAME = {"doCheckPrimitives", "invoke",
                                                                                       "invoke"};

// CC-OFFNXT(G.FUN.01-CPP) test case logic
static void DoCheckPrimitivesImpl(ani_boolean z, ani_byte b, ani_char c, ani_short s, ani_int i, ani_long l,
                                  ani_float f, ani_double d, bool &allMatched)
{
    auto isStackWalkerOk = CheckStackWalker(CHECK_PRIMITIVES_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    EXPECT_EQ(z, BOOLEAN_CONST);
    EXPECT_EQ(b, BYTE_CONST);
    EXPECT_EQ(c, CHAR_CONST);
    EXPECT_EQ(s, SHORT_CONST);
    EXPECT_EQ(i, INT_CONST);
    EXPECT_EQ(l, LONG_CONST);
    EXPECT_EQ(f, FLOAT_CONST);
    EXPECT_EQ(d, DOUBLE_CONST);
    allMatched = (isStackWalkerOk && z == BOOLEAN_CONST && b == BYTE_CONST && c == CHAR_CONST && s == SHORT_CONST &&
                  i == INT_CONST && l == LONG_CONST && f == FLOAT_CONST && d == DOUBLE_CONST);
}

// CC-OFFNXT(G.FUN.01-CPP) test case logic
ani_int DoCheckPrimitives(ani_env *env, ani_boolean z, ani_byte b, ani_char c, ani_short s, ani_int i, ani_long l,
                          ani_float f, ani_double d)
{
    bool allMatched = false;
    DoCheckPrimitivesImpl(z, b, c, s, i, l, f, d, allMatched);
    if (!allMatched) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
    return INT_CONST;
}

TEST_F(ProxyInvokeTest, check_primitives)
{
    ani_native_function nativeFunc = {CHECK_PRIMITIVES_METHOD_NAME[0].data(), CHECK_PRIMITIVES_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckPrimitives)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    ani_int result {};
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Int(proxy, checkPrimitivesMethod_, &result, BOOLEAN_CONST, BYTE_CONST, CHAR_CONST,
                                          SHORT_CONST, INT_CONST, LONG_CONST, FLOAT_CONST, DOUBLE_CONST),
              ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(result, INT_CONST);
}

static constexpr std::array<const std::string_view, 3> CHECK_REFERENCES_METHOD_NAME = {"doCheckReferences", "invoke",
                                                                                       "invoke"};

static void DoCheckReferencesImpl(ani_env *env, ani_ref refArg, ani_int intArg, bool &allMatched)
{
    auto isStackWalkerOk = CheckStackWalker(CHECK_REFERENCES_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    bool isRefArgOk = CheckStringReference(env, refArg);
    EXPECT_EQ(isRefArgOk, true);
    EXPECT_EQ(intArg, INT_CONST);
    allMatched = (isStackWalkerOk && isRefArgOk && intArg == INT_CONST);
}

ani_ref DoCheckReferences(ani_env *env, ani_ref refArg, ani_int intArg)
{
    bool allMatched = false;
    DoCheckReferencesImpl(env, refArg, intArg, allMatched);
    if (!allMatched) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
    return refArg;
}

TEST_F(ProxyInvokeTest, check_references)
{
    ani_native_function nativeFunc = {CHECK_REFERENCES_METHOD_NAME[0].data(), CHECK_REFERENCES_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckReferences)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    ani_ref inpArg0 = {};
    ani_ref outArg {};
    ASSERT_EQ(CreateStringReference(env_, inpArg0), true);
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Ref(proxy, checkReferencesMethod_, &outArg, inpArg0, INT_CONST), ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(CheckStringReference(env_, outArg), true);
}

static constexpr std::array<const std::string_view, 3> GET_PRIMITIVE_PROP_METHOD_NAME = {"doCheckGetPrimitiveProp",
                                                                                         "get", "invokeGet"};

ani_int DoCheckGetPrimitiveProp(ani_env *env)
{
    auto isStackWalkerOk = CheckStackWalker(GET_PRIMITIVE_PROP_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    if (!isStackWalkerOk) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
    return INT_CONST;
}

TEST_F(ProxyInvokeTest, get_primitive_prop)
{
    ani_native_function nativeFunc = {GET_PRIMITIVE_PROP_METHOD_NAME[0].data(), GET_PRIMITIVE_PROP_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckGetPrimitiveProp)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    ani_int outArg {};
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Int(proxy, getPrimitivePropMethod_, &outArg), ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(outArg, INT_CONST);
}

static constexpr std::array<const std::string_view, 3> SET_PRIMITIVE_PROP_METHOD_NAME = {"doCheckSetPrimitiveProp",
                                                                                         "set", "invokeSet"};

static void DoCheckSetPrimitivePropImpl(ani_int intArg, bool &allMatched)
{
    bool isStackWalkerOk = CheckStackWalker(SET_PRIMITIVE_PROP_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    EXPECT_EQ(intArg, INT_CONST);
    allMatched = (isStackWalkerOk && intArg == INT_CONST);
}

void DoCheckSetPrimitiveProp(ani_env *env, ani_int intArg)
{
    bool allMatched = false;
    DoCheckSetPrimitivePropImpl(intArg, allMatched);
    if (!allMatched) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
}

TEST_F(ProxyInvokeTest, set_primitive_prop)
{
    ani_native_function nativeFunc = {SET_PRIMITIVE_PROP_METHOD_NAME[0].data(), SET_PRIMITIVE_PROP_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckSetPrimitiveProp)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Void(proxy, setPrimitivePropMethod_, INT_CONST), ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

static constexpr std::array<const std::string_view, 3> GET_REFERENCE_PROP_METHOD_NAME = {"doCheckGetReferenceProp",
                                                                                         "get", "invokeGet"};

ani_ref DoCheckGetReferenceProp(ani_env *env)
{
    bool isStackWalkerOk = CheckStackWalker(GET_REFERENCE_PROP_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    ani_ref aniRef = {};
    bool isStrRefCreate = CreateStringReference(env, aniRef);
    EXPECT_EQ(isStrRefCreate, true);
    if (!isStackWalkerOk || !isStrRefCreate) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
    return aniRef;
}

TEST_F(ProxyInvokeTest, get_reference_prop)
{
    ani_native_function nativeFunc = {GET_REFERENCE_PROP_METHOD_NAME[0].data(), GET_REFERENCE_PROP_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckGetReferenceProp)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    ani_ref outArg {};
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Ref(proxy, getReferencePropMethod_, &outArg), ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(CheckStringReference(env_, outArg), true);
}

static constexpr std::array<const std::string_view, 3> SET_REFERENCE_PROP_METHOD_NAME = {"doCheckSetReferenceProp",
                                                                                         "set", "invokeSet"};

static void DoCheckSetReferencePropImpl(ani_env *env, ani_ref refArg, bool &allMatched)
{
    bool isStackWalkerOk = CheckStackWalker(SET_REFERENCE_PROP_METHOD_NAME);
    EXPECT_EQ(isStackWalkerOk, true);
    bool isStringRefArgOk = CheckStringReference(env, refArg);
    EXPECT_EQ(isStringRefArgOk, true);
    allMatched = (isStackWalkerOk && isStringRefArgOk);
}

void DoCheckSetReferenceProp(ani_env *env, ani_ref refArg)
{
    bool allMatched = false;
    DoCheckSetReferencePropImpl(env, refArg, allMatched);
    if (!allMatched) {
        if (AniTest::ThrowError(env) != ANI_OK) {
            ADD_FAILURE() << "ThrowError() failed to set pending exception";
        }
    }
}

TEST_F(ProxyInvokeTest, set_reference_prop)
{
    ani_native_function nativeFunc = {SET_REFERENCE_PROP_METHOD_NAME[0].data(), SET_REFERENCE_PROP_SIGNATURE.data(),
                                      reinterpret_cast<void *>(DoCheckSetReferenceProp)};
    BindChecker(nativeFunc);

    ani_object proxy {};
    GetProxyObject(proxy);

    ani_ref inpArg0 = {};
    ASSERT_EQ(CreateStringReference(env_, inpArg0), true);
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Void(proxy, setReferencePropMethod_, inpArg0), ANI_OK);
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}
}  // namespace ark::ets::ani::testing
