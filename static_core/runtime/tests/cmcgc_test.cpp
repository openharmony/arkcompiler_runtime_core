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

#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/thread_scopes.h"

namespace ark::mem {

namespace cvm = common_vm;

class CMCGCTest : public testing::Test {
public:
    static RuntimeOptions CreateRuntimeOptions()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("cmc-gc");
        options.SetLoadRuntimes({"ets"});
        options.SetGcTriggerType("debug-never");

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        return options;
    }

    CMCGCTest() : CMCGCTest(CreateRuntimeOptions()) {}

    explicit CMCGCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~CMCGCTest()
    {
        Runtime::Destroy();
    }
};

constexpr size_t REGION_FILL_OBJS_COUNT = 8192;
constexpr size_t TEST_ARRAY_SIZE = 16;
constexpr size_t TEST_CLASS_VTABLE_SIZE = 0;
constexpr size_t TEST_CLASS_IMT_SIZE = 0;

TEST_F(CMCGCTest, AllocEtsMovableObject)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto *vm = execCtx->GetPandaVM();
    auto *objCls = vm->GetClassLinker()->GetClassRoot(ets::EtsClassRoot::OBJECT);
    ets::EtsObject *arr = ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE)->AsObject();
    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsObject> arrHandle(execCtx, arr);
    auto *arrPtr = arrHandle.GetPtr();
    std::vector<ets::EtsObject *> arrs;
    for (int i = 0; i < REGION_FILL_OBJS_COUNT; i++) {
        arrs.push_back(ets::EtsObjectArray::Create(objCls, TEST_ARRAY_SIZE)->AsObject());
    }
    cvm::BaseRuntime::GetInstance()->RequestGC(cvm::GCReason::GC_REASON_FORCE, false, cvm::GCType::GC_TYPE_YOUNG);
    cvm::BaseRuntime::WaitForGCFinish();
    ASSERT_NE(arrPtr, arrHandle.GetPtr());
}

TEST_F(CMCGCTest, AllocateNonMovableObject)
{
    auto *execCtx = ets::EtsExecutionContext::GetCurrent();
    auto langCtx = execCtx->GetPandaVM()->GetLanguageContext();
    auto *runtime = Runtime::GetCurrent();
    auto *linkerExt = runtime->GetClassLinker()->GetExtension(langCtx);
    std::string className("Foo");
    Class *coreCls =
        linkerExt->CreateClass(reinterpret_cast<const uint8_t *>(className.data()), TEST_CLASS_VTABLE_SIZE,
                               TEST_CLASS_IMT_SIZE, AlignUp(sizeof(Class) + OBJECT_POINTER_SIZE, OBJECT_POINTER_SIZE));
    ets::EtsClass *etsCls = ets::EtsClass::FromRuntimeClass(coreCls);
    ets::EtsHandleScope scope(execCtx);
    ets::EtsHandle<ets::EtsClass> etsClsHandle(execCtx, etsCls);
    auto *etsClsPtr = etsClsHandle.GetPtr();
    cvm::BaseRuntime::GetInstance()->RequestGC(cvm::GCReason::GC_REASON_FORCE, false, cvm::GCType::GC_TYPE_YOUNG);
    cvm::BaseRuntime::WaitForGCFinish();
    ASSERT_EQ(etsClsPtr, etsClsHandle.GetPtr());
}
}  // namespace ark::mem
