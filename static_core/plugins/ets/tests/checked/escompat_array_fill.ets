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

class TestClass {
    num: int;
    public constructor(num: int) {
        this.num = num;
	}
    public get(): int {
        return this.num;
    }
}

let array_size: int = 10;
let array1: Array<int> = new Array<int>(array_size);

function check_array(arr: Array<int>, start: int, end: int, expected: int) {
    for (let i = start; i < end; i++) {
        assertEQ(arr[i], expected);
    }
}

//! CHECKER       escompat Array fill intrinsic
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_fill.ETSGLOBAL::test_array_fill"
//! METHOD        "escompat_array_fill.ETSGLOBAL::test_array_fill"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::fill/
//! INST_COUNT    /CallVirtual.*escompat.Array::fill/, 13
function test_array_fill() {
    array1.fill(99, 0, array_size);
    check_array(array1, 0, array_size, 99);

    array1.fill(11, array_size / 2 - array_size, array_size);
    check_array(array1, 0, array_size / 2, 99);
    check_array(array1, array_size / 2, array_size, 11);

    array1.fill(12, -(array_size + 2) , array_size);
    check_array(array1, 0, array_size, 12);

    array1.fill(13, array_size / 2 - array_size, array_size + 3);
    check_array(array1, 0, array_size / 2, 12);
    check_array(array1, array_size / 2, array_size, 13);

    array1.fill(14, 0, array_size + 2);
    check_array(array1, 0, array_size, 14);

    assertEQ(array1.fill(15, 0, 1)[0], 15);
    assertEQ(array1.fill(15, 0, 1)[1], 14);
    assertEQ(array1.fill(16, 1, 2)[1], 16);

    let arrNull = new Array<null>(3);
    arrNull.fill(null, 0, 3);
    for (let i = 0; i < 3; i++) {
        assertEQ(arrNull[i], null);
    }
    let arrUndef = new Array<undefined>(5);
    arrUndef.fill(undefined, 0, 5)
    for (let i = 0; i < 5; i++) {
        assertEQ(arrUndef[i], undefined);
    }
    let arrUnion = new Array<TestClass | undefined>(5);
    arrUnion.fill(undefined, 0, 5)
    arrUnion.fill(new TestClass(2), 3, 5)
    for (let i = 0; i < 5; i++) {
        if (i < 3) {
            assertEQ(arrUndef[i], undefined);
            continue
        }
        assertEQ(arrUnion[i] instanceof TestClass, true);
        let tmpObj = arrUnion[i] as TestClass
        assertEQ(tmpObj.get(), 2);
    }
    let arrObj = new Array<TestClass>(50000);
    arrObj.fill(new TestClass(3), 0, 50000);
    for (let i = 0; i < 50000; i++) {
        assertEQ(arrObj[i].get(), 3);
    }
}

class MyArray<T> extends Array<T> {
    counter: int;
    public constructor(len: number) {
		super(len);
        this.counter = 0;
	}

	public override fill(value: T, start: int, end: int): this {
        this.counter += 1;
		super.fill(value, start, end);
        return this;
	}

    public getCounter(): int {
        return this.counter;
    }
}

//! CHECKER       escompat Array test call virtual fill
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_fill.ETSGLOBAL::test_array_call_virtual_fill"
//! METHOD        "escompat_array_fill.ETSGLOBAL::test_array_call_virtual_fill"
//! PASS_AFTER    "Codegen"
//! INST          /escompat_array_fill.MyArray::fill/
//! INST_COUNT    /escompat_array_fill.MyArray::fill/, 8
function test_array_call_virtual_fill() {
    let array2: MyArray<int> = new MyArray<int>(array_size);
    array2.fill(99, 0, array_size);
    check_array(array2, 0, array_size, 99);
    assertEQ(array2.getCounter(), 1);

    array2.fill(11, array_size / 2 - array_size, array_size);
    check_array(array2, 0, array_size / 2, 99);
    check_array(array2, array_size / 2, array_size, 11);
    assertEQ(array2.getCounter(), 2);

    array2.fill(12, -(array_size + 2) , array_size);
    check_array(array2, 0, array_size, 12);
    assertEQ(array2.getCounter(), 3);

    array2.fill(13, array_size / 2 - array_size, array_size + 3);
    check_array(array2, 0, array_size / 2, 12);
    check_array(array2, array_size / 2, array_size, 13);
    assertEQ(array2.getCounter(), 4);

    array2.fill(14, 0, array_size + 2);
    check_array(array2, 0, array_size, 14);
    assertEQ(array2.getCounter(), 5);

    assertEQ(array2.fill(15, 0, 1)[0], 15);
    assertEQ(array2.getCounter(), 6);
    assertEQ(array2.fill(15, 0, 1)[1], 14);
    assertEQ(array2.getCounter(), 7);
    assertEQ(array2.fill(16, 1, 2)[1], 16);
    assertEQ(array2.getCounter(), 8);
}
