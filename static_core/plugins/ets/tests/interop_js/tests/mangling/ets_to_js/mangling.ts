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

const etsVm = globalThis.gtest.etsVm;

function testGetClassColonSeparator() {
    const cls1 = etsVm.getClass('Lstd/core/Object;')
    const cls2 = etsVm.getClass('Lstd:core/Object;')
    ASSERT_TRUE(cls1 !== undefined)
    ASSERT_TRUE(cls1 === cls2)
}

function testGetClassNestedPackage() {
    const obj = etsVm.getClass('Lstd:core:sub/Object;')
    ASSERT_TRUE(obj === undefined)

    const obj2 = etsVm.getClass('Lstd.core:sub:Object;')
    ASSERT_TRUE(obj2 === undefined)
}

function testGetUserDefinedVar() {
    const cls1 = etsVm.getClass('Lmangling/ETSGLOBAL;')
    const cls2 = etsVm.getClass('Lmangling:ETSGLOBAL;')
    ASSERT_TRUE(cls1 !== undefined)
    ASSERT_TRUE(cls1 === cls2)

    ASSERT_EQ(cls1.x, 3)
    ASSERT_EQ(cls1.x, cls2.x)
}

testGetClassColonSeparator()
testGetClassNestedPackage()
testGetUserDefinedVar()
