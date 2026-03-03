/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::ani::testing {

class WeakReferenceDeleteTest : public AniTest {};

TEST_F(WeakReferenceDeleteTest, from_null_ref)
{
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(nullRef, &wref), ANI_OK);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
}

TEST_F(WeakReferenceDeleteTest, from_undefined_ref)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
}

TEST_F(WeakReferenceDeleteTest, from_object_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
}

TEST_F(WeakReferenceDeleteTest, from_object_gref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_ref objectGRef;
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &objectGRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectGRef, &wref), ANI_OK);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
}

TEST_F(WeakReferenceDeleteTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);
    ASSERT_EQ(env_->c_api->WeakReference_Delete(nullptr, wref), ANI_INVALID_ARGS);
}

TEST_F(WeakReferenceDeleteTest, weak_reference_delete_under_pending_error)
{
    std::string longString(10000U, 'a');
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(longString.c_str(), longString.size(), &strRef), ANI_OK);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(strRef, &wref), ANI_OK);

    ani_ref anyStringRef {};
    ASSERT_EQ(env_->Any_New(strRef, 0U, nullptr, &anyStringRef), ANI_PENDING_ERROR);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(readability-magic-numbers)
