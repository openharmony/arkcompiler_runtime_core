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

TEST_F(VerifierEtsOpcodeTest, DISABLED_NegDispatch1)
{
    verifier::TypeSystem typeSystem(service_->verifierService, ark::panda_file::SourceLang::ETS);

    // ets.async.dispatch with wrong argument type
    const auto source =
        ".language eTS\n"
        ".record R {}\n"
        ".record ets.coroutine.Async {} <external>\n"
        ".record ETSGLOBAL <extends=panda.Object, access.record=public> {}\n"
        ".function void ETSGLOBAL.test(R a0)"
        "<static, access.function=public, ets.annotation.class=ets.coroutine.Async, ets.annotation.id=id_0>"
        "{\n"
        "    ets.async.dispatch a0\n"
        "    return.void\n"
        "}\n";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "ETSGLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

TEST_F(VerifierEtsOpcodeTest, DISABLED_NegDispatch2)
{
    verifier::TypeSystem typeSystem(service_->verifierService, ark::panda_file::SourceLang::ETS);

    // ets.async.dispatch outside of async function
    const auto source = R"(
    .language eTS
    .record arkruntime.AsyncContext {} <external>
    .record ETSGLOBAL <extends=panda.Object, access.record=public> {}
    .function void ETSGLOBAL.test(arkruntime.AsyncContext a0) <static, access.function=public> {
        ets.async.dispatch a0
        return.void
    }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "ETSGLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

TEST_F(VerifierEtsOpcodeTest, DISABLED_PosDispatch1)
{
    verifier::TypeSystem typeSystem(service_->verifierService, ark::panda_file::SourceLang::ETS);

    const auto source =
        ".language eTS\n"
        ".record arkruntime.AsyncContext {} <external>\n"
        ".record ets.coroutine.Async {} <external>\n"
        ".record ETSGLOBAL <extends=panda.Object, access.record=public> {}\n"
        ".function void ETSGLOBAL.test(arkruntime.AsyncContext a0)"
        "<static, access.function=public, ets.annotation.class=ets.coroutine.Async, ets.annotation.id=id_0>"
        "{\n"
        "    ets.async.dispatch a0\n"
        "    return.void\n"
        "}\n";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "ETSGLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

}  // namespace ark::ets::test