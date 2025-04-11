/**
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

const etsVm = globalThis.gtest.etsVm;

const etsArray = etsVm.getClass('LETSGLOBAL;').arr;
const etsArrayConcat1 = etsVm.getClass('LETSGLOBAL;').arrConcat1;
const etsArrayConcat2 = etsVm.getClass('LETSGLOBAL;').arrConcat2;
const etsArrayCopyWithin1 = etsVm.getClass('LETSGLOBAL;').arrayCopyWithin1;
const etsArrayCopyWithin2 = etsVm.getClass('LETSGLOBAL;').arrayCopyWithin2;
const etsArrayCopyWithin3 = etsVm.getClass('LETSGLOBAL;').arrayCopyWithin3;
const etsArrayFill = etsVm.getClass('LETSGLOBAL;').arrayFill;
const etsArrayFlat = etsVm.getClass('LETSGLOBAL;').arrayFlat;
const etsArrayIndex = etsVm.getClass('LETSGLOBAL;').arrayIndex;
const etsArrayPop = etsVm.getClass('LETSGLOBAL;').arrayPop;
const etsArrayPush = etsVm.getClass('LETSGLOBAL;').arrayPush;
const etsArrayReverse = etsVm.getClass('LETSGLOBAL;').arrayReverse;
const etsArrayShift = etsVm.getClass('LETSGLOBAL;').arrayShift;
const etsArraySlice = etsVm.getClass('LETSGLOBAL;').arraySlice;
const etsArraySort = etsVm.getClass('LETSGLOBAL;').arraySort;
const etsArraySplice1 = etsVm.getClass('LETSGLOBAL;').arraySplice1;
const etsArraySplice3 = etsVm.getClass('LETSGLOBAL;').arraySplice3;
const etsArrayToSorted = etsVm.getClass('LETSGLOBAL;').arrayToSorted;
const etsArrayUnshift = etsVm.getClass('LETSGLOBAL;').arrayUnshift;
const etsArrayWith = etsVm.getClass('LETSGLOBAL;').arrayWith;

let localArray: Array<Number> = new Array<Number>(1, 2, 3, 4);
let i = 0;
const funcPredicate1 = (num: number, index: number, array: Array<Number>): boolean => num % 2 === 0;
const funcPredicate2 = (num: number, index: number, array: Array<Number>): boolean => num < 6;
const flatMapAndFlatten = (num: number, k: number, arr: Array<Number>): Number[] => [num * num];
const mapAndFlatten = (num: number, k: number, arr: Array<Number>): number => num * num;
const sumReducer = (previousValue: number, currentValue: number, index: number, array: Array<Number>): number => {
    return previousValue + currentValue;
};
const concatenateReducer = (previousValue: string, currentValue: string, index: number, array: Array<string>): string => {
    return previousValue + '' + currentValue;
};

ASSERT_TRUE(typeof etsArray === 'object');
ASSERT_TRUE(etsArray instanceof Array);
ASSERT_TRUE(etsArray[0] === 1 && etsArray[1] === 2 && etsArray[2] === 3 && etsArray[3] === 4);
ASSERT_TRUE(etsArray.length === 4);
ASSERT_TRUE(etsArray.at(1) === 2) && etsArray.at(-1) === 4;
ASSERT_TRUE(etsArrayConcat1.concat(etsArrayConcat2).toString() === '1,2,3,4,5,6,7,8' &&
            etsArrayConcat1.concat(localArray).toString() === '1,2,3,4,1,2,3,4' &&
            localArray.concat(etsArrayConcat1).toString() === '1,2,3,4,1,2,3,4');
ASSERT_TRUE(etsArrayCopyWithin1.copyWithin(1).toString() === '1,1,2,3' &&
            etsArrayCopyWithin2.copyWithin(0, 2).toString() === '3,4,3,4' &&
            etsArrayCopyWithin3.copyWithin(0, 1, 3).toString() === '2,3,3,4');
ASSERT_TRUE(etsArrayFill.fill(8, 1, 3).toString() === '1,8,8,4');
ASSERT_TRUE(etsArrayFlat.flat().toString() === '1,2,3,4,5,6');
ASSERT_TRUE(etsArray.includes(3) && !(etsArray.includes(7)));
ASSERT_TRUE(etsArrayIndex.indexOf(1) === 0 && etsArrayIndex.indexOf(2) === 2);
ASSERT_TRUE(etsArray.join('') === '1234');
ASSERT_TRUE(etsArrayIndex.lastIndexOf(2) === 3 && etsArrayIndex.lastIndexOf(2, 2) === 2);
ASSERT_TRUE(etsArrayPop.pop() === 4);
etsArrayPush.push(5);
ASSERT_TRUE(etsArrayPush.pop() === 5);
ASSERT_TRUE(etsArrayReverse.reverse().toString() === '4,3,2,1');
ASSERT_TRUE(etsArrayShift.shift() === 1);
ASSERT_TRUE(etsArraySlice.slice(2).toString() === '3,4' && etsArraySlice.slice(1, 3).toString() === '2,3');
ASSERT_TRUE(etsArraySort.sort().toString() === '1,2,3,4');
ASSERT_TRUE(etsArraySplice1.splice(1).toString() === '2,3,4' && etsArraySplice3.splice(1, 1, 2).toString() === '2');
ASSERT_TRUE(etsArray.toLocaleString() === '1,2,3,4');
ASSERT_TRUE(etsArray.toReversed().toString() === '4,3,2,1');
ASSERT_TRUE(etsArrayToSorted.toSorted().toString() === '1,2,3,4');
ASSERT_TRUE(etsArray.toSpliced(1).toString() === '1' &&
            etsArray.toSpliced(1, 2).toString() === '1,4' &&
            etsArray.toSpliced(1, 2, 3).toString() === '1,3,4');
ASSERT_TRUE(etsArrayUnshift.unshift() === 4 && etsArrayUnshift.unshift(1) === 5 &&
            etsArrayUnshift.unshift(1, 2) === 7 && etsArrayUnshift.unshift(1, 2, 3) === 10);
ASSERT_TRUE(etsArrayWith.with(1, 9).toString() === '1,9,3,4');