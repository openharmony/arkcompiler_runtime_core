/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsPassingReferenceType : public EtsInteropTest {};

TEST_F(EtsPassingReferenceType, referenceAnyParam)
{
    auto ret = CallEtsMethod<bool>("referenceAnyParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceAnyWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceAnyWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceLiteralParam)
{
    auto ret = CallEtsMethod<bool>("referenceLiteralParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceLiteralWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceLiteralWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceExtraSetParam)
{
    auto ret = CallEtsMethod<bool>("referenceExtraSetParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceExtraSetWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceExtraSetWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPickParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetPickParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPickWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetPickWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceSubsetOmitParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetOmitParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetOmitWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetOmitWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPartialParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetPartialParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPartialWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceSubsetPartialWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceUnionArrParam)
{
    auto ret = CallEtsMethod<bool>("referenceUnionArrParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceUnionObjParam)
{
    auto ret = CallEtsMethod<bool>("referenceUnionObjParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceUnionWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceUnionWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceClassParam)
{
    auto ret = CallEtsMethod<bool>("referenceClassParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceClassWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceClassWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceInterfaceParam)
{
    auto ret = CallEtsMethod<bool>("referenceInterfaceParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceInterfaceWithoutParam)
{
    auto ret = CallEtsMethod<bool>("referenceInterfaceWithoutParam");
    ASSERT_EQ(ret, true);
}

// NOTE(pishin) enable after fixibng #17878
TEST_F(EtsPassingReferenceType, DISABLED_referenceObjectWithArr)
{
    auto ret = CallEtsMethod<bool>("referenceObjectWithArr");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing