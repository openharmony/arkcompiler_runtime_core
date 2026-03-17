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

#include "runtime/handle_scope.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/tests/test_utils.h"
#include "runtime/core/core_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace ark::mem {

class GlobStorLostTest : public testing::Test {
public:
    GlobStorLostTest() : GlobStorLostTest(CreateDefaultOptions()) {}

    explicit GlobStorLostTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~GlobStorLostTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(GlobStorLostTest);
    NO_MOVE_SEMANTIC(GlobStorLostTest);

    static RuntimeOptions CreateDefaultOptions()
    {
        RuntimeOptions options;
        options.SetCompilerEnableJit(false);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetGcType("g1-gc");
        options.SetLoadRuntimes({"core"});
        options.SetGcTriggerType("debug-never");
        options.SetShouldInitializeIntrinsics(false);
        return options;
    }
    static constexpr size_t STRING_LEN = 10U;
};

TEST_F(GlobStorLostTest, GlobalStorageSatbLostTest)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    auto *gc = vm->GetGC();
    auto *mthread = ManagedThread::GetCurrent();

    [[maybe_unused]] ScopedManagedCodeThread mscope(mthread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hscope(mthread);

    VMHandle<coretypes::Array> href(mthread, ObjectAllocator::AllocArray(1U, ClassRoot::ARRAY_STRING, true));
    auto *str = ObjectAllocator::AllocString(STRING_LEN, false, false);
    auto *reference = vm->GetGlobalObjectStorage()->Add(str, Reference::ObjectType::WEAK);
    ASSERT_EQ(Reference::ObjectType(ToUintPtr(reference) & 3U), Reference::ObjectType::WEAK);

    {
        OnGCPhaseFinishListener<GCPhase::GC_PHASE_MARK> listener([mthread, vm, &href, reference]() {
            ScopedManagedCodeThread scope(mthread);
            auto *referent = vm->GetGlobalObjectStorage()->Get(reference);
            href->Set(mthread, 0, referent);
        });
        gc->AddListener(&listener);
        ScopedNativeCodeThread nativeScope(mthread);
        GCTask task(GCTaskCause::NATIVE_ALLOC_CAUSE);
        task.Run(*gc);
    }

    auto *savedString = href->Get<ObjectHeader *>(mthread, 0);
    ASSERT_EQ(savedString, str);
    auto *lb = ObjectToRegion(savedString)->GetLiveBitmap();
    ASSERT_NE(lb, nullptr);
    ASSERT_TRUE(lb->Test(savedString));
}

}  // namespace ark::mem
