/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/mem/object_helpers.h"

#include "test_utils.h"

namespace ark::mem {

class IntrusiveGCTestApiTest : public testing::TestWithParam<const char *> {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType(GetParam());
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);

        Runtime::Create(options);
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }
};

TEST_P(IntrusiveGCTestApiTest, TestMarkQueue)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();
    GC *gc = vm->GetGC();
    ScopedManagedCodeThread s(thread);
    ObjectAllocator objectAllocator;
    ObjectHeader *object = objectAllocator.AllocObjectInYoung();
    vm->MarkObject(object);
    size_t count = 0;
    bool found = false;
    vm->IterateOverMarkQueue([&count, &found, object](ObjectHeader *obj) {
        ++count;
        if (object == obj) {
            found = true;
        }
    });
    ASSERT_EQ(1U, count);
    ASSERT_TRUE(found);

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> objHandle(thread, object);
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    object = objHandle.GetPtr();  // GC may move the object
    found = false;
    vm->IterateOverMarkQueue([&found, object](ObjectHeader *obj) {
        if (object == obj) {
            found = true;
        }
    });
    ASSERT_TRUE(found);

    vm->ClearMarkQueue();
    count = 0;
    vm->IterateOverMarkQueue([&count]([[maybe_unused]] ObjectHeader *obj) { ++count; });
    ASSERT_EQ(0, count);
}

INSTANTIATE_TEST_SUITE_P(IntrusiveGCTestAPITestSuite, IntrusiveGCTestApiTest,
                         testing::Values("stw", "gen-gc", "g1-gc"));

}  // namespace ark::mem
