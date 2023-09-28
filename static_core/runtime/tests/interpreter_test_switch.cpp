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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "bytecode_instruction.h"
#include "include/thread_scopes.h"
#include "libpandafile/bytecode_instruction-inl.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/interpreter/runtime_interface.h"

namespace panda::interpreter::test {

class InterpreterTestSwitch : public testing::Test {
public:
    InterpreterTestSwitch()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetInterpreterType("cpp");
        Runtime::Create(options);
    }

    ~InterpreterTestSwitch() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(InterpreterTestSwitch);
    NO_MOVE_SEMANTIC(InterpreterTestSwitch);
};

constexpr int32_t RET = 10;

static int32_t EntryPoint(Method * /* unused */)
{
    auto *thread = ManagedThread::GetCurrent();
    thread->SetCurrentDispatchTable<false>(thread->GetDebugDispatchTable());
    return RET;
}

TEST_F(InterpreterTestSwitch, SwitchToDebug)
{
    pandasm::Parser p;

    auto source = R"(
        .function i32 f() {
            ldai 10
            return
        }

        .function i32 g() {
            call f
            return
        }

        .function i32 main() {
            call g
            return
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));
    auto *extension = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    class Listener : public RuntimeListener {
    public:
        struct Event {
            ManagedThread *thread;
            Method *method;
            uint32_t bc_offset;
        };

        void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bc_offset) override
        {
            events_.push_back({thread, method, bc_offset});
        }

        auto &GetEvents() const
        {
            return events_;
        }

    private:
        std::vector<Event> events_ {};
    };

    Listener listener {};

    auto *notification_manager = Runtime::GetCurrent()->GetNotificationManager();
    notification_manager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    std::vector<Value> args;
    Value v;
    Method *main_method;

    auto *thread = ManagedThread::GetCurrent();

    {
        ScopedManagedCodeThread smc(thread);
        PandaString descriptor;

        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        ASSERT_NE(klass, nullptr);

        main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
        ASSERT_NE(main_method, nullptr);

        Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(f_method, nullptr);

        f_method->SetCompiledEntryPoint(reinterpret_cast<const void *>(EntryPoint));

        v = main_method->Invoke(thread, args.data());
    }

    notification_manager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    ASSERT_EQ(v.GetAs<int32_t>(), RET);
    ASSERT_EQ(listener.GetEvents().size(), 1U);

    auto &event = listener.GetEvents()[0];
    EXPECT_EQ(event.thread, thread);
    EXPECT_EQ(event.method, main_method);
    EXPECT_EQ(event.bc_offset, BytecodeInstruction::Size(BytecodeInstruction::Format::V4_V4_ID16));
}

}  // namespace panda::interpreter::test
