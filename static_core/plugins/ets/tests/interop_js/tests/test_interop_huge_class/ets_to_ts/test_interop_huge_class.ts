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

let etsVm = globalThis.gtest.etsVm;
let HugeClass = etsVm.getClass('Ltest_interop_huge_class/HugeClass;');

let instance = new HugeClass();
ASSERT_TRUE(instance.field0 === 0);
ASSERT_TRUE(instance.method0(1) === 1);
ASSERT_TRUE(instance.field100 === 100);
ASSERT_TRUE(instance.method100(1) === 101);
ASSERT_TRUE(instance.field249 === 249);
ASSERT_TRUE(instance.method249(1) === 250);
instance.field0 = 1000;
ASSERT_TRUE(instance.field0 === 1000);
