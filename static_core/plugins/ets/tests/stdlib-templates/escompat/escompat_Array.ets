/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

    failures += check((): int => { return testFlatEmpty()}, "Test flat empty");
    failures += check((): int => { return testFlatFlat()}, "Test flat non empty flat array");
    failures += check((): int => { return testFlatNonFlat1()}, "Test flat non empty non flat array 1");
    failures += check((): int => { return testFlatNonFlat2()}, "Test flat non empty non flat array 2");

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


function testShift(): int {
    let array: Array<Object> = new Array<Object>(source);
    let length: int = array.length();

    let shifted = array.shift();

    length--;
    if(array.length() != length) {
        console.println("Target array length mismatch: " + array.length() + " but expected" + length);
        return 1;
    }

    if(shifted != source[0] as Object) {
        console.println("Unexpected value returned at shift");
        return 1;
    }

    for(let i: int = 0; i< array.length(); i++) {
        let j = i+1;
        if(array.at(i) == source[j]) continue;
        console.println("Data mismatch");
        return 1;
    }

    return 0;
}

function testShiftEmpty(): int {
    let array: Array<Object> = new Array<Object>();
    let length: int = array.length();

    let result = array.shift();
    if(array.length() != 0) {
        console.println("Array<Object> size mismatch");
        return 1;
    }
    if(result != null) {
        console.println("Unexpected data returned at shift empty " + result);
        return 1;
    }
    return 0;
}


function testPop(): int {
    let array: Array<Object> = new Array<Object>(source);
    let length: int = array.length();

    let popped = array.pop();

    length--;
    if(array.length() != length) {
        console.println("Target array length mismatch: " + array.length() + " but expected" + length);
        return 1;
    }

    if(popped != source[length] as Object) {
        console.println("Unexpected value returned at pop");
        return 1;
    }

    for(let i: int = 0; i< array.length(); i++) {
        if(array.at(i) == source[i]) continue;
        console.println("Data mismatch");
        return 1;
    }
    return 0;
}


function testPopEmpty(): int {
    let array: Array<Object> = new Array<Object>();
    let length: int = array.length();

    let result = array.pop();
    if(array.length() != 0) {
        console.println("Array<Object> size mismatch");
        return 1;
    }
    if(result != null) {
        console.println("Unexpected data returned at pop empty" + result);
        return 1;
    }
    return 0;
}


function testPushEmpty(): int {
    let array: Array<Object> = new Array<Object>();


    let length: int = array.push(source[0]);
    if(length == 1) return 0;
    console.println("Bad length after push");
    return 1;
}

function testPush(): int {
    let array: Array<Object> = new Array<Object>(source);

    let length: int = array.length();
    length++;

    let test: int = array.push(source[0]);

    if(test == length) return 0;
    console.println("Bad length after push");
    return 1;
}


function testIncludes(): int {
    let array: Array<Object> = new Array<Object>(source);

    if(array.length() != source.length) {
        console.println("Unexpected target length");
        return 1;
    }

    for(let i: int = 0; i< array.length(); i++) {
        if(array.includes(source[i])) continue;
        console.println("Missed expected data");
        return 1;
    }

    return 0;
}

function testIncludesInEmpty(): int {
    let array: Array<Object> = new Array<Object>();

    if(array.length() != 0) {
        console.println("Unexpected target length");
        return 1;
    }

    for(let i: int = 0; i< array.length(); i++) {
        if(array.includes(source[i])) return 1;
    }

    return 0;
}


function testIndexOf(): int {
    let array: Array<Object> = new Array<Object>(source);
    let result: int = array.indexOf(source[3]);
    if(result == 3) return 0;

    console.println("Unexpected data returned");
    return 1;

}

function testIndexOfMissed(): int {
    let array: Array<Object> = new Array<Object>(source);
    let test: Object = {{.item.fillValue}};
    let result: int = array.indexOf(test);
    if(result == -1) return 0;
    console.println("Unexpected data returned");
    return 1;
}


function testIndexOfFromEmpty(): int {
    let array: Array<Object> = new Array<Object>();

    let result: int = array.indexOf(source[3]);
    if(result == -1) return 0;
    console.println("Unexpected data returned");
    return 1;
}

function testGoodPosIndex(): int {
    let array: Array<Object> = new Array<Object>(source);

    let test = array.at(0);
    if(test == source[0]) return 0;
    console.println("Bad at() result");
    return 1;
}

function testGoodNegIndex(): int {
    let array: Array<Object> = new Array<Object>(source);

    let test = array.at(-array.length());
    if(test == source[0]) return 0;
    console.println("Bad at() result");
    return 1;
}


function testBadNegIndex(): int {
    let array: Array<Object> = new Array<Object>(source);
    let exp = array.at(-100);
    if(exp == null) return 0;
    console.println("Unexpected result on bad negative index");
    return 1;
}

function testBadPosIndex(): int {
    let array: Array<Object> = new Array<Object>(source);
    let exp = array.at(100);
    if(exp == null) return 0;
    console.println("Unexpected result on bad positive index");
    return 1;
}


function checkValues(source: Array<Object>, value: Object): int {
    for(let i: int = 0; i < source.length(); i++) {
        if(source.at(i) == value) continue;
        return 1;
    }
    return 0;
}


function checkValues(source: Array<Object>, value: Object, start: int, end: int): int {
    for(let i: int = start; i < end; i++) {
        if(source.at(i) == value) continue;
        return 1;
    }
    return 0;
}

function testCreateFrom(): int {
    let array: Array<Object> = Array.from<Object>(source);
    if(array.length() != source.length) {
        console.println("Data length mismatch");
        return 1;
    }
    for(let i: int = 0; i< source.length; i++) {
        if(array.at(i) ==  source[i]) continue;
        console.println("Data mismatch");
        return 1;
    }
    return 0;
}

function testCreateFrom2(): int {
    let array: Array<Object> = Array.from<Object, Object>(source, (v: Object): Object => { return v; } );
    if(array.length() != source.length) {
        console.println("Data length mismatch");
        return 1;
    }
    for(let i: int = 0; i< source.length; i++) {
        if(array.at(i) ==  source[i]) continue;
        console.println("Data mismatch");
        return 1;
    }
    return 0;
}

function testMap0(): int {
    let array: Array<Object> = new Array<Object>(source);
    let map: Object = {{.item.fillValue}};

    let target: Array<Int> = array.map<Int>((item: Object): Int => { return item as Int; });
    if(target.length() != array.length()) {
        console.println("Data length mismatch");
        return 1;
    }

    return 0;
}

function testEvery(): int {
    let array = new Array<Object>(source2);
    let test: boolean = array.every((item: Object): boolean => { return item instanceof {{.item.itemType}}});
    if(test) return 0;
    console.println("Unexpected every result");
    return 1;
}

function testSome(): int {
    let array: Array<Object> = new Array<Object>(source2);
    let match: Object = source2[1];

    let test: boolean = array.some((item: Object): boolean => { return item == match});
    if(test) return 0;
    console.println("Unexpected some result");
    return 1;
}

function testSomeNegative(): int {
    let array: Array<Object> = new Array<Object>(source2);
    let match: Object = {{.item.fillValue}};

    let test: boolean = array.some((item: Object): boolean => { return item == match});
    if(!test) return 0;
    console.println("Unexpected negative some result");
    return 1;
}


function testFill0(): int {
    let array: Array<Object> = new Array<Object>(source);
    array.fill({{.item.fillValue}});
    return checkValues(array, {{.item.fillValue}});
}

function testFill3(): int {
    let array: Array<Object> = new Array<Object>(source);
    array.fill({{.item.fillValue}}, 1, 3);
    return checkValues(array, {{.item.fillValue}}, 1, 3);
}

function testConcat(): int {
    let array1: Array<Object> = new Array<Object>(source);
    let array2: Array<Object> = new Array<Object>(source2);

    let target: Array<Object> =  array1.concat(array2);
    if(target.length() != array1.length() * 2) {
        console.println("Expected array length mismatch");
        return 1;
    }
    let tl: int = array1.length();

    for(let i = 0; i < tl; i++) {
        if( target.at(i) != array1.at(i)) {
            console.println("produced array not contains this array");
            return 1;
        }
        if(target.at(i+tl) != array2.at(i)) {
            console.println("produced array not contains given array");
            return 1;
        }
    }

    return 0;
}

function testJoinEmpty():int {
    let empty: Object[] = [];
    let src = new Array<Object>(empty);

    let target: String = src.join();
    if(target != "") {
        console.println("Unexpected empty join result");
        return 1;
    }
    return 0;
}

function testJoin0(): int {
    let src = new Array<Object>(source2);
    let target: String = src.join();

    if(target != "{{.item.joinResult0}}") {
        console.println("Unexpected join(0) result: " + target + " vs " + "{{.item.joinResult0}}");
        return 1;
    }
    return 0;
}

function testJoin1(): int {
    let src = new Array<Object>(source2);
    let target: String = src.join("+");

    if(target != "{{.item.joinResult1}}") {
        console.println("Unexpected join(1) result: " + target + " vs " + "{{.item.joinResult1}}");
        return 1;
    }
    return 0;
}

function testSlice0(): int {
    let src = new Array<Object>(source2);

    let sliceStart: int = 0;
    let sliceEnd  : int = src.length();
    let length: int = sliceEnd - sliceStart;

    let target = src.slice();

    if(target.length() != length) {
        console.println("Unexpected length change on slice(0)");
        return 1;
    }


    for(let i: int = sliceStart; i < sliceEnd; i++) {
        if(target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return 1;
    }
    return 0;
}

function testSlice1(): int {
    let src = new Array<Object>(source2);

    let sliceStart: int = 0;
    let sliceEnd  : int = src.length();
    let length: int = sliceEnd - sliceStart;

    let target = src.slice(sliceStart);

    if(target.length() != length) {
        console.println("Unexpected length change on slice(1)");
        return 1;
    }


    for(let i: int = sliceStart; i < sliceEnd; i++) {
        if(target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return 1;
    }
    return 0;
}

function testSlice2(): int {
    let src = new Array<Object>(source2);

    let sliceStart: int = 0;
    let sliceEnd  : int = src.length();
    let length: int = sliceEnd - sliceStart;

    let target = src.slice(sliceStart, sliceEnd);

    if(target.length() != length) {
        console.println("Unexpected length change on slice(2)");
        return 1;
    }


    for(let i: int = sliceStart; i < sliceEnd; i++) {
        if(target.at(i) == src.at(i)) continue;
        console.println("Bad slice copy");
        return 1;
    }
    return 0;
}

function testFind(): int {
    let src = new Array<Object>(source);
    let match: Object = source[2];

    let test = src.find((item: Object): boolean => { return item == match});
    if(test == match) return 0;
    console.println("Missed expected")
    return 1;
}

function testFindMissData(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let test = src.find((item: Object): boolean => { return item == match});
    if(test == null) return 0;
    console.println("Found some unexpected")
    return 1;
}

function testFindIndex(): int {
    let src = new Array<Object>(source);
    let match: Object = source[2];

    let index: int = src.findIndex((item: Object): boolean => { return item == match});
    if(index == 2) return 0;
    console.println("Missed expected index")
    return 1;
}

function testFindMissIndex(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let index: int = src.findIndex((item: Object): boolean => { return item == match});
    if(index == -1) return 0;
    console.println("Found some unexpected")
    return 1;
}



function testFindStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = source[2];

    let test = src.find((item: Object): boolean => { return item == match});
    if(test == match) return 0;
    console.println("Missed expected")
    return 1;
}

function testFindMissDataStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let test = src.find((item: Object): boolean => { return item == match});
    if(test == null) return 0;
    console.println("Found some unexpected")
    return 1;
}


function testFindIndexStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = source[2];

    let index: int = src.findIndex((item: Object): boolean => { return item == match});
    if(index == 2) return 0;
    console.println("Missed expected index")
    return 1;
}

function testFindMissIndexStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let index: int = src.findIndex((item: Object): boolean => { return item == match});
    if(index == -1) return 0;
    console.println("Found some unexpected")
    return 1;
}

function testFindLast(): int {
    let src = new Array<Object>(source);
    let match: Object = source[3];

    let test = src.findLast((item: Object): boolean => { return item == match});
    if(test == match) return 0;
    console.println("Missed expected")
    return 1;

}

function testFindLastStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = source[3];

    let test = src.findLast((item: Object): boolean => { return item == match});
    if(test == match) return 0;
    console.println("Missed expected")
    return 1;

}


function testFindLastIndex(): int {
    let src = new Array<Object>(source);
    let match: Object = source[3];

    let test: int = src.findLastIndex((item: Object): boolean => { return item == match});
    if(test == 3) return 0;

    return 1;

}

function testFindLastIndexMissed(): int {
    let src = new Array<Object>(source);
    let match: Object = "Missed";

    let test: int = src.findLastIndex((item: Object): boolean => { return item == match});
    if(test == -1) return 0;

    return 1;

}


function testFindLastMissed(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let test = src.findLast((item: Object): boolean => { return item == match});
    if(test == null) return 0;
    console.println("Found some unexpected")
    return 1;
}

function testFindLastMissedStatic(): int {
    let src = new Array<Object>(source);
    let match: Object = {{.item.fillValue}};

    let test = src.findLast((item: Object): boolean => { return item == match});
    if(test == null) return 0;
    console.println("Found some unexpected")
    return 1;
}


function testReverse(): int {
    let src = new Array<Object>(source);
    src.reverse();
    let l:int = src.length();

    for(let i: int = 0; i< l; i++) {
        if(src.at(i) == source[l-i-1]) continue;
        console.println("Bad data after reverse");
        return 1;
    }

    return 0;
}

function testToReversed(): int {
    let src = new Array<Object>(source);
    let reversed = src.toReversed();

    let l:int = reversed.length();

    for(let i: int = 0; i< l; i++) {
        if(reversed.at(i) == source[l-i-1]) continue;
        console.println("Bad data after reverse");
        return 1;
    }

    return 0;
}

function testToSorted(): int {

    let unsorted: Object[] = new Object[source.length];
    let l:int = unsorted.length;

    for(let i: int = 0; i<l; i++) {
        unsorted[i] = source[l-i-1];
    }

    let src = new Array<Object>(unsorted);

    let sorted = src.toSorted();

    for(let i: int = 0; i< src.length(); i++) {
        if(sorted.at(i) == source[i]) continue;
        console.print("Bad data order");
        return 1;
    }

    return 0;
}


function testReduce(): int {
    let arr: Array<Object> = new Array<Object>(source2);

    let reduce = arr.reduce<Object>((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    });
    let expected: String = "{{.item.reduce}}";
    if(expected == reduce as String) return 0;
    console.println("Unexpected reduce: " + reduce);
    return 1;
}

function testReduceRight(): int {
    let arr: Array<Object> = new Array<Object>(source2);

    let reduce = arr.reduce<Object>((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    });
    let expected: String = "{{.item.reduceRight}}";
    if(expected == reduce as String) return 0;
    console.println("Unexpected reduce: " + reduce);
    return 1;
}


function testReduceWithStart(): int {
    let arr: Array<Object> = new Array<Object>(source2);

    let reduce = arr.reduce<Object>((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    }, "Hello");
    let expected: String = "Hello{{.item.reduce}}";
    if(expected == reduce as String) return 0;
    console.println("Unexpected reduce: " + reduce);
    return 1;
}

function testReduceRightWithStart(): int {
    let arr: Array<Object> = new Array<Object>(source2);

    let reduce = arr.reduce<Object>((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString();
        tmp += val.toString();
        return tmp;
    }, "Hello");
    let expected: String = "Hello{{.item.reduceRight}}";
    if(expected == reduce as String) return 0;
    console.println("Unexpected reduce: " + reduce);
    return 1;
}

function testFlatEmpty(): int {
    let arr: Array<Object> = new Array<Object>();

    let flatten = arr.flat();
    if(flatten.length() == 0) return 0;
    console.println("Unexpected flatten array length");
    return 1;
}

function testFlatFlat(): int {
    let arr: Array<Object> = new Array<Object>(source);

    let flatten = arr.flat();
    if(flatten.length() == arr.length()) return 0;
    console.println("Unexpected flatten array length");
    return 1;
}

function testFlatNonFlat1(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length();

    arr.push(source);

    let flatten = arr.flat();

    if(flatten.length() == length * 2) return 0;
    console.println("Unexpected flatten array length: " + flatten.length() + " expected: " + length * 2);
    return 1;
}

function testFlatNonFlat2(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length();

    arr.push(new Array<Object>(source));

    let flatten = arr.flat();


    if(flatten.length() == length * 2) return 0;
    console.println("Unexpected flatten array length: " + flatten.length() + " expected: " + length * 2);
    return 1;
}

function testSplice1(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length();

    arr.splice(2);
    if(arr.length() == 2) return 0;
    console.println("Unexected size after splice");
    return 1;
}

function testSplice2(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length();

    arr.splice(2, 1);
    if(arr.length() == 4) return 0;
    console.println("Unexected size after splice");
    return 1;
}

function testSplice3(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let length = arr.length();

    arr.splice(-1);
    if(arr.length() == 4) return 0;
    console.println("Unexected size after splice");
    return 1;
}


{% for ci in copyWithin %}
function array{{.ci.name}}(): int {
    let arr: Array<Object> = new Array<Object>(source);
    let target: Array<Object> = arr.copyWithin({{.ci.params|join(', ')}});

    return 0;
}
{%- endfor %}


function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Exception catched: " + e);
        return 1;
    }

    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

{%- endfor %}
