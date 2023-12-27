/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "absint/reg_context.h"
#include "absint/exec_context.h"

#include "public_internal.h"
#include "jobs/service.h"
#include "type/type_system.h"
#include "value/abstract_typed_value.h"

#include "util/lazy.h"
#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

#include <functional>

// NOLINTBEGIN(readability-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace panda::verifier::test {

TEST_F(VerifierTest, AbsIntExecContext)
{
    using Builtin = Type::Builtin;
    auto *config = NewConfig();
    RuntimeOptions runtimeOpts;
    Runtime::Create(runtimeOpts);
    auto *service = CreateService(config, Runtime::GetCurrent()->GetInternalAllocator(),
                                  Runtime::GetCurrent()->GetClassLinker(), "");
    TypeSystem typeSystem(service->verifierService);
    Variables variables;

    auto i16 = Type {Builtin::I16};
    auto i32 = Type {Builtin::I32};

    auto u16 = Type {Builtin::U16};

    auto nv = [&variables] { return variables.NewVar(); };

    AbstractTypedValue av1 {i16, nv()};
    AbstractTypedValue av2 {i32, nv()};
    AbstractTypedValue av3 {u16, nv()};

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint8_t instructions[128];

    ExecContext execCtx {&instructions[0], &instructions[127], &typeSystem};

    std::array<const uint8_t *, 6> cp = {&instructions[8],  &instructions[17], &instructions[23],
                                         &instructions[49], &instructions[73], &instructions[103]};

    execCtx.SetCheckPoints(ConstLazyFetch(cp));

    // 1 1 1 1 1 1
    // C I C I C I
    //   E     E

    RegContext ctx1;
    RegContext ctx2;
    RegContext ctx3;

    ctx1[-1] = av1;
    ctx1[0] = av2;

    ctx2[0] = av1;  // compat with 1

    ctx3[-1] = av3;  // incompat with 1

    execCtx.CurrentRegContext() = ctx1;

    execCtx.StoreCurrentRegContextForAddr(cp[0]);
    execCtx.StoreCurrentRegContextForAddr(cp[1]);
    execCtx.StoreCurrentRegContextForAddr(cp[2]);
    execCtx.StoreCurrentRegContextForAddr(cp[3]);
    execCtx.StoreCurrentRegContextForAddr(cp[4]);
    execCtx.StoreCurrentRegContextForAddr(cp[5]);

    execCtx.AddEntryPoint(cp[1], EntryPointType::METHOD_BODY);
    execCtx.AddEntryPoint(cp[4], EntryPointType::METHOD_BODY);

    const uint8_t *ep = &instructions[0];
    EntryPointType ept;

    execCtx.CurrentRegContext() = ctx2;
    while (ep != cp[0]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep++);

    execCtx.CurrentRegContext() = ctx3;
    while (ep != cp[1]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep);

    execCtx.CurrentRegContext() = ctx2;
    while (ep != cp[2]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep++);

    execCtx.CurrentRegContext() = ctx3;
    while (ep != cp[3]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep++);

    execCtx.CurrentRegContext() = ctx2;
    while (ep != cp[4]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep++);

    execCtx.CurrentRegContext() = ctx3;
    while (ep != cp[5]) {
        execCtx.StoreCurrentRegContextForAddr(ep++);
    }
    execCtx.StoreCurrentRegContextForAddr(ep++);

    execCtx.GetEntryPointForChecking(&ep, &ept);

    execCtx.GetEntryPointForChecking(&ep, &ept);

    auto status = execCtx.GetEntryPointForChecking(&ep, &ept);

    EXPECT_EQ(status, ExecContext::Status::ALL_DONE);

    DestroyService(service, false);
    DestroyConfig(config);
}

}  // namespace panda::verifier::test

// NOLINTEND(readability-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)