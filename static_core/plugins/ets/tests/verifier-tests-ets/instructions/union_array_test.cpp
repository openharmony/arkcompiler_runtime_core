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

#include <gtest/gtest.h>

#include "ets_opcode_test.h"

namespace ark::ets::test {

// isinstance on a union of arrays whose element types are unions themselves:
// Type::GetArrayElementType must produce a normalized union (no nested unions).
TEST_F(VerifierEtsOpcodeTest, IsinstanceUnionOfArraysOfUnions)
{
    auto source = R"(
        .language eTS
        .record Test {}
        .record A {}
        .record B {}
        .record C {}
        .record D {}
        .function void Test.test({U{UA, B}[], {UC, D}[]} a0) <static, access.function=public> {
            lda.obj a0
            isinstance {UA, B}[]
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "Test", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

// checkcast to an unrelated array type makes the register type Bot (the path is
// infeasible at runtime); a subsequent ldarr.obj must not report an error.
TEST_F(VerifierEtsOpcodeTest, LdarrObjAfterImpossibleArrayCheckcast)
{
    auto source = R"(
        .language eTS
        .record Test {}
        .record A {}
        .record B {}
        .function void Test.test(B[] a0, i32 a1) <static, access.function=public> {
            lda.obj a0
            checkcast A[]
            sta.obj v0
            lda a1
            ldarr.obj v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "Test", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

// Primitive arrays are invariant: checkcast u1[] -> u16[] is impossible and yields Bot;
// a subsequent primitive array load must not report an error.
TEST_F(VerifierEtsOpcodeTest, PrimitiveLdarrAfterImpossibleArrayCheckcast)
{
    auto source = R"(
        .language eTS
        .record Test {}
        .function void Test.test(u1[] a0, i32 a1) <static, access.function=public> {
            lda.obj a0
            checkcast u16[]
            sta.obj v0
            lda a1
            ldarru.16 v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "Test", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

// Same as above for a primitive array store.
TEST_F(VerifierEtsOpcodeTest, PrimitiveStarrAfterImpossibleArrayCheckcast)
{
    auto source = R"(
        .language eTS
        .record Test {}
        .function void Test.test(u1[] a0, i32 a1) <static, access.function=public> {
            lda.obj a0
            checkcast u16[]
            sta.obj v0
            ldai 7
            starr.16 v0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "Test", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

// Same as above for an object array store.
TEST_F(VerifierEtsOpcodeTest, StarrObjAfterImpossibleArrayCheckcast)
{
    auto source = R"(
        .language eTS
        .record Test {}
        .record A {}
        .record B {}
        .function void Test.test(B[] a0, i32 a1, A a2) <static, access.function=public> {
            lda.obj a0
            checkcast A[]
            sta.obj v0
            lda.obj a2
            starr.obj v0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "Test", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

}  // namespace ark::ets::test
