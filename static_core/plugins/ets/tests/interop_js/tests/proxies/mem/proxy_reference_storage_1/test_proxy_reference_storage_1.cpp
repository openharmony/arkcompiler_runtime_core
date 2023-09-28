/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

// NOLINTBEGIN(readability-magic-numbers,cppcoreguidelines-pro-type-cstyle-cast)
namespace panda::ets::interop::js::ets_proxy::testing {

class SharedReferenceStorage1GTest : public js::testing::EtsInteropTest {
public:
    void SetUp() override
    {
        storage_ = SharedReferenceStorage::Create();
        auto ctx = InteropCtx::Current();
        ctx->SetJSEnv(GetJsEnv());

        memset(&object_array_, 0, sizeof(object_array_));
        next_free_idx_ = 0;
    }

    EtsObject *NewEtsObject()
    {
        if (next_free_idx_ == MAX_OBJECTS) {
            std::cerr << "Out of memory" << std::endl;
            std::abort();
        }
        return EtsObject::FromCoreType(&object_array_[next_free_idx_++]);
    }

    SharedReference *CreateReference(EtsObject *ets_object)
    {
        napi_value js_obj;
        NAPI_CHECK_FATAL(napi_create_object(GetJsEnv(), &js_obj));
        SharedReference *ref = storage_->CreateETSObjectRef(InteropCtx::Current(), ets_object, js_obj);

        // Emulate wrappper usage
        ((uintptr_t *)ref)[0] = 0xcc00ff23deadbeef;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ((uintptr_t *)ref)[1] = 0xdd330047beefdead;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        return ref;
    }

    SharedReference *GetReference(void *data)
    {
        return storage_->GetReference(data);
    }

    void RemoveReference(SharedReference *ref)
    {
        return storage_->RemoveReference(ref);
    }

    bool CheckAlive(void *data)
    {
        return storage_->CheckAlive(data);
    }

protected:
    std::unique_ptr<SharedReferenceStorage> storage_ {};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    static constexpr size_t MAX_OBJECTS = 128;
    std::array<ObjectHeader, MAX_OBJECTS> object_array_ {};
    size_t next_free_idx_ {};
};

TEST_F(SharedReferenceStorage1GTest, test_0)
{
    EtsObject *ets_object = NewEtsObject();

    ASSERT_EQ(storage_->HasReference(ets_object), false);
    SharedReference *ref = CreateReference(ets_object);
    ASSERT_EQ(storage_->HasReference(ets_object), true);

    SharedReference *ref_x = storage_->GetReference(ets_object);
    SharedReference *ref_y = GetReference((void *)ref);

    ASSERT_EQ(ref, ref_x);
    ASSERT_EQ(ref, ref_y);

    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_1)
{
    EtsObject *ets_object = NewEtsObject();

    SharedReference *ref = CreateReference(ets_object);

    // Check allocated ref
    SharedReference *ref_0 = GetReference((void *)(uintptr_t(ref) + 0));
    ASSERT_EQ(ref_0, ref);

    // Check unaligned address);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 1)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 2)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 3)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 4)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 5)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 6)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 7)), false);

    // Check next unallocated reference
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + sizeof(SharedReference))), false);

    // Check virtually unmapped space
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 16 * 4096)), false);

    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_2)
{
    EtsObject *ets_object = NewEtsObject();
    SharedReference *ref = CreateReference(ets_object);

    EtsObject *ets_object_1 = NewEtsObject();
    SharedReference *ref_1 = CreateReference(ets_object_1);

    EtsObject *ets_object_2 = NewEtsObject();
    SharedReference *ref_2 = CreateReference(ets_object_2);

    SharedReference *ref_1a = GetReference((void *)ref_1);
    ASSERT_EQ(ref_1a, ref_1);

    RemoveReference(ref_1);

    // Check deallocated ref
    ASSERT_EQ(CheckAlive((void *)ref_1), false);

    // Check
    EtsObject *ets_object_4 = NewEtsObject();
    SharedReference *ref_4 = CreateReference(ets_object_4);
    ASSERT_EQ(ref_4, ref_1);

    RemoveReference(ref);
    RemoveReference(ref_2);
    RemoveReference(ref_4);

    ASSERT_EQ(CheckAlive((void *)ref), false);
    ASSERT_EQ(CheckAlive((void *)ref_2), false);
    ASSERT_EQ(CheckAlive((void *)ref_4), false);
}

}  // namespace panda::ets::interop::js::ets_proxy::testing
// NOLINTEND(readability-magic-numbers,cppcoreguidelines-pro-type-cstyle-cast)
