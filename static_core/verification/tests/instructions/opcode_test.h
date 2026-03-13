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

#ifndef VERIFICATION_TESTS_INSTRUTIONS_OPCODE_TEST_H
#define VERIFICATION_TESTS_INSTRUTIONS_OPCODE_TEST_H

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

class VerifyOpcodeTest : public VerifierTest {
public:
    VerifyOpcodeTest()
    {
        config_ = verifier::NewConfig();
        service_ = CreateService(config_, Runtime::GetCurrent()->GetInternalAllocator(),
                                 ark::Runtime::GetCurrent()->GetClassLinker(), "");
    }

    ~VerifyOpcodeTest() override
    {
        DestroyService(service_, false);
        DestroyConfig(config_);
    }

    NO_COPY_SEMANTIC(VerifyOpcodeTest);
    NO_MOVE_SEMANTIC(VerifyOpcodeTest);

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
        return klass->GetDirectMethod(utf::CStringAsMutf8(methodName.c_str()));
    }

private:
    verifier::Config *config_;
    verifier::Service *service_;
};

}  // namespace ark::verifier::test

#endif
