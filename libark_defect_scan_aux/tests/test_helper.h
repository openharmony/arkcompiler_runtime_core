/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef LIBARK_DEFECT_SCAN_AUX_TESTS_TEST_HELPER_H
#define LIBARK_DEFECT_SCAN_AUX_TESTS_TEST_HELPER_H

#include "libpandabase/macros.h"

namespace panda::defect_scan_aux::test {
class TestHelper {
public:
    template <typename T1, typename T2>
    inline static void ExpectEqual(const T1 &left, const T2 &right)
    {
        if (left != static_cast<T1>(right)) {
            std::cerr << "ExpectEqual failed" << std::endl;
            UNREACHABLE();
        }
    }

    template <typename T1, typename T2>
    inline static void ExpectNotEqual(const T1 &left, const T2 &right)
    {
        if (left == static_cast<T2>(right)) {
            std::cerr << "ExpectNotEqual failed" << std::endl;
            UNREACHABLE();
        }
    }

    inline static void ExpectTrue(bool val)
    {
        if (!val) {
            std::cerr << "ExpectTrue failed" << std::endl;
            UNREACHABLE();
        }
    }

    inline static void ExpectFalse(bool val)
    {
        if (val) {
            std::cerr << "ExpectFalse failed" << std::endl;
            UNREACHABLE();
        }
    }
};
}  // namespace panda::defect_scan_aux::test

#endif  // LIBARK_DEFECT_SCAN_AUX_TESTS_TEST_HELPER_H