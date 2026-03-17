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
#include "runtime/tests/test_utils.h"
#include "runtime/core/core_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/handle_scope-inl.h"

namespace ark::mem {

class StringLostTest : public testing::Test {
public:
    StringLostTest() : StringLostTest(CreateDefaultOptions()) {}

    explicit StringLostTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~StringLostTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StringLostTest);
    NO_MOVE_SEMANTIC(StringLostTest);

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
};

static std::string GetTestString()
{
    static std::string s = "abcdef__";
    return s;
}

static coretypes::String *GetOrInternString(PandaVM *vm)
{
    auto s = GetTestString();
    return vm->GetStringTable()->GetOrInternString(reinterpret_cast<const uint8_t *>(s.c_str()), s.size(),
                                                   vm->GetLanguageContext());
}

TEST_F(StringLostTest, StringTableSatbLostTest)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    auto *gc = vm->GetGC();
    auto *mthread = ManagedThread::GetCurrent();

    [[maybe_unused]] ScopedManagedCodeThread mscope(mthread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hscope(mthread);

    VMHandle<coretypes::Array> href(mthread, ObjectAllocator::AllocArray(1, ClassRoot::ARRAY_STRING, true));

    bool checkEqStr;
    auto *str = [vm, &checkEqStr]() {
        // Allocate nonmovable string equal to test_string
        std::string data = GetTestString();
        auto *str = coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(data.c_str()), data.size(),
                                                       data.size(), true, vm->GetLanguageContext(), vm, false, false);
        vm->GetStringTable()->GetOrInternString(str, vm->GetLanguageContext());
        auto *check = GetOrInternString(vm);
        checkEqStr = (check == str);
        return str;
    }();
    ASSERT_TRUE(checkEqStr);

    // Push string to table
    ASSERT_NE(GetOrInternString(vm), nullptr);
    {
        OnGCPhaseFinishListener<GCPhase::GC_PHASE_MARK> listener([mthread, vm, str, &href, &checkEqStr]() {
            ScopedManagedCodeThread scope(mthread);
            auto *s = GetOrInternString(vm);
            checkEqStr = (s == str);
            href->Set(mthread, 0, s);
        });
        gc->AddListener(&listener);
        ScopedNativeCodeThread nativeScope(mthread);
        GCTask task(GCTaskCause::NATIVE_ALLOC_CAUSE);
        task.Run(*gc);
    }

    ASSERT_TRUE(checkEqStr);
    auto *savedString = href->Get<ObjectHeader *>(mthread, 0);
    ASSERT_EQ(savedString, str);
    auto *lb = ObjectToRegion(savedString)->GetLiveBitmap();
    ASSERT_NE(lb, nullptr);
    ASSERT_TRUE(lb->Test(savedString));
}

}  // namespace ark::mem
