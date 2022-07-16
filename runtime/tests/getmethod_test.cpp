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

#include <cmath>
#include <cstdint>
#include <sstream>

#include "assembly-parser.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/bytecode_emitter.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/value.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"

namespace panda::test {

class GetMethodTest : public testing::Test {
public:
    GetMethodTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~GetMethodTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_ = nullptr;
};

TEST_F(GetMethodTest, GetMethod)
{
    pandasm::Parser p;
    PandaStringStream ss;
    ss << ".record R1 {}" << std::endl;

    // define some methods unsorted
    std::string methods_name[] = {"ab", "hello", "f1", "say", "world", "k0", "a"};
    size_t methods_num = sizeof(methods_name) / sizeof(methods_name[0]);
    for (size_t i = 0; i < methods_num; i++) {
        ss << ".function void R1." << methods_name[i] << "() {" << std::endl;
        ss << "    return.void" << std::endl;
        ss << "}" << std::endl;
    }

    auto source = ss.str();
    auto res = p.Parse(source.c_str());
    ASSERT_TRUE(res) << res.Error().message;

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr) << pandasm::AsmEmitter::GetLastError();

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;
    auto *ext = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R1"), &descriptor));
    ASSERT_NE(klass, nullptr);

    // check if methods sorted by id and name
    auto methods = klass->GetMethods();
    ASSERT_EQ(methods.size(), methods_num);
    for (size_t i = 0; i < methods_num; i++) {
        for (size_t j = i + 1; j < methods_num; j++) {
            ASSERT_TRUE(methods[i].GetFileId().GetOffset() < methods[j].GetFileId().GetOffset());
            ASSERT_TRUE(methods[i].GetName() < methods[j].GetName());
        }
    }

    // get each method by id and name
    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID)},
                        Method::Proto::RefTypeVector {});
    for (size_t i = 0; i < methods_num; i++) {
        Method *method = klass->GetClassMethod(utf::CStringAsMutf8(methods_name[i].c_str()), proto);
        ASSERT_NE(method, nullptr);
        ASSERT_EQ(method, klass->GetStaticClassMethod(method->GetFileId()));
    }
}

}  // namespace panda::test
