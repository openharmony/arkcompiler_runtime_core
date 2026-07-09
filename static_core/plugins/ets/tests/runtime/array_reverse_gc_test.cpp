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
#include <vector>

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/region_space.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"

#if defined(ARK_USE_COMMON_RUNTIME)
#include "runtime/mem/gc/cmc/cmc-gc.h"
#include "runtime/mem/gc/cmc/common_components/heap/collector/region_rset.h"
#endif

namespace ark::ets::intrinsics {
extern "C" PANDA_PUBLIC_API void EtsStdCoreArrayReverse(ark::ObjectHeader *bufferHeader, int32_t length);
}  // namespace ark::ets::intrinsics

namespace ark::test {

class ArrayReverseGCTestBase : public testing::Test {
public:
    explicit ArrayReverseGCTestBase(const char *gcType) : gcType_(gcType) {}

    void SetUp() override
    {
        RuntimeOptions options;
        options.SetCompilerEnableJit(false);
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetGcType(gcType_);
        options.SetLoadRuntimes({"ets"});
        options.SetRunGcInPlace(true);
        options.SetGcTriggerType("debug-never");
        options.SetG1EnableConcurrentUpdateRemset(false);

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});
        Runtime::Create(options);
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    ets::EtsObject *AllocString(size_t length, ets::PandaEtsVM *vm, bool movable = true)
    {
        PandaVector<uint8_t> data(length);
        return ets::EtsObject::FromCoreType(coretypes::String::CreateFromMUtf8(
            data.data(), length, length, true, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS),
            vm, movable, false));
    }

    ets::EtsArray *AllocObjArr(size_t length, ets::PandaEtsVM *vm, bool nonMovable = false)
    {
        const uint8_t *descriptor = vm->GetLanguageContext().GetStringArrayClassDescriptor();
        ets::EtsClass *klass = vm->GetClassLinker()->GetClass(reinterpret_cast<const char *>(descriptor));
        if (klass == nullptr) {
            std::cerr << "Failed to get klass of array of strings" << std::endl;
            std::abort();
        }
        SpaceType spaceType = nonMovable ? SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT : SpaceType::SPACE_TYPE_OBJECT;
        return ets::EtsArray::FromEtsObject(
            ets::EtsObject::FromCoreType(coretypes::Array::Create(klass->GetRuntimeClass(), length, spaceType)));
    }

    static void CheckReversedArray(ets::EtsArray *arr, const std::vector<ObjectHeader *> &origObjArray)
    {
        auto *thread = ManagedThread::GetCurrent();
        size_t length = origObjArray.size();
        for (size_t i = 0; i < length; i++) {
            ASSERT_EQ(arr->GetCoreType()->Get<ObjectHeader *>(thread, static_cast<ArraySizeT>(i)),
                      origObjArray[length - 1 - i])
                << "reverse failed at index " << i;
        }
    }

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t STRING_LEN = 10U;

private:
    const char *gcType_;
};

#if !defined(ARK_USE_COMMON_RUNTIME)
class ArrayReverseG1Test : public ArrayReverseGCTestBase {
public:
    ArrayReverseG1Test() : ArrayReverseGCTestBase("g1-gc") {}
};

TEST_F(ArrayReverseG1Test, ArrayReverseDirtyCardTable)
{
    auto *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(thread);
    auto *vm = ets::EtsCoroutine::GetCurrent()->GetPandaVM();
    auto *gc = vm->GetGC();
    mem::CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(cardTable, nullptr);

    auto *executionCtx = ets::EtsExecutionContext::GetCurrent();
    [[maybe_unused]] ets::EtsHandleScope scope(executionCtx);

    size_t cardSize = mem::CardTable::GetCardSize();

    ets::EtsHandle<ets::EtsArray> objArray(executionCtx, AllocObjArr(1, vm));
    size_t elemSize = objArray->GetElementSize();
    size_t elemPerCard = cardSize / elemSize;
    size_t arrSize = 5U * elemPerCard;
    objArray = ets::EtsHandle<ets::EtsArray>(executionCtx, AllocObjArr(arrSize, vm));
    ASSERT_TRUE(mem::ObjectToRegion(objArray->GetCoreType())->IsYoung());
    {
        ScopedNativeCodeThread nativeScope(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    ASSERT_TRUE(mem::ObjectToRegion(objArray->GetCoreType())->HasFlag(mem::IS_OLD));

    uintptr_t arrDataBegin = ToUintPtr(objArray->GetCoreType()) + coretypes::Array::GetDataOffset();
    size_t arrZeroIndex = (arrDataBegin % cardSize) / elemSize;
    // index of the first array element that starts at the start of card
    size_t cardStartIndex = (elemPerCard - arrZeroIndex) % elemPerCard;
    size_t length = cardStartIndex + 2U * elemPerCard + 1U;
    ASSERT_LT(length, arrSize) << "array of length " << arrSize << " is too small to reverse " << length << " elements";
    ASSERT_EQ((arrDataBegin + (length - 1U) * elemSize) % cardSize, 0U)
        << "last element of reversed range must be at the start of a new card";

    std::vector<ObjectHeader *> origObjArray(length);
    for (size_t i = 0; i < length; i++) {
        auto *str = AllocString(STRING_LEN, vm, true);
        ASSERT_TRUE(mem::ObjectToRegion(str->GetCoreType())->IsYoung());
        objArray->GetCoreType()->Set<ObjectHeader *, false>(thread, static_cast<ArraySizeT>(i), str->GetCoreType());
        origObjArray[i] = str->GetCoreType();
    }
    for (size_t i = 0; i < length; i++) {
        ASSERT_FALSE(cardTable->GetCardPtr(arrDataBegin + i * elemSize)->IsMarked());
    }

    ark::ets::intrinsics::EtsStdCoreArrayReverse(objArray->GetCoreType(), static_cast<int32_t>(length));

    CheckReversedArray(objArray.GetPtr(), origObjArray);

    for (size_t i = 0; i < length; i++) {
        EXPECT_TRUE(cardTable->GetCardPtr(arrDataBegin + i * elemSize)->IsMarked())
            << "element " << i << "/" << length << ": card not marked after reverse";
    }
}
#else  // ARK_USE_COMMON_RUNTIME
namespace cvm = ark::common_vm;

class ArrayReverseCMCTest : public ArrayReverseGCTestBase {
public:
    ArrayReverseCMCTest() : ArrayReverseGCTestBase("cmc-gc") {}

    static bool RegionRSetMarks(ObjectHeader *obj)
    {
        auto *region = cvm::RegionDesc::GetRegionDescAt(obj);
        bool found = false;
        region->GetRSet()->VisitAllMarkedCardBefore(
            [obj, &found](cvm::BaseObject *markedObj) { found |= ToUintPtr(markedObj) == ToUintPtr(obj); },
            region->GetRegionBase(), ToUintPtr(obj) + OBJECT_POINTER_SIZE);
        return found;
    }
};

TEST_F(ArrayReverseCMCTest, ArrayReverseDirtyRSet)
{
    auto *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(thread);
    auto *vm = ets::EtsCoroutine::GetCurrent()->GetPandaVM();

    auto *executionCtx = ets::EtsExecutionContext::GetCurrent();
    [[maybe_unused]] ets::EtsHandleScope scope(executionCtx);

    constexpr size_t ARRAY_SIZE = 10U;
    ets::EtsHandle<ets::EtsArray> objArray(executionCtx, AllocObjArr(ARRAY_SIZE, vm, true));

    std::vector<ObjectHeader *> origObjArray(ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        auto *str = AllocString(STRING_LEN, vm, true);
        objArray->GetCoreType()->Set<ObjectHeader *, false>(thread, static_cast<ArraySizeT>(i), str->GetCoreType());
        origObjArray[i] = str->GetCoreType();
    }
    ASSERT_FALSE(RegionRSetMarks(objArray->GetCoreType()));

    ark::ets::intrinsics::EtsStdCoreArrayReverse(objArray->GetCoreType(), static_cast<int32_t>(ARRAY_SIZE));

    CheckReversedArray(objArray.GetPtr(), origObjArray);

    EXPECT_TRUE(RegionRSetMarks(objArray->GetCoreType())) << "reverse did not mark the RSet card";
}

#endif  // ARK_USE_COMMON_RUNTIME

}  // namespace ark::test
