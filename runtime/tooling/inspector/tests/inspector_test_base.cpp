/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "inspector_test_base.h"
#include "combined_event_loop.h"
#include "common.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "class_helper.h"
#include "compiler.h"
#include "mem/panda_string.h"
#include "runtime.h"
#include "runtime_options.h"
#include "source_lang_enum.h"
#include "tooling/pt_thread.h"
#include "utils/logger.h"
#include "utils/utf.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <system_error>
#include <utility>

using testing::HasSubstr;

namespace panda::tooling::inspector::test {
void InspectorTestBase::SetUp()
{
    // Check for unsupported methods on the server, i.e. with id.
    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::WARNING, Logger::Component::DEBUGGER,
                                         HasSubstr(R"(Unsupported method: {"id")"))).Times(0);

    // Set up runtime.
    RuntimeOptions options;
    options.SetShouldInitializeIntrinsics(false);
    options.SetShouldLoadBootPandaFiles(false);
    Runtime::Create(options);

    // Set up connection.
    client_.Connect(server_, [](std::error_code ec) { ASSERT_TRUE(IsOk(ec)); });
    (server_ + client_).Poll();

    SetUpSourceFiles();

    if (AttachDebugger()) {
        client_.Call("Runtime.runIfWaitingForDebugger");
        debugger_.GetHooks().VmInitialization(PtThread::NONE);
    }
}

void InspectorTestBase::TearDown()
{
    (server_ + client_).Poll();
    Runtime::Destroy();
}

Class *InspectorTestBase::LoadSourceFile(const std::string &source)
{
    auto program = pandasm::Parser().Parse(source);
    if (!program) {
        return nullptr;
    }

    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();

    if (auto pf = pandasm::AsmEmitter::Emit(program.Value())) {
        classLinker->AddPandaFile(std::move(pf));
    } else {
        return nullptr;
    }

    PandaString storage;
    auto descriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &storage);

    ScopedMutatorLock lock;
    return classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(descriptor);
}
}  // namespace panda::tooling::inspector::test
