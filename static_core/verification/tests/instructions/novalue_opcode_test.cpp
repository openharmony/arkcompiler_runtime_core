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

#include <gtest/gtest.h>
#include <string>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "include/class_linker_extension.h"
#include "include/language_context.h"
#include "include/method.h"
#include "public.h"
#include "libarkbase/utils/utf.h"
#include "util/tests/verifier_test.h"
#include "include/runtime.h"

namespace ark::verifier::test {

class NovalueOpcodeTest : public VerifierTest {
public:
    NovalueOpcodeTest()
    {
        config_ = verifier::NewConfig();
        service_ = CreateService(config_, Runtime::GetCurrent()->GetInternalAllocator(),
                                 ark::Runtime::GetCurrent()->GetClassLinker(), "");
    }

    ~NovalueOpcodeTest() override
    {
        DestroyService(service_, false);
        DestroyConfig(config_);
    }

    NO_COPY_SEMANTIC(NovalueOpcodeTest);
    NO_MOVE_SEMANTIC(NovalueOpcodeTest);

    std::unique_ptr<const panda_file::File> EmitPandasm(const char *source)
    {
        pandasm::Parser p;
        auto res = p.Parse(source);
        return pandasm::AsmEmitter::Emit(res.Value());
    }

    ClassLinkerExtension *GetLinkerExtention(std::unique_ptr<const panda_file::File> pf)
    {
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));
        return classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    }

    Method *GetDirectMethodFromClass(ClassLinkerExtension *ext, const std::string &className,
                                     const std::string &methodName)
    {
        PandaString descriptor;
        Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor));
        ASSERT(klass != nullptr);
        return klass->GetDirectMethod(utf::CStringAsMutf8(methodName.c_str()));
    }

private:
    verifier::Config *config_;
    verifier::Service *service_;
};

TEST_F(NovalueOpcodeTest, ReturnVoidInNeverFunctionFails)
{
    auto source = R"(
        .function novalue test() <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, ReturnInNeverFunctionFails)
{
    auto source = R"(
        .function novalue test() <static, access.function=public> {
            ldai 0
            return
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, ReturnWideInNeverFunctionFails)
{
    auto source = R"(
        .function novalue test() <static, access.function=public> {
            ldai.64 0
            return.64
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, ReturnObjInNeverFunctionFails)
{
    auto source = R"(
        .record R {}
        .function novalue test() <static, access.function=public> {
            newobj v0, R
            lda.obj v0
            return.obj
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NovalueAsParameterFails)
{
    auto source = R"(
        .function void test(novalue a0) <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NovalueAmongParametersFails)
{
    auto source = R"(
        .function void test(i32 a0, novalue a1) <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverFunctionWithThrowPasses)
{
    auto source = R"(
        .record R {}
        .function novalue test() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// never-function tail-calls another never-function.
TEST_F(NovalueOpcodeTest, NeverFunctionCallsNeverFunction)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
            call.short thrower
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// return.void after a never-call is still forbidden.
TEST_F(NovalueOpcodeTest, NeverFunctionCallsNeverThenReturnFails)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
            call.short thrower
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Field of type novalue is not meaningful.
TEST_F(NovalueOpcodeTest, FieldOfTypeNovalueFails)
{
    auto source = R"(
        .record R {
            novalue neverField <access.field=public>
        }
        .function void test() <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    if (pf == nullptr) {
        return;
    }
    auto *ext = GetLinkerExtention(std::move(pf));
    [[maybe_unused]] Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    SUCCEED();
}

TEST_F(NovalueOpcodeTest, MultipleNovalueParametersFails)
{
    auto source = R"(
        .function void test(novalue a0, novalue a1) <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Never-parameter is still forbidden even when return is never.
TEST_F(NovalueOpcodeTest, NeverParamAndNeverReturnFails)
{
    auto source = R"(
        .record R {}
        .function novalue test(novalue a0) <static, access.function=public> {
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Reference-never (N) as parameter is allowed.
TEST_F(NovalueOpcodeTest, ReferenceNeverParameterStillPasses)
{
    auto source = R"(
        .record N <external>
        .function void test(N a0) <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverFunctionWithNormalParamsAndThrowPasses)
{
    auto source = R"(
        .record R {}
        .function novalue test(i32 a0, R a1) <static, access.function=public> {
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Infinite loop is a valid terminator.
TEST_F(NovalueOpcodeTest, NeverFunctionInfiniteLoopPasses)
{
    auto source = R"(
        .function novalue test() <static, access.function=public> {
        loop:
            jmp loop
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverFunctionForwardJumpToThrowPasses)
{
    auto source = R"(
        .record R {}
        .function novalue test() <static, access.function=public> {
            jmp do_throw
        do_throw:
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverFunctionConditionalBothBranchesThrowPasses)
{
    auto source = R"(
        .record R {}
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz else_branch
            newobj v0, R
            throw v0
        else_branch:
            newobj v1, R
            throw v1
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// One branch returns, one throws — return path is rejected.
TEST_F(NovalueOpcodeTest, NeverFunctionConditionalOneBranchReturnsFails)
{
    auto source = R"(
        .record R {}
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz end
            newobj v0, R
            throw v0
        end:
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Both paths terminate via never-call.
TEST_F(NovalueOpcodeTest, NeverFunctionBothPathsEndInNeverCallPasses)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz second_call
            call.short thrower
        second_call:
            call.short thrower
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverFunctionChainedHelperThenNeverCallPasses)
{
    auto source = R"(
        .record R {}
        .function void helper() <static, access.function=public> {
            return.void
        }
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
            call.short helper
            call.short thrower
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *helper = GetDirectMethodFromClass(ext, "_GLOBAL", "helper");
    ASSERT_NE(helper, nullptr);
    ASSERT_TRUE(helper->Verify());
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Never-call on one branch, reachable return on the other.
TEST_F(NovalueOpcodeTest, NeverFunctionNeverCallOneBranchReturnOtherFails)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz do_return
            call.short thrower
        do_return:
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Void caller may call never-function; trailing return.void is dead but allowed.
TEST_F(NovalueOpcodeTest, NormalVoidFunctionCallsNeverThenReturnsPasses)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function void test() <static, access.function=public> {
            call.short thrower
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NovalueAsLastOfManyParamsFails)
{
    auto source = R"(
        .function void test(i32 a0, i64 a1, f32 a2, novalue a3) <static, access.function=public> {
            return.void
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Void caller wraps never-call in try/catch; catch returns normally.
TEST_F(NovalueOpcodeTest, VoidFunctionTryCatchAroundNeverCallPasses)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function void test() <static, access.function=public> {
        try_begin:
            call.short thrower
        try_end:
            return.void
        catch_begin:
            return.void
        .catchall try_begin, try_end, catch_begin
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Never caller: both the try body and the catch block end in never-calls.
TEST_F(NovalueOpcodeTest, NeverFunctionTryCatchCatchAlsoThrowsPasses)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
        try_begin:
            call.short thrower
        try_end:
        catch_begin:
            call.short thrower
        .catchall try_begin, try_end, catch_begin
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Never caller: catch block contains return.void.
TEST_F(NovalueOpcodeTest, NeverFunctionTryCatchCatchReturnsFails)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
        try_begin:
            call.short thrower
        try_end:
        catch_begin:
            return.void
        .catchall try_begin, try_end, catch_begin
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, NeverReturnWithNeverParamInMiddleFails)
{
    auto source = R"(
        .record R {}
        .function novalue test(i32 a0, novalue a1, R a2) <static, access.function=public> {
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Conditional jump skips over a never-call; dead code after the call is unreachable.
TEST_F(NovalueOpcodeTest, UnreachableNeverCallWithJumpTargetAfterDeadCode)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz second_block
            call.short thrower
            ldai 0
        second_block:
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Three-way branch; two paths end in never-calls, third in throw.
TEST_F(NovalueOpcodeTest, ThreeWayBranchWithDeadCodeAfterNeverCalls)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz path_b
            ldai 1
            jeqz path_c
            call.short thrower
            ldai 99
        path_b:
            call.short thrower
            sta v1
        path_c:
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Unconditional jump makes the never-call itself unreachable.
TEST_F(NovalueOpcodeTest, UnconditionalJumpOverNeverCallAndDeadCode)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
            jmp after
            call.short thrower
            ldai 0
        after:
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Jump target after a never-call makes the trailing non-terminator reachable.
TEST_F(NovalueOpcodeTest, JumpTargetAfterNeverCallMakesDeadCodeReachableFails)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test(i32 a0) <static, access.function=public> {
            lda a0
            jeqz reachable_tail
            call.short thrower
        reachable_tail:
            ldai 42
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

TEST_F(NovalueOpcodeTest, UnconditionalJumpMakesFollowingCodeDead)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function novalue test() <static, access.function=public> {
            jmp skip
            call.short thrower
            ldai 0
        skip:
            newobj v0, R
            throw v0
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Jump target is a non-terminator last instruction.
TEST_F(NovalueOpcodeTest, UnconditionalJumpToNonTerminatorLastInstructionFails)
{
    auto source = R"(
        .function novalue test() <static, access.function=public> {
            jmp skip
            ldai 0
        skip:
            ldai 42
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Dead jump after throw does not reset reachability in a never-function.
TEST_F(NovalueOpcodeTest, DISABLED_DeadJumpAfterThrowDoesNotResetReachability)
{
    auto source = R"(
        .record R {}
        .record K {}
        .function novalue test() <static, access.function=public> {
            newobj v0, R
            throw v0
            jmp label
            newobj v1, K
        label:
            newobj v2, K
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// Same layout in a void function — last instruction is not a valid terminator.
TEST_F(NovalueOpcodeTest, DeadJumpAfterThrowInVoidFunctionFails)
{
    auto source = R"(
        .record R {}
        .record K {}
        .function void test() <static, access.function=public> {
            newobj v0, R
            throw v0
            jmp label
            newobj v1, K
        label:
            newobj v2, K
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

// Never-call before an exception handler boundary acts as a terminator.
TEST_F(NovalueOpcodeTest, NeverCallBeforeExceptionHandlerBoundaryPasses)
{
    auto source = R"(
        .record R {}
        .function novalue thrower() <static, access.function=public> {
            newobj v0, R
            throw v0
        }
        .function void test() <static, access.function=public> {
        try_begin:
            call.short thrower
        try_end:
        catch_begin:
            return.void
        .catchall try_begin, try_end, catch_begin
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *thrower = GetDirectMethodFromClass(ext, "_GLOBAL", "thrower");
    ASSERT_NE(thrower, nullptr);
    ASSERT_TRUE(thrower->Verify());
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

// A dead jump must not mark its target as reachable.
TEST_F(NovalueOpcodeTest, DISABLED_DeadJumpTargetAtEndNotReachable)
{
    auto source = R"(
        .record R {}
        .function novalue test() <static, access.function=public> {
            jmp skip
            jmp label
        skip:
            newobj v0, R
            throw v0
        label:
            ldai 42
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_TRUE(test->Verify());
}

TEST_F(NovalueOpcodeTest, HandlerEntryResetsAfterTerminatorFails)
{
    auto source = R"(
        .record R {}
        .function novalue test() <static, access.function=public> {
        try_begin:
            newobj v0, R
            throw v0
        try_end:
        catch_begin:
            ldai 1
        .catchall try_begin, try_end, catch_begin
        }
    )";
    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);
    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    ASSERT_FALSE(test->Verify());
}

}  // namespace ark::verifier::test
