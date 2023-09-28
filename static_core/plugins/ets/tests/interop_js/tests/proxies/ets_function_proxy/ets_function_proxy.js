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

const {
    getTestFunction
} = require("ets_function_proxy.test.js")

{   // test simple function-proxy capabilities
    const sum_double = getTestFunction("sum_double");

    ASSERT_TRUE(Object.isSealed(sum_double));
    ASSERT_EQ(sum_double(3, 4), 3 + 4);

    ASSERT_THROWS(TypeError, () => (sum_double(1)));
    ASSERT_THROWS(TypeError, () => (sum_double(1, 2, 3, 4)));
    ASSERT_THROWS(TypeError, () => (sum_double(1, true)));
}

{   // test object-proxy
    const point_create = getTestFunction("point_create");
    const point_get_x = getTestFunction("point_get_x");
    const point_get_y = getTestFunction("point_get_y");

    let p = point_create(2, 6);
    let x = point_get_x(p);
    let y = point_get_y(p);

    ASSERT_EQ(x, 2);
    ASSERT_EQ(y, 6);
}
