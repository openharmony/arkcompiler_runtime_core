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

#ifndef VERIFICATION_TESTS_ETS_INSTRUCTION_TEST_H
#define VERIFICATION_TESTS_ETS_INSTRUCTION_TEST_H

#include <gtest/gtest.h>
#include <string>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "include/method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "public.h"
#include "verification/type/type_system.h"
#include "verification/public_internal.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets::test {

class VerifierEtsOpcodeTest : public testing::Test {
public:
    VerifierEtsOpcodeTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            UNREACHABLE();
        }
        options_.SetBootPandaFiles({stdlib});
        Runtime::Create(options_);

        config_ = verifier::NewConfig();
        service_ = CreateService(config_, Runtime::GetCurrent()->GetInternalAllocator(),
                                 ark::Runtime::GetCurrent()->GetClassLinker(), "");
    }

    ~VerifierEtsOpcodeTest() override
    {
        DestroyService(service_, false);
        DestroyConfig(config_);
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(VerifierEtsOpcodeTest);
    NO_MOVE_SEMANTIC(VerifierEtsOpcodeTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

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
        auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
            ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
        return ext;
    }

    Method *GetDirectMethodFromClass(ClassLinkerExtension *ext, const std::string &className,
                                     const std::string &methodName)
    {
        PandaString descriptor;
        Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor));
        Method *func = klass->GetDirectMethod(utf::CStringAsMutf8(methodName.c_str()));
        return func;
    }

protected:
    verifier::Service *service_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    verifier::Config *config_ = nullptr;
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

}  // namespace ark::ets::test

#endif
