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
const TestJSSlice = ets_mod.getFunction("Array_TestJSSlice");

{   // Test JS Array<FooClass>
    TestJSSlice(new Array(new FooClass("zero"), new FooClass("one")));
}

{   // Test ETS Array<Object>
    let arr = CreateEtsSample();
    let sliced = arr.slice(1);
    ASSERT_EQ(sliced.at(0), "foo");
    ASSERT_EQ(sliced.length(), arr.length() - 1);
    // TODO(oignatenko) uncomment below after interop will be supported for this method signature
    // let sliced1 = arr.slice(1, 2);
    // ASSERT_EQ(sliced1.at(0), "foo");
}

GCJSRuntimeCleanup();
