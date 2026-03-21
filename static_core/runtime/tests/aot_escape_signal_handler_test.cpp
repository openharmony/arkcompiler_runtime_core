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
#include <csignal>
#include <string>

#include "aot/aot_builder/aot_builder.h"
#include "aot/compiled_method.h"
#include "compiler/code_info/code_info_builder.h"
#include "runtime/include/runtime.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "runtime/signal_handler.h"
#include "mem/gc/gc_types.h"

namespace ark::test {

namespace {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

std::string GetPandaStdLibPath()
{
    auto execPath = ark::os::file::File::GetExecutablePath().Value();
    return execPath + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
}

RuntimeOptions MakeBaseOptions()
{
    RuntimeOptions options;
    options.SetLoadRuntimes({"core"});
    options.SetBootPandaFiles({GetPandaStdLibPath()});
    options.SetGcType("g1-gc");
    options.SetCompilerEnableJit(false);
    options.SetCompilerEnableOsr(false);
    options.SetArkAot(true);
    return options;
}

void BuildMinimalAnFile(const std::string &anPath)
{
    auto options = MakeBaseOptions();
    Runtime::Create(options);
    auto *runtime = Runtime::GetCurrent();

    compiler::AotBuilder aotBuilder;
    aotBuilder.SetArch(RUNTIME_ARCH);
    auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    aotBuilder.SetGcType(static_cast<uint32_t>(gcType));

    const char *pandaFileName = "aot_escape_test.pf";
    aotBuilder.StartFile(pandaFileName, 0xABCD0001U);

    auto *thread = MTManagedThread::GetCurrent();
    if (thread != nullptr) {
        thread->ManagedCodeBegin();
    }
    auto *etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(runtime->GetRuntimeType()));
    static const std::string className = "AotEscapeTestClass";
    auto *klass = etx->CreateClass(reinterpret_cast<const uint8_t *>(className.data()), 0, 0,
                                   AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
    if (thread != nullptr) {
        thread->ManagedCodeEnd();
    }

    constexpr uint32_t classId = 13U;
    klass->SetFileId(panda_file::File::EntityId(classId));
    aotBuilder.StartClass(*klass);

    constexpr uint32_t methodId = 42U;
    Method method(klass, nullptr, panda_file::File::EntityId(methodId), panda_file::File::EntityId(), 0U, 1U, nullptr);
    {
        ArenaAllocator allocator(SpaceType::SPACE_TYPE_COMPILER);
        compiler::CodeInfoBuilder codeBuilder(RUNTIME_ARCH, &allocator);
        ArenaVector<uint8_t> data(allocator.Adapter());
        codeBuilder.Encode(&data);
        compiler::CompiledMethod compiled(RUNTIME_ARCH, &method, 0U);
        static const std::string methodName = "AotEscapeTestClass::testMethod";
        compiled.SetCode(Span(reinterpret_cast<const uint8_t *>(methodName.data()), methodName.size() + 1U));
        compiled.SetCodeInfo(Span(data).ToConst());
        aotBuilder.AddMethod(compiled);
    }

    klass->SetMethods(Span<Method>(&method, 1U), 1U, 0U);
    aotBuilder.EndClass();
    uint32_t hash = GetHash32String(reinterpret_cast<const uint8_t *>(className.data()));
    aotBuilder.InsertEntityPairHeader(hash, classId);
    aotBuilder.InsertClassHashTableSize(1U);
    aotBuilder.EndFile();

    compiler::RuntimeInterface iruntime;
    aotBuilder.SetRuntime(&iruntime);
    aotBuilder.Write("aot_escape_test", anPath);

    Runtime::Destroy();
}

}  // namespace

class AotEscapeSignalHandlerTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        anPath_ = "AotEscapeTestTmp.an";
        BuildMinimalAnFile(anPath_);
    }

    static void TearDownTestSuite()
    {
        std::remove(anPath_.c_str());
    }

    AotEscapeSignalHandlerTest() = default;
    ~AotEscapeSignalHandlerTest() override = default;

    NO_COPY_SEMANTIC(AotEscapeSignalHandlerTest);
    NO_MOVE_SEMANTIC(AotEscapeSignalHandlerTest);

protected:
    static std::string anPath_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::string AotEscapeSignalHandlerTest::anPath_;

TEST_F(AotEscapeSignalHandlerTest, AotEscapeTestSetDirectly)
{
    auto options = MakeBaseOptions();
    options.SetAotFile(anPath_);

    std::string aotEscapePath = "AotEscapeTestSetDirectlyPath.txt";
    std::remove(aotEscapePath.c_str());
    AotEscapeSignalHandler::SetEscaped(false);
    AotEscapeSignalHandler::SetEscapedFlagFilePath(aotEscapePath);

    Runtime::Create(options);
    auto hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_TRUE(hasAot);
    Runtime::Destroy();

    AotEscapeSignalHandler::SetEscaped(true);
    Runtime::Create(options);
    hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_FALSE(hasAot);
    Runtime::Destroy();

    std::remove(aotEscapePath.c_str());
    AotEscapeSignalHandler::SetEscaped(false);
}

#if defined(PANDA_TARGET_AMD64)
TEST_F(AotEscapeSignalHandlerTest, AotEscapeTestCallAction)
{
    auto options = MakeBaseOptions();
    options.SetAotFile(anPath_);

    std::string aotEscapePath = "AotEscapeTest2EscapeFlagPath.txt";
    std::remove(aotEscapePath.c_str());
    AotEscapeSignalHandler::SetEscaped(false);
    AotEscapeSignalHandler::SetEscapedFlagFilePath(aotEscapePath);

    Runtime::Create(options);
    auto hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_TRUE(hasAot);

    ucontext_t uc;
    mcontext_t ucMcontext = {};
    uc.uc_mcontext = ucMcontext;
    AotEscapeSignalHandler::HandleAction(SIGSEGV, nullptr, &uc);
    Runtime::Destroy();

    Runtime::Create(options);
    hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_TRUE(hasAot);

    AotEscapeSignalHandler::HandleAction(SIGABRT, nullptr, &uc);
    AotEscapeSignalHandler::HandleAction(SIGBUS, nullptr, &uc);
    Runtime::Destroy();
    Runtime::Create(options);
    hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_FALSE(hasAot);
    Runtime::Destroy();

    std::remove(aotEscapePath.c_str());
    AotEscapeSignalHandler::SetEscaped(false);
}
#endif

}  // namespace ark::test
