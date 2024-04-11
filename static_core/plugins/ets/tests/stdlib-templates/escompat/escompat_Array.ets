/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for item in escompat_Array %}
/*---
desc:  {function: Test_escompat_Array }
---*/


function main(): int {
    let failures: int = 0;

    failures += check((): int => { return testReduce()}, "Test reduce");
    failures += check((): int => { return testReduceWithStart()}, "Test reduce with start value");

    failures += check((): int => { return testReduceRight()}, "Test reduce backward");
    failures += check((): int => { return testReduceRightWithStart()}, "Test reduce backward with start value");

    failures += check((): int => { return testToSorted()}, "Test toSorted()");
    failures += check((): int => { return testFindLastIndex()}, "Test findLastIndex method");
    failures += check((): int => { return testFindLastIndexMissed()}, "Test findLastIndex method on missed data");
    failures += check((): int => { return testPushEmpty()}, "Test push() method with empty Array<Object>");
    failures += check((): int => { return testPush()}, "Test push() method");

    failures += check((): int => { return testMap0()}, "Test create new Array<Object> object apply given function");
    failures += check((): int => { return testEvery()}, "Test Array<Object> every() method");
    failures += check((): int => { return testSome()}, "Test Array<Object> some() method");
    failures += check((): int => { return testSomeNegative()}, "Test Array<Object> some() method");

    failures += check((): int => { return testReverse()}, "Test Array<Object> reverse function");
    failures += check((): int => { return testToReversed()}, "Test Arrat toReversed function");

    failures += check((): int => { return testCreateFrom()}, "Test Create Array<Object> object from array");
    failures += check((): int => { return testCreateFrom2()}, "Test Create Array<Object> object from array with given function");

    failures += check((): int => { return testGoodNegIndex()}, "Test good negative index");
    failures += check((): int => { return testGoodPosIndex()}, "Test good positive index");

    failures += check((): int => { return testBadNegIndex()}, "Test bad negative index");
    failures += check((): int => { return testBadPosIndex()}, "Test bad positive index");

    failures += check((): int => { return testFill0()}, "Test Array<Object> fill(0) method");
    failures += check((): int => { return testFill3()}, "Test Array<Object> fill(3) method");
    failures += check((): int => { return testConcat()}, "Test Array<Object> concatenate method");

    failures += check((): int => { return testJoinEmpty()}, "Test join(0) method with empty source array");
    failures += check((): int => { return testJoin0()}, "Test join(0) method");
    failures += check((): int => { return testJoin1()}, "Test join(1) method");

    failures += check((): int => { return testSlice0()}, "Test slice(0) method");
    failures += check((): int => { return testSlice1()}, "Test slice(1) method");
    failures += check((): int => { return testSlice2()}, "Test slice(2) method");

    failures += check((): int => { return testFind()}, "Test find method");
    failures += check((): int => { return testFindMissData()}, "Test find missed data");
    failures += check((): int => { return testFindIndex()}, "Test find index method");
    failures += check((): int => { return testFindMissIndex()}, "Test find missed index method");

    failures += check((): int => { return testFindStatic()}, "Test static find method");
    failures += check((): int => { return testFindMissDataStatic()}, "Test static find missed data");
    failures += check((): int => { return testFindIndexStatic()}, "Test static find method");
    failures += check((): int => { return testFindMissIndex()}, "Test static find missed index method");

    failures += check((): int => { return testFindLast()}, "Test find backward");
    failures += check((): int => { return testFindLastMissed()}, "Test find backward missed");
    failures += check((): int => { return testFindLastStatic()}, "Test static find backward");
    failures += check((): int => { return testFindLastMissedStatic()}, "Test static find backward missed");

    failures += check((): int => { return testShift()}, "Test shift() method");
    failures += check((): int => { return testShiftEmpty()}, "Test shift() method with empty Array<Object>");

    failures += check((): int => { return testPop()}, "Test pop() method");
    failures += check((): int => { return testPopEmpty()}, "Test pop() method");

    failures += check((): int => { return testIncludes()}, "Test includes() method");
    failures += check((): int => { return testIncludesInEmpty()}, "Test includes() method with emptyArray");

    failures += check((): int => { return testIndexOf()}, "Test indexOf method");
    failures += check((): int => { return testIndexOfMissed()}, "Test indexOf method with missed data");
    failures += check((): int => { return testIndexOfFromEmpty()}, "Test indexOf method with empty array");

    // failures += check((): int => { return testFlatEmpty()}, "Test flat empty");
    // failures += check((): int => { return testFlatFlat()}, "Test flat non empty flat array");
    // failures += check((): int => { return testFlatNonFlat1()}, "Test flat non empty non flat array 1");
    // failures += check((): int => { return testFlatNonFlat2()}, "Test flat non empty non flat array 2");

    failures += check((): int => { return testSplice1()}, "Test splice 1");
    failures += check((): int => { return testSplice2()}, "Test splice 2");
    failures += check((): int => { return testSplice3()}, "Test splice 3");

    {% for ci in copyWithin -%}
    failures += check((): int => { return array{{.ci.name}}()}, "Test262: {{.ci.name}}");
    {%+ endfor %}

    return check(failures, "All tests run");
}

const source: Object[] = {{.item.copySource}};
const source2: Object[] = {{.item.concatSource}};

const SUCCESS = 0;
const FAILURE = 1;

function testShift(): int {
    const array = Array.from<object>(source)
    let length = array.length;

    let shifted = array.shift();

    length--;
    if (array.length != length) {
        console.println("Target array length mismatch: " + array.length + " but expected" + length);
        return FAILURE;
    }

    if (shifted != source[0] as Object) {
        console.println("Unexpected value returned at shift");
        return FAILURE;
    }

    for (let i: int = 0; i < array.length; i++) {
        let j = i+1;
        if (array.at(i) == source[j]) continue;
        console.println("Data mismatch");
        return FAILURE;
    }

    return SUCCESS;
}

function testShiftEmpty(): int {
    let array: Array<Object> = new Array<Object>();
    let length = array.length;

    let result = array.shift();
    if (array.length != 0) {
        console.println("Array<Object> size mismatch");
        return FAILURE;
    }
    if (result != null) {
        console.println("Unexpected data returned at shift empty " + result);
        return FAILURE;
    }
    return SUCCESS;
}


function testPop(): int {
    const array = Array.from<object>(source)
    let length = array.length;

    let popped = array.pop();

    length--;
    if (array.length != length) {
        console.println("Target array length mismatch: " + array.length + " but expected" + length);
        return FAILURE;
    }

    if (popped != source[length as int] as Object) {
        console.println("Unexpected value returned at pop");
        return FAILURE;
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.at(i) == source[i]) continue;
        console.println("Data mismatch");
        return FAILURE;
    }
    return SUCCESS;
}


function testPopEmpty(): int {
    let array: Array<Object> = new Array<Object>();
    let length = array.length;

    let result = array.pop();
    if (array.length != 0) {
        console.println("Array<Object> size mismatch");
        return FAILURE;
    }
    if (result != null) {
        console.println("Unexpected data returned at pop empty" + result);
        return FAILURE;
    }
    return SUCCESS;
}


function testPushEmpty(): int {
    let array: Array<Object> = new Array<Object>();


    let length = array.push(source[0]);
    if (length == 1) return SUCCESS;
    console.println("Bad length after push");
    return FAILURE;
}

function testPush(): int {
    const array = Array.from<object>(source)

    let length = array.length;
    length++;

    let test = array.push(source[0]);

    if (test == length) return SUCCESS;
    console.println("Bad length after push");
    return FAILURE;
}


function testIncludes(): int {
    const array = Array.from<object>(source)

    if (array.length != source.length) {
        console.println("Unexpected target length");
        return FAILURE;
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.includes(source[i])) continue;
        console.println("Missed expected data");
        return FAILURE;
    }

    return SUCCESS;
}

function testIncludesInEmpty(): int {
    let array: Array<Object> = new Array<Object>();

    if (array.length != 0) {
        console.println("Unexpected target length");
        return FAILURE;
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.includes(source[i])) return FAILURE;
    }

    return SUCCESS;
}


function testIndexOf(): int {
    const array = Array.from<object>(source)
    let result = array.indexOf(source[3]);
    if (result == 3) return SUCCESS;

    console.println("Unexpected data returned");
    return FAILURE;

}

function testIndexOfMissed(): int {
    const array = Array.from<object>(source)
    let test: Object = {{.item.fillValue}};
    let result = array.indexOf(test);
    if (result == -1) return SUCCESS;
    console.println("Unexpected data returned");
    return FAILURE;
}


function testIndexOfFromEmpty(): int {
    let array: Array<Object> = new Array<Object>();

    let result = array.indexOf(source[3]);
    if (result == -1) return SUCCESS;
    console.println("Unexpected data returned");
    return FAILURE;
}

function testGoodPosIndex(): int {
    const array = Array.from<object>(source)
    let test = array.at(0);
    if (test == source[0]) return SUCCESS;
    console.println("Bad at() result");
    return FAILURE;
}

function testGoodNegIndex(): int {
    const array = Array.from<object>(source)
    let test = array.at(-array.length);
    if (test == source[0]) return SUCCESS;
    console.println("Bad at() result");
    return FAILURE;
}


function testBadNegIndex(): int {
    const array = Array.from<object>(source)
    let exp = array.at(-100);
    if (exp == null) return SUCCESS;
    console.println("Unexpected result on bad negative index");
    return FAILURE;
}

function testBadPosIndex(): int {
    const array = Array.from<object>(source)
    let exp = array.at(100);
    if (exp == null) return SUCCESS;
    console.println("Unexpected result on bad positive index");
    return FAILURE;
}


function checkValues(source: Array<Object>, value: Object): int {
    for (let i: int = 0; i < source.length; i++) {
        if (source.at(i) == value) continue;
        return FAILURE;
    }
    return SUCCESS;
}


function checkValues(source: Array<Object>, value: Object, start: int, end: int): int {
    for (let i: int = start; i < end; i++) {
        if (source.at(i) == value) continue;
        return FAILURE;
    }
    return SUCCESS;
}

function testCreateFrom(): int {
    let array: Array<Object> = Array.from<Object>(source);
    if (array.length != source.length) {
        console.println("Data length mismatch");
        return FAILURE;
    }
    for (let i: int = 0; i < source.length; i++) {
        if (array.at(i) ==  source[i]) continue;
        console.println("Data mismatch");
        return FAILURE;
    }
    return SUCCESS;
}

function testCreateFrom2(): int {
    const sourceArr = Array.from<object>(source)
    const array = Array.from<Object, Object>(sourceArr, (v: Object, _: number): Object => { return v; } );
    if (array.length != source.length) {
        console.println("Data length mismatch");
        return FAILURE;
    }
    for (let i: int = 0; i < source.length; i++) {
        if (array.at(i) ==  source[i]) continue;
        console.println("Data mismatch");
        return FAILURE;
    }
    return SUCCESS;
}

function testMap0(): int {
    const array = Array.from<object>(source)
    let map: Object = {{.item.fillValue}};

    let target = array.map((item: Object): {{.item.copySourceItemType}} => { return item as {{.item.copySourceItemType}}; });
    if (target.length != array.length) {
        console.println("Data length mismatch");
        return FAILURE;
    }

    return SUCCESS;
}

function testEvery(): int {
    const array = Array.from<object>(source2)
    let test: boolean = array.every((item: Object): boolean => { return item instanceof {{.item.itemType}}});
    if (test) return SUCCESS;
    console.println("Unexpected every result");
    return FAILURE;
}

function testSome(): int {
    const array = Array.from<object>(source2)
    let match: Object = source2[1];

    let test: boolean = array.some((item: Object): boolean => { return item == match});
    if (test) return SUCCESS;
    console.println("Unexpected some result");
    return FAILURE;
}

function testSomeNegative(): int {
    let array: Array<Object> = new Array<Object>(source2);
    let match: Object = {{.item.fillValue}};

    let test: boolean = array.some((item: Object): boolean => { return item == match});
    if (!test) return SUCCESS;
    console.println("Unexpected negative some result");
    return FAILURE;
}


function testFill0(): int {
    const array = Array.from<object>(source)
    array.fill({{.item.fillValue}});
    return checkValues(array, {{.item.fillValue}});
}

function testFill3(): int {
    const array = Array.from<object>(source)
    array.fill({{.item.fillValue}}, 1, 3);
    return checkValues(array, {{.item.fillValue}}, 1, 3);
}

function testConcat(): int {
    let array1: Array<Object> = new Array<Object>(source);
    let array2: Array<Object> = new Array<Object>(source2);

    let target: Array<Object> =  array1.concat(array2);
    if (target.length != array1.length * 2) {
        console.println("Expected array length mismatch");
        return FAILURE;
    }
    let tl = array1.length;

    for (let i = 0; i < tl; i++) {
        if ( target.at(i) != array1.at(i)) {
            console.println("produced array not contains this array");
            return FAILURE;
        }
        if (target.at(i+tl) != array2.at(i)) {
            console.println("produced array not contains given array");
            return FAILURE;
        }
    }

    return SUCCESS;
}

function testJoinEmpty():int {
    let empty: Object[] = [];
    let src = new Array<Object>(empty);

    let target: String = src.join();
    if (target != "") {
        console.println("Unexpected empty join result");
        return FAILURE;
    }
    return SUCCESS;
}

function testJoin0(): int {
    const src = Array.from<object>(source2)
    let target: String = src.join();

    if (target != "{{.item.joinResult0}}") {
        console.println("Unexpected join(0) result: " + target + " vs " + "{{.item.joinResult0}}");
        return FAILURE;
    }
    return SUCCESS;
}

function testJoin1(): int {
    const src = Array.from<object>(source2)
    let target: String = src.join("+");

    if (target != "{{.item.joinResult1}}") {
        console.println("Unexpected join(1) result: " + target + " vs " + "{{.item.joinResult1}}");
        return FAILURE;
    }
    return SUCCESS;
}

function testSlice0(): int {
    const src = Array.from<object>(source2)

    let sliceStart: number = 0;
    let sliceEnd = src.length;
    let length = sliceEnd - sliceStart;

    let target = src.slice();

    if (target.length != length) {
        console.println("Unexpected length change on slice(0)");
        return FAILURE;
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return FAILURE;
    }
    return SUCCESS;
}

function testSlice1(): int {
    let src = new Array<Object>(source2);

    let sliceStart: number = 0;
    let sliceEnd = src.length;
    let length = sliceEnd - sliceStart;

    let target = src.slice(sliceStart);

    if (target.length != length) {
        console.println("Unexpected length change on slice(1)");
        return FAILURE;
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return FAILURE;
    }
    return SUCCESS;
}

function testSlice2(): int {
    let src = new Array<Object>(source2);

    let sliceStart: number = 0;
    let sliceEnd = src.length;
    let length = sliceEnd - sliceStart;

    let target = src.slice(sliceStart, sliceEnd);

    if (target.length != length) {
        console.println("Unexpected length change on slice(2)");
        return FAILURE;
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return FAILURE;
    }
    return SUCCESS;
}

function testFind(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2];

    let test = src.find((item: Object): boolean => { return item == match});
    if (test == match) return SUCCESS;
    console.println("Missed expected")
    return FAILURE;
}

function testFindMissData(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let test = src.find((item: Object): boolean => { return item == match});
    if (test == null) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}

function testFindIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2];

    let index = src.findIndex((item: Object): boolean => { return item == match});
    if (index == 2) return SUCCESS;
    console.println("Missed expected index")
    return FAILURE;
}

function testFindMissIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let index = src.findIndex((item: Object): boolean => { return item == match});
    if (index == -1) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}



function testFindStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2];

    let test = src.find((item: Object): boolean => { return item == match});
    if (test == match) return SUCCESS;
    console.println("Missed expected")
    return FAILURE;
}

function testFindMissDataStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let test = src.find((item: Object): boolean => { return item == match});
    if (test == null) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}


function testFindIndexStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2];

    let index = src.findIndex((item: Object): boolean => { return item == match});
    if (index == 2) return SUCCESS;
    console.println("Missed expected index")
    return FAILURE;
}

function testFindMissIndexStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let index = src.findIndex((item: Object): boolean => { return item == match});
    if (index == -1) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}

function testFindLast(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3];

    let test = src.findLast((item: Object): boolean => { return item == match});
    if (test == match) return SUCCESS;
    console.println("Missed expected")
    return FAILURE;
}

function testFindLastStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3];

    let test = src.findLast((item: Object): boolean => { return item == match});
    if (test == match) return SUCCESS;
    console.println("Missed expected")
    return FAILURE;
}


function testFindLastIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3];

    let test = src.findLastIndex((item: Object): boolean => { return item == match});
    if (test == 3) return SUCCESS;

    return FAILURE;
}

function testFindLastIndexMissed(): int {
    const src = Array.from<object>(source)
    let match: Object = "Missed";

    let test = src.findLastIndex((item: Object): boolean => { return item == match});
    if (test == -1) return SUCCESS;

    return FAILURE;
}


function testFindLastMissed(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let test = src.findLast((item: Object): boolean => { return item == match});
    if (test == null) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}

function testFindLastMissedStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}};

    let test = src.findLast((item: Object): boolean => { return item == match});
    if (test == null) return SUCCESS;
    console.println("Found some unexpected")
    return FAILURE;
}


function testReverse(): int {
    const src = Array.from<object>(source)
    src.reverse();
    let l = src.length;

    for (let i = 0; i < l; i++) {
        if (src.at(i as int) == source[l-i-1 as int]) continue;
        console.println("Bad data after reverse");
        return FAILURE;
    }

    return SUCCESS;
}

function testToReversed(): int {
    const src = Array.from<object>(source)
    let reversed = src.toReversed();

    let l = reversed.length;

    for (let i = 0; i < l; i++) {
        if (reversed.at(i as int) == source[l-i-1 as int]) continue;
        console.println("Bad data after reverse");
        return FAILURE;
    }

    return SUCCESS;
}

function testToSorted(): int {

    let unsorted: Object[] = new Object[source.length];
    let l:int = unsorted.length;

    for (let i: int = 0; i <l; i++) {
        unsorted[i] = source[l-i-1];
    }

    const src = Array.from<object>(unsorted)

    let sorted = src.toSorted();

    for (let i: int = 0; i < src.length; i++) {
        if (sorted.at(i) == source[i]) continue;
        console.print("Bad data order");
        return FAILURE;
    }

    return SUCCESS;
}


function testReduce(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduce((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    });
    let expected: String = "{{.item.reduce}}";
    if (expected == reduce as String) return SUCCESS;
    console.println("Unexpected reduce: " + reduce);
    return FAILURE;
}

function testReduceRight(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduceRight((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    });
    let expected: String = "{{.item.reduceRight}}";
    if (expected == reduce as String) return SUCCESS;
    console.println("Unexpected reduce: " + reduce);
    return FAILURE;
}


function testReduceWithStart(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduce((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    }, "Hello");
    let expected: String = "Hello{{.item.reduce}}";
    if (expected == reduce as String) return SUCCESS;
    console.println("Unexpected reduce: " + reduce);
    return FAILURE;
}

function testReduceRightWithStart(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduceRight((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    }, "Hello");
    let expected: String = "Hello{{.item.reduceRight}}";
    if (expected == reduce as String) return SUCCESS;
    console.println("Unexpected reduce: " + reduce);
    return FAILURE;
}

function testFlatEmpty(): int {
    let arr: Array<Object> = new Array<Object>();

    let flatten = arr.flat<Object>();
    if (flatten.length == 0) return SUCCESS;
    console.println("Unexpected flatten array length");
    return FAILURE;
}

function testFlatFlat(): int {
    let arr: Array<Object> = new Array<Object>(source);

    let flatten = arr.flat<Object>();
    if (flatten.length == arr.length) return SUCCESS;
    console.println("Unexpected flatten array length");
    return FAILURE;
}

function testFlatNonFlat1(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length;

    arr.push(source);

    let flatten = arr.flat<Object>();

    if (flatten.length == length * 2) return SUCCESS;
    console.println("Unexpected flatten array length: " + flatten.length + " expected: " + length * 2);
    return FAILURE;
}

function testFlatNonFlat2(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length;

    arr.push(new Array<Object>(source));

    let flatten = arr.flat<Object>();


    if (flatten.length == length * 2) return SUCCESS;
    console.println("Unexpected flatten array length: " + flatten.length + " expected: " + length * 2);
    return FAILURE;
}

function testSplice1(): int {
    const arr = Array.from<object>(source)
    let length = arr.length;

    arr.splice(2);
    if (arr.length == 2) return SUCCESS;
    console.println("Unexected size after splice");
    return FAILURE;
}

function testSplice2(): int {
    const arr = Array.from<object>(source)
    let length = arr.length;

    arr.splice(2, 1);
    if (arr.length == 4) return SUCCESS;
    console.println("Unexected size after splice");
    return FAILURE;
}

function testSplice3(): int {
    const arr = Array.from<object>(source)
    let length = arr.length;

    arr.splice(-1);
    if (arr.length == 4) return SUCCESS;
    console.println("Unexected size after splice");
    return FAILURE;
}


{% for ci in copyWithin %}
function array{{.ci.name}}(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let target: Array<Object> = arr.copyWithin({{.ci.params|join(', ')}});

    return SUCCESS;
}
{%- endfor %}


function check(result: int, message: String): int {
    if (result == 0) {
      return SUCCESS;
    }
    console.println("\nFAILED: " + message);
    return FAILURE;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Exception catched: " + e);
        return FAILURE;
    }

    if (result == 0) {
      return SUCCESS;
    }
    console.println("\nFAILED: " + message);
    return FAILURE;
}

{%- endfor %}
