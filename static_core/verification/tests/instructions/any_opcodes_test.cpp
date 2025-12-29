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

#include "opcode_test.h"

namespace ark::verifier::test {

TEST_F(VerifyOpcodeTest, AnyIsInstance)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a0
            any.isinstance a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyCallShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, NegAnyCallShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            ldai 1
            sta v1
            any.call.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

TEST_F(VerifyOpcodeTest, AnyCallRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.range a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyCallThisShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.this.short "bar", a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyCallThisRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.this.range "foo", a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyCall0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.0 a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyNew0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.new.0 a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyNewRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.new.range a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyNewShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.new.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyCallThis0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.this.0 "baz", a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyLdbyval)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyLdbyname)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyname a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyLdbynamev)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyname.v v1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyLdbyidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            any.ldbyidx a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbyval)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a2
            any.stbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbyvalWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.null
            any.stbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbyname)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            any.stbyname a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbynameWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.null
            sta.obj v1
            any.stbyname v1, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbynamev)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.stbyname.v a1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStbynamevWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            lda.null
            sta.obj v1
            any.stbyname.v v1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStvalidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v0
            lda.obj a1
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, AnyStvalidxWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v0
            lda.null
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyOpcodeTest, NegAnyStvalidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai 0x3ff0000000000000
            sta v0
            lda.obj a1
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

}  // namespace ark::verifier::test