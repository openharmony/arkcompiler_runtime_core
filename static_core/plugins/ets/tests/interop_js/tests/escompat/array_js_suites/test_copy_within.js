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

const { etsVm, getTestModule } = require("escompat.test.js")

const ets_mod = getTestModule("escompat_test");
const GCJSRuntimeCleanup = ets_mod.getFunction("GCJSRuntimeCleanup");
const FooClass = ets_mod.getClass("FooClass");
const CreateEtsSample = ets_mod.getFunction("Array_CreateEtsSample");
const TestJSCopyWithin = ets_mod.getFunction("Array_TestJSCopyWithin");

{   // Test JS Array<FooClass>
    TestJSCopyWithin(new Array(new FooClass("zero"), new FooClass("one")));
}

{   // Test ETS Array<Object>
    let arr = CreateEtsSample();
    arr.push("something");
    ASSERT_EQ(arr.at(2), "something");
    arr.copyWithin(2, 0);
    ASSERT_EQ(arr.at(2), 123);

    ASSERT_EQ(arr.at(1), "foo");
    // TODO(oignatenko) uncomment below after interop will be supported for this method signature
    // arr.copyWithin(1);
    // ASSERT_EQ(arr.at(1), 123);
}

GCJSRuntimeCleanup();