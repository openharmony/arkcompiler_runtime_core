/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

//! CHECKER JIT IR Builder, MyArray.get/set shall not be inlined as Intrinsics.EscompatArrayGet/Set in IRBuilder pass.
//! RUN     force_jit: true, options: "--compiler-regex='.*(test1|array[GS]et|MyArray).*'", entry: "escompat_array_get_set_derived.ETSGLOBAL::test1"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::arrayGet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::._get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::arraySet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::._set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/

//! CHECKER AOT IR Builder, MyArray.get/set shall not be inlined as escompat.Array.get/set.
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*(array[GS]et|MyArray).*'"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::arrayGet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::._get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::._get/
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::arraySet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::._set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::._set/

//! CHECKER AOT IR Builder, MyArray.get/set shall not be inlined as escompat.Array.get/set. (Output correctness check)
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: ""
//! RUN        entry: "escompat_array_get_set_derived.ETSGLOBAL::test1"

//! CHECKER JIT IR Builder, Inlining & static devirtualization as escompat.Array.get/set.
//! RUN     force_jit: true, options: "--compiler-regex='.*(test2|array[GS]et|MyArray).*'", entry: "escompat_array_get_set_derived.ETSGLOBAL::test2"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::test2"
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._get/
//! INST_COUNT /LoadArray/, 3
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._set/
//! INST_COUNT /StoreArray/, 3

//! CHECKER AOT IR Builder, Inlining & static devirtualization as escompat.Array.get/set.
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*(test2|array[GS]et|MyArray).*'"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::test2"
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._get/
//! INST_COUNT /LoadArray/, 3
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._set/
//! INST_COUNT /StoreArray/, 3
//! RUN        entry: "escompat_array_get_set_derived.ETSGLOBAL::test2"

//! CHECKER JIT IR Builder, Inlining & static devirtualization as MyArray.get/set.
//! RUN     force_jit: true, options: "--compiler-regex='.*(test3|array[GS]et|MyArray).*'", entry: "escompat_array_get_set_derived.ETSGLOBAL::test3"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::test3"
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._get/
//! INST_COUNT /LoadArray/, 3
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._set/
//! INST_COUNT /StoreArray/, 3

//! CHECKER AOT IR Builder, Inlining & static devirtualization as MyArray.get/set.
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*(test3|array[GS]et|MyArray).*'"
//! METHOD  "escompat_array_get_set_derived.ETSGLOBAL::test3"
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._get/
//! INST_COUNT /LoadArray/, 3
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST_NOT   /Call(Virtual|Static) .*Array::._set/
//! INST_COUNT /StoreArray/, 3
//! RUN        entry: "escompat_array_get_set_derived.ETSGLOBAL::test3"

let myArrayGetCounter = 0;
let myArraySetCounter = 0;

function clearCounter(): void {
    myArrayGetCounter = 0;
    myArraySetCounter = 0;
}

class MyArray<T> extends Array<T> {
    public constructor(len: number) {
		super(len);
	}

	public override $_get(index: int): T {
		myArrayGetCounter += 1;
		return super.$_get(index);
	}

	public $_set(index: int, value: T): void {
		myArraySetCounter += 1;
		super.$_set(index, value);
	}
}

function arraySet(arr: Array<int>): void {
    arr[1] = 2;
    arr[3] = 5;
    arr[8] = 13;
}

function arrayGet(arr: Array<int>): int {
    return arr[1] + arr[3] + arr[8];
}

const BASE_ARRAY_BRANCH = 0;
const DERIVED_ARRAY_BRANCH = 1;
const NUM_ARRAY_BRANCHES = 2;

function __noinline__testWithRandomBranch(branch: int): void {
    let arr: Array<int>;
    if (branch == BASE_ARRAY_BRANCH) {
        arr = new Array<int>(16);
    } else {
        arr = new MyArray<int>(16);
    }
    arraySet(arr);
    assertEQ(arrayGet(arr), 20);
    if (branch == BASE_ARRAY_BRANCH) {
        assertEQ(myArrayGetCounter, 0);
        assertEQ(myArraySetCounter, 0);
    } else {
        assertEQ(myArrayGetCounter, 3);
        assertEQ(myArraySetCounter, 3);
    }
}

function __noinline__getRandomBranch() {
    return (Date.now() % NUM_ARRAY_BRANCHES) as int;
}

function test1(): void {
    const branch = __noinline__getRandomBranch();
    __noinline__testWithRandomBranch(branch);
    clearCounter();
    __noinline__testWithRandomBranch(branch ^ 1);
}

function test2(): void {
    const arr = new Array<int>(16);
    arraySet(arr);
    assertEQ(arrayGet(arr), 20);
    assertEQ(myArrayGetCounter, 0);
    assertEQ(myArraySetCounter, 0);
}

function test3(): void {
    const arr = new MyArray<int>(16);
    arraySet(arr);
    assertEQ(arrayGet(arr), 20);
    assertEQ(myArrayGetCounter, 3);
    assertEQ(myArraySetCounter, 3);
}
