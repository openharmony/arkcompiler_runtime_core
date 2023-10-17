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
const PrimitivesAccess = getTestClass("PrimitivesAccess");
const PrimitivesAccessStatic = getTestClass("PrimitivesAccessStatic");

let pa = new PrimitivesAccess();
let pas = PrimitivesAccessStatic;

let typelist = [
    "byte", "short", "int", "long",
    "float", "double",
    "char", "boolean"
];

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
        TestAccessorsOf(pa, tname, ...values);
        TestAccessorsOf(pas, tname, ...values);
    }

    function IntBit(exp) {
        ASSERT_TRUE(exp < 53, "IntBit overflow");
        return (1 << (exp % 30)) * (1 << (exp - exp % 30));
    }
    function TestSInt(tname, bits) {
        let msb = bits - 1;
        TestAccessors(tname, 0, 1, -1, IntBit(msb) - 1, -IntBit(msb));
    }
    function TestUInt(tname, bits) {
        let msb = bits - 1;
        TestAccessors(tname, 0, 1, IntBit(msb), IntBit(msb + 1) - 1);
    }

    TestSInt("byte", 8);
    TestSInt("short", 16);
    TestSInt("int", 32);
    TestSInt("long", 53);
    TestAccessors("float", 0, 1, 1.25, 0x1234 / 256, Infinity, NaN);
    TestAccessors("double", 0, 1, 1.33333, 0x123456789a / 256, Infinity, NaN);
    TestUInt("char", 16);
    TestAccessors("boolean", false, true);
}

{   // Test typechecks
    typelist.forEach(function (tname) {
        function check(o) {
            ASSERT_THROWS(TypeError, () => (o["f_" + tname] = undefined));
        }
        check(pa);
        check(pas);
    })
}
