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
#include <iostream>
#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropInterfaceReturnsValuesJsToArkts : public EtsInteropTest {};

/* test set employing interface imported from JS as is */
TEST_F(EtsInteropInterfaceReturnsValuesJsToArkts, test_interface_returns_literal_type_imported)
{
    [[maybe_unused]] auto ret = CallJsMethod<bool>("getLiteralTypeReturn", "./index.js");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesJsToArkts, test_interface_returns_native_array_type_imported)
{
    [[maybe_unused]] auto ret = CallJsMethod<bool>("getNativeArrayReturn", "./index.js");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesJsToArkts, DISABLED_test_interface_returns_record_type_imported)
{
    [[maybe_unused]] auto ret = CallJsMethod<bool>("getRecordTypeReturn", "./index.js");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesJsToArkts, test_interface_returns_ref_type_imported)
{
    [[maybe_unused]] auto ret = CallJsMethod<bool>("getRefTypeReturn", "./index.js");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesJsToArkts, test_interface_returns_union_type_imported)
{
    [[maybe_unused]] auto ret = CallJsMethod<bool>("getUnion", "./index.js");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
