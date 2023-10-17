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

#include <gtest/gtest.h>

#include "macros.h"
#include <libpandafile/include/source_lang_enum.h>

#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_root.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"

namespace panda::ets::test {

class EtsVMTest : public testing::Test {
public:
    EtsVMTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        panda::Runtime::Create(options);
    }
    ~EtsVMTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsVMTest);
    NO_MOVE_SEMANTIC(EtsVMTest);
};

static void AssertCompoundClassRoot(EtsClassLinker *class_linker, EtsClassRoot root)
{
    EtsClass *klass = class_linker->GetClassRoot(root);
    Class *core_class = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(core_class->IsInstantiable());
    ASSERT_FALSE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_FALSE(klass->IsAbstract());
    ASSERT_FALSE(core_class->IsProxy());
    ASSERT_EQ(core_class->GetLoadContext(), class_linker->GetEtsClassLinkerExtension()->GetBootContext());

    if (root == EtsClassRoot::OBJECT) {
        ASSERT_TRUE(core_class->IsObjectClass());
        ASSERT_EQ(klass->GetBase(), nullptr);
    } else {
        ASSERT_FALSE(core_class->IsObjectClass());
        ASSERT_NE(klass->GetBase(), nullptr);
        ASSERT_EQ(klass->GetBase(), class_linker->GetClassRoot(EtsClassRoot::OBJECT));
    }
}

static void AssertCompoundContainerClassRoot(EtsClassLinker *class_linker, EtsClassRoot root)
{
    EtsClass *klass = class_linker->GetClassRoot(root);
    Class *core_class = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(core_class->IsInstantiable());
    ASSERT_TRUE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(core_class->IsProxy());
    ASSERT_EQ(core_class->GetLoadContext(), class_linker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_FALSE(core_class->IsObjectClass());
    ASSERT_NE(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetBase(), class_linker->GetClassRoot(EtsClassRoot::OBJECT));
}

static void AssertPrimitiveClassRoot(EtsClassLinker *class_linker, EtsClassRoot root)
{
    EtsClass *klass = class_linker->GetClassRoot(root);
    Class *core_class = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_FALSE(core_class->IsInstantiable());
    ASSERT_FALSE(klass->IsArrayClass());
    ASSERT_TRUE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsPublic());
    ASSERT_TRUE(klass->IsFinal());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(core_class->IsProxy());
    ASSERT_EQ(core_class->GetLoadContext(), class_linker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_EQ(klass->GetBase(), nullptr);
}

static void AssertPrimitiveContainerClassRoot(EtsClassLinker *class_linker, EtsClassRoot root)
{
    EtsClass *klass = class_linker->GetClassRoot(root);
    Class *core_class = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(core_class->IsInstantiable());
    ASSERT_TRUE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsPublic());
    ASSERT_TRUE(klass->IsFinal());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(core_class->IsProxy());
    ASSERT_EQ(core_class->GetLoadContext(), class_linker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_NE(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetBase(), class_linker->GetClassRoot(EtsClassRoot::OBJECT));
}

TEST_F(EtsVMTest, InitTest)
{
    auto runtime = Runtime::GetCurrent();
    ASSERT_NE(runtime, nullptr);

    PandaEtsVM *ets_vm = PandaEtsVM::GetCurrent();
    ASSERT_NE(ets_vm, nullptr);

    EtsClassLinker *class_linker = ets_vm->GetClassLinker();
    ASSERT_NE(class_linker, nullptr);

    EtsClassLinkerExtension *ext = class_linker->GetEtsClassLinkerExtension();
    ASSERT_NE(ext, nullptr);
    ASSERT_EQ(ext->GetLanguage(), panda_file::SourceLang::ETS);
    ASSERT_TRUE(ext->IsInitialized());

    AssertCompoundClassRoot(class_linker, EtsClassRoot::OBJECT);
    AssertCompoundClassRoot(class_linker, EtsClassRoot::CLASS);
    AssertCompoundClassRoot(class_linker, EtsClassRoot::STRING);

    AssertCompoundContainerClassRoot(class_linker, EtsClassRoot::STRING_ARRAY);

    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::BOOLEAN);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::BYTE);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::SHORT);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::CHAR);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::INT);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::LONG);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::FLOAT);
    AssertPrimitiveClassRoot(class_linker, EtsClassRoot::DOUBLE);

    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::BOOLEAN_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::BYTE_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::SHORT_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::CHAR_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::INT_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::LONG_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::FLOAT_ARRAY);
    AssertPrimitiveContainerClassRoot(class_linker, EtsClassRoot::DOUBLE_ARRAY);

    EtsCoroutine *ets_coroutine = EtsCoroutine::GetCurrent();
    ASSERT_NE(ets_coroutine, nullptr);

    auto vm = ets_coroutine->GetVM();
    ASSERT_EQ(vm->GetAssociatedThread(), ets_coroutine);

    ASSERT_NE(vm, nullptr);
    ASSERT_EQ(ext->GetLanguage(), vm->GetLanguageContext().GetLanguage());

    ASSERT_NE(vm->GetHeapManager(), nullptr);
    ASSERT_NE(vm->GetMemStats(), nullptr);

    ASSERT_NE(vm->GetGC(), nullptr);
    ASSERT_NE(vm->GetGCTrigger(), nullptr);
    ASSERT_NE(vm->GetGCStats(), nullptr);
    ASSERT_NE(vm->GetReferenceProcessor(), nullptr);

    ASSERT_NE(vm->GetGlobalObjectStorage(), nullptr);
    ASSERT_NE(vm->GetStringTable(), nullptr);

    ASSERT_NE(vm->GetMonitorPool(), nullptr);
    ASSERT_NE(vm->GetThreadManager(), nullptr);
    ASSERT_NE(vm->GetRendezvous(), nullptr);

    ASSERT_NE(vm->GetCompiler(), nullptr);
    ASSERT_NE(vm->GetCompilerRuntimeInterface(), nullptr);
}

}  // namespace panda::ets::test
