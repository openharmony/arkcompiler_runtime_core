/*
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

#include <gtest/gtest.h>

#include <vector>

#include "assembly-parser.h"
#include "libpandafile/value.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"

namespace panda::test {

inline std::string separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class PandaExceptionTest : public testing::Test {
protected:
    enum class ExType { INT = 0, JIT };

    PandaExceptionTest()
    {
        options_.SetHeapSizeLimit(64_MB);
        options_.SetInternalMemorySizeLimit(64_MB);

        options_.SetShouldInitializeIntrinsics(false);

        options_.SetBootClassSpaces({"core"});
        options_.SetRuntimeType("core");

        options_.SetGcType("gen-gc");

        auto exec_path = panda::os::file::File::GetExecutablePath();
        std::string panda_std_lib =
            exec_path.Value() + separator() + ".." + separator() + "pandastdlib" + separator() + "pandastdlib.bin";
        options_.SetBootPandaFiles({panda_std_lib});
    }

    ~PandaExceptionTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    inline void CreateRuntime(ExType ex_type)
    {
        switch (ex_type) {
            case ExType::INT: {
                options_.SetCompilerEnableJit(false);
                break;
            }

            case ExType::JIT: {
                options_.SetCompilerEnableJit(true);
                options_.SetCompilerHotnessThreshold(0);
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        Runtime::Create(options_);

        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    panda::MTManagedThread *thread_ {nullptr};
    RuntimeOptions options_;
};

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallShortINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            call.short R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallShortJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            call.short R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor
            sta.obj v0
            call R.CauseAbstractMethodException, v0, v1, v2, v3
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor
            sta.obj v0
            call R.CauseAbstractMethodException, v0, v1, v2, v3
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallRangeINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            call.range R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallRangeJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            call.range R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallAccShortINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            initobj R.ctor
            call.acc.short R.CauseAbstractMethodException, v0, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallAccShortJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            initobj R.ctor
            call.acc.short R.CauseAbstractMethodException, v0, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallAccINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            movi v1, 2
            movi v2, 3
            initobj R.ctor
            call.acc R.CauseAbstractMethodException, v0, v1, v2, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodStaticCallAccJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            movi v1, 2
            movi v2, 3
            initobj R.ctor
            call.acc R.CauseAbstractMethodException, v0, v1, v2, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallShortINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            call.virt.short R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallShortJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            call.virt.short R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor
            sta.obj v0
            call.virt R.CauseAbstractMethodException, v0, v1, v2, v3
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor
            sta.obj v0
            call.virt R.CauseAbstractMethodException, v0, v1, v2, v3
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallRangeINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            call.virt.range R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallRangeJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj R.ctor
            sta.obj v0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            call.virt.range R.CauseAbstractMethodException, v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallAccShortINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            initobj R.ctor
            call.virt.acc.short R.CauseAbstractMethodException, v0, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallAccShortJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            initobj R.ctor
            call.virt.acc.short R.CauseAbstractMethodException, v0, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallAccINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            movi v1, 2
            movi v2, 3
            initobj R.ctor
            call.virt.acc R.CauseAbstractMethodException, v0, v1, v2, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodVirtualCallAccJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <ctor> {
        return.void
    }

    .function void R.CauseAbstractMethodException(R a0, i32 a1, i32 a2, i32 a3) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 1
            movi v1, 2
            movi v2, 3
            initobj R.ctor
            call.virt.acc R.CauseAbstractMethodException, v0, v1, v2, 0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectShortINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj.short R.ctor
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectShortJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            initobj.short R.ctor
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor, v0, v1, v2, v3
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0, i32 a1, i32 a2, i32 a3, i32 a4) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            initobj R.ctor, v0, v1, v2, v3
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectRangeINT)
{
    CreateRuntime(ExType::INT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0, i32 a1, i32 a2, i32 a3, i32 a4, i32 a5) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            initobj.range R.ctor, v0
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

/**
 * This test is a core test that's supposed
 * to provoke the AbstractMethodException, to catch it,
 * and to return 0 as the result.
 */
TEST_F(PandaExceptionTest, AbstractMethodInitObjectRangeJIT)
{
    CreateRuntime(ExType::JIT);

    auto source = R"(
    .record panda.AbstractMethodError <external>

    .record panda.Exception <external>

    .record R {}

    .function void R.ctor(R a0, i32 a1, i32 a2, i32 a3, i32 a4, i32 a5) <noimpl>

    .record ProvokeAbstractMethodException {
    }

    .function i32 ProvokeAbstractMethodException.main() <static> {
        try_begin:
            movi v0, 0
            movi v1, 1
            movi v2, 2
            movi v3, 3
            movi v4, 4
            initobj.range R.ctor, v0
            sta.obj v0
        try_end:
            jmp no_exceptions
        handler_begin_incorrect:
            movi v0, 0x2
            lda v0
            return
        handler_begin_correct:
            movi v0, 0x0
            lda v0
            return
        no_exceptions:
            movi v0, 0xffffffffffffffff
            lda v0
            return

      .catch panda.AbstractMethodError, try_begin, try_end, handler_begin_correct
      .catch panda.Exception, try_begin, try_end, handler_begin_incorrect
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();

    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass =
        class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
            ClassHelper::GetDescriptor(utf::CStringAsMutf8("ProvokeAbstractMethodException"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value result = method->Invoke(ManagedThread::GetCurrent(), args.data());

    int64_t expected_result = 0;
    int64_t unexpected_exception = 2;
    int64_t no_exceptions = -1;

    ASSERT_NE(result.GetAs<int64_t>(), unexpected_exception)
        << "AbstractMethod exception should have been thrown, but another has";
    ASSERT_NE(result.GetAs<int64_t>(), no_exceptions) << "No exceptions were thrown";
    ASSERT_EQ(result.GetAs<int64_t>(), expected_result) << "Unexpected error";
}

}  // namespace panda::test