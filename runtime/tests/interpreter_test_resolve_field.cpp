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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "assembly-parser.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/bytecode_emitter.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/value.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/tests/class_linker_test_extension.h"
#include "runtime/tests/interpreter/test_interpreter.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/hclass.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/native_pointer.h"

namespace panda::interpreter {

namespace test {

using DynClass = panda::coretypes::DynClass;
using DynObject = panda::coretypes::DynObject;

class InterpreterTestResolveField : public testing::Test {
public:
    InterpreterTestResolveField()
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

    ~InterpreterTestResolveField()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_;
};

TEST_F(InterpreterTestResolveField, ResolveField)
{
    auto pf = panda::panda_file::File::Open("../bin-gtests/pre-build/interpreter_test_resolve_field.abc");
    ASSERT_NE(pf, nullptr);

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));
    auto *extension = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    {
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R1"), &descriptor));
        ASSERT_NE(klass, nullptr);

        Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("get"));
        ASSERT_NE(method, nullptr);

        std::vector<Value> args;
        Value v = method->Invoke(ManagedThread::GetCurrent(), args.data());
        ASSERT_FALSE(ManagedThread::GetCurrent()->HasPendingException());

        auto ret = v.GetAs<int32_t>();
        ASSERT_EQ(ret, 10);
    }

    {
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R2"), &descriptor));
        ASSERT_NE(klass, nullptr);

        Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("get"));
        ASSERT_NE(method, nullptr);

        std::vector<Value> args;
        Value v = method->Invoke(ManagedThread::GetCurrent(), args.data());
        ASSERT_FALSE(ManagedThread::GetCurrent()->HasPendingException());

        auto ret = v.GetAs<int32_t>();
        ASSERT_EQ(ret, 20);
    }
}

}  // namespace test
}  // namespace panda::interpreter