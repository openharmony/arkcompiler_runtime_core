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

const { getTestClass } = require("ets_proxy.test.js");
const ReferencesAccess = getTestClass("ReferencesAccess");
const ReferencesAccessStatic = getTestClass("ReferencesAccessStatic");
const UClass1 = getTestClass("UClass1");
const UClass2 = getTestClass("UClass2");

let ra = new ReferencesAccess();
let ras = ReferencesAccessStatic;

{   // Test property accessors
    function TestAccessors(tname, ...values) {
        function TestAccessorsOf(o, tname, ...values) {
            for (let v of values) {
                o["f_" + tname] = v;
                ASSERT_EQ(o["getf_" + tname](), v);
                o["setf_" + tname](v);
                ASSERT_EQ(o["f_" + tname], v)
            }
        }
        TestAccessorsOf(ra, tname, ...values);
        TestAccessorsOf(ras, tname, ...values);
    }

    TestAccessors("UClass1", null, new UClass1());
    TestAccessors("String", null, "fooo", "0123456789abcdef_");
    TestAccessors("JSValue", null, 1234, "fooo", {}, new UClass1());
    // TestAccessors("Promise", null, new Promise(function () { }, function () { })); // TODO(konstanting): enable

    {
        let arr = ["a", "bb", "ccc"];
        ra.f_A_String = arr;
        ASSERT_EQ(JSON.stringify(ra.f_A_String), JSON.stringify(arr));
    }
    {
        let arr = [new UClass1(), new UClass1()];
        ra.f_A_UClass1 = arr;
        ASSERT_EQ(ra.f_A_UClass1[0], arr[0]);
        ASSERT_EQ(ra.f_A_UClass1[1], arr[1]);
    }
}

{   // Test typechecks
    function TestTypecheck(tname, ...values) {
        function check(o, v) {
            ASSERT_THROWS(TypeError, () => (o["f_" + tname] = v));
        }
        for (let v of values) {
            check(ra, v);
            check(ras, v);
        }
    }

    TestTypecheck("UClass1", NaN, {}, new UClass2());
    TestTypecheck("String", NaN, {}, new UClass1());
    // TestTypecheck("Promise", NaN, {}, new UClass1()); // TODO(konstanting): enable

    TestTypecheck("A_String", NaN, {}, [1], ["1", 1], [{}], [new UClass2()]);
    TestTypecheck("A_UClass1", NaN, {}, [1], ["1", 1], [{}], [new UClass2()]);
}
