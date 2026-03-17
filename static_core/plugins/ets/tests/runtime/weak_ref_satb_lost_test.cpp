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

#include "runtime/tests/test_utils.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/region_allocator.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::test {

class WeakRefLostTest : public testing::Test {
public:
    WeakRefLostTest()
    {
        RuntimeOptions options;
        options.SetCompilerEnableJit(false);
        options.SetShouldLoadBootPandaFiles(true);  // Need for weakrefs
        options.SetGcType("g1-gc");
        options.SetLoadRuntimes({"ets"});
        options.SetGcTriggerType("debug-never");

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }

        options.SetBootPandaFiles({stdlib});
        Runtime::Create(options);
    }
    ~WeakRefLostTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(WeakRefLostTest);
    NO_MOVE_SEMANTIC(WeakRefLostTest);

    ets::EtsObject *AllocString(size_t length, ets::PandaEtsVM *vm, bool movable = true)
    {
        PandaVector<uint8_t> data(length);
        return ets::EtsObject::FromCoreType(coretypes::String::CreateFromMUtf8(
            data.data(), length, length, true, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS),
            vm, movable, false));
    }

    ets::EtsArray *AllocObjArr(size_t length, ets::PandaEtsVM *vm)
    {
        const uint8_t *descriptor = vm->GetLanguageContext().GetStringArrayClassDescriptor();
        ets::EtsClass *klass = vm->GetClassLinker()->GetClass(reinterpret_cast<const char *>(descriptor));
        if (klass == nullptr) {
            std::cerr << "Failed to get klass of array of strings" << std::endl;
            std::abort();
        }
        return ets::EtsArray::FromEtsObject(
            ets::EtsObject::FromCoreType(coretypes::Array::Create(klass->GetRuntimeClass(), length)));
    }

    static constexpr size_t STRING_LEN = 10U;
};

TEST_F(WeakRefLostTest, WeakRefLostTest)
{
    auto *mthread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(mthread);
    auto *vm = ets::EtsCoroutine::GetCurrent()->GetPandaVM();
    auto *gc = vm->GetGC();

    auto *execContext = ets::EtsExecutionContext::GetCurrent();

    [[maybe_unused]] ets::EtsHandleScope scope(execContext);
    ets::EtsHandle<ets::EtsArray> harr(execContext, AllocObjArr(1, vm));

    auto *wrKlass = vm->GetClassLinker()->GetClass("Lstd/core/BaseWeakRef;");
    ASSERT_NE(wrKlass, nullptr);
    auto *ref = static_cast<ets::EtsWeakReference *>(ets::EtsObject::Create(execContext, wrKlass));
    ets::EtsHandle<ets::EtsObject> hwref(execContext, ref);
    auto *obj = AllocString(STRING_LEN, vm, false);
    ref->SetReferent(obj);

    {
        mem::OnGCPhaseFinishListener<mem::GCPhase::GC_PHASE_MARK> listener([mthread, &hwref, &harr]() {
            ScopedManagedCodeThread scope(mthread);
            auto *ref = static_cast<ets::EtsWeakReference *>(hwref.GetPtr());
            auto *referent = ets::EtsObject::ToCoreType(ref->GetReferent());
            harr->GetCoreType()->Set(mthread, 0, referent);
        });
        gc->AddListener(&listener);
        ScopedNativeCodeThread nativeScope(mthread);
        GCTask task(GCTaskCause::NATIVE_ALLOC_CAUSE);
        task.Run(*gc);
    }

    // If we managed to catch a error then arr is referencing dead object with first index
    auto *obj1 = harr->GetCoreType()->Get<ObjectHeader *>(mthread, 0);
    ASSERT_EQ(ToUintPtr(obj1), ToUintPtr(obj));
    auto *lb = mem::ObjectToRegion(obj1)->GetLiveBitmap();
    ASSERT_NE(lb, nullptr);
    ASSERT_TRUE(lb->Test(obj1));
}

}  // namespace ark::test
