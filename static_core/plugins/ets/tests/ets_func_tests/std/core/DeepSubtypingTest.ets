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

type F<T> = FixedArray<T>;
type F4<T> = FixedArray<FixedArray<FixedArray<FixedArray<T>>>>;
type F10<T> = FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<FixedArray<T>>>>>>>>>>;
type F50<T> = F10<F10<F10<F10<F10<T>>>>>;
type F254<T> = F4<F50<F50<F50<F50<F50<T>>>>>>;
type F255<T> = F<F254<T>>;
type F256<T> = F<F255<T>>;
type F257<T> = F<F256<T>>;

function checkDeepSubtypingPositive() {
    const s254: F254<string> = [];
    const s255: F255<string> = [];
    const s256: F256<string> = [];
    const s257: F257<string> = [];
    const o254: F254<object> = [];
    const o255: F255<object> = [];
    const o256: F256<object> = [];
    const o257: F257<object> = [];

    arktest.assertTrue(Class.of(s254).isSubtypeOf(Class.of(o254)),
    'Error: type of the 254 times nested FixedArray of string must be a subtype of the 254 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s255).isSubtypeOf(Class.of(o254)),
    'Error: type of the 255 times nested FixedArray of string must be a subtype of the 254 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s256).isSubtypeOf(Class.of(o254)),
    'Error: type of the 256 times nested FixedArray of string must be a subtype of the 254 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s257).isSubtypeOf(Class.of(o254)),
    'Error: type of the 257 times nested FixedArray of string must be a subtype of the 254 times nested FixedArray of object');

    arktest.assertTrue(Class.of(s255).isSubtypeOf(Class.of(o255)),
    'Error: type of the 255 times nested FixedArray of string must be a subtype of the 255 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s256).isSubtypeOf(Class.of(o255)),
    'Error: type of the 256 times nested FixedArray of string must be a subtype of the 255 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s257).isSubtypeOf(Class.of(o255)),
    'Error: type of the 257 times nested FixedArray of string must be a subtype of the 255 times nested FixedArray of object');

    arktest.assertTrue(Class.of(s256).isSubtypeOf(Class.of(o256)),
    'Error: type of the 256 times nested FixedArray of string must be a subtype of the 256 times nested FixedArray of object');
    arktest.assertTrue(Class.of(s257).isSubtypeOf(Class.of(o256)),
    'Error: type of the 257 times nested FixedArray of string must be a subtype of the 256 times nested FixedArray of object');

    arktest.assertTrue(Class.of(s257).isSubtypeOf(Class.of(o257)),
    'Error: type of the 257 times nested FixedArray of string must be a subtype of the 257 times nested FixedArray of object');

    arktest.assertTrue(Class.of(s257).isSubtypeOf(Class.of(s257)),
    'Error: type of the 257 times nested FixedArray of string must be a subtype of the 257 times nested FixedArray of string');
    arktest.assertTrue(Class.of(o257).isSubtypeOf(Class.of(o256)),
    'Error: type of the 257 times nested FixedArray of object must be a subtype of the 256 times nested FixedArray of object');
}

function checkDeepSubtypingNegative () {
    const s254: F254<string> = [];
    const s255: F255<string> = [];
    const s256: F256<string> = [];
    const o254: F254<object> = [];
    const o255: F255<object> = [];
    const o256: F256<object> = [];
    const o257: F257<object> = [];

    arktest.assertFalse(Class.of(o254).isSubtypeOf(Class.of(s254)),
    'Error: type of the 254 times nested FixedArray of object must not be a subtype of the 254 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o255).isSubtypeOf(Class.of(s254)),
    'Error: type of the 255 times nested FixedArray of object must not be a subtype of the 254 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o256).isSubtypeOf(Class.of(s254)),
    'Error: type of the 256 times nested FixedArray of object must not be a subtype of the 254 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o257).isSubtypeOf(Class.of(s254)),
    'Error: type of the 257 times nested FixedArray of object must not be a subtype of the 254 times nested FixedArray of string');

    arktest.assertFalse(Class.of(o254).isSubtypeOf(Class.of(s255)),
    'Error: type of the 254 times nested FixedArray of object must not be a subtype of the 255 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o255).isSubtypeOf(Class.of(s255)),
    'Error: type of the 255 times nested FixedArray of object must not be a subtype of the 255 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o256).isSubtypeOf(Class.of(s255)),
    'Error: type of the 256 times nested FixedArray of object must not be a subtype of the 255 times nested FixedArray of string');
    arktest.assertFalse(Class.of(o257).isSubtypeOf(Class.of(s255)),
    'Error: type of the 257 times nested FixedArray of object must not be a subtype of the 255 times nested FixedArray of string');

    arktest.assertFalse(Class.of(o254).isSubtypeOf(Class.of(o255)),
    'Error: type of the 254 times nested FixedArray of object must not be a subtype of the 255 times nested FixedArray of object');
    arktest.assertFalse(Class.of(o255).isSubtypeOf(Class.of(o256)),
    'Error: type of the 255 times nested FixedArray of object must not be a subtype of the 256 times nested FixedArray of object');
    arktest.assertFalse(Class.of(o256).isSubtypeOf(Class.of(o257)),
    'Error: type of the 256 times nested FixedArray of object must not be a subtype of the 257 times nested FixedArray of object');

    arktest.assertFalse(Class.of(s254).isSubtypeOf(Class.of(o255)),
    'Error: type of the 254 times nested FixedArray of string must not be a subtype of the 255 times nested FixedArray of object');
    arktest.assertFalse(Class.of(s255).isSubtypeOf(Class.of(o256)),
    'Error: type of the 255 times nested FixedArray of string must not be a subtype of the 256 times nested FixedArray of object');
    arktest.assertFalse(Class.of(s256).isSubtypeOf(Class.of(o257)),
    'Error: type of the 256 times nested FixedArray of string must not be a subtype of the 257 times nested FixedArray of object');
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Deep Subtyping Check test");
    suite.addTest("checkDeepSubtypingPositive", checkDeepSubtypingPositive);
    suite.addTest("checkDeepSubtypingNegative", checkDeepSubtypingNegative);
    return suite.run();
}
