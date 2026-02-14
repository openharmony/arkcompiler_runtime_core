/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

const stsVm = globalThis.gtest.etsVm;
const recordClass = stsVm.getClass('Lrecord/ETSGLOBAL;');


const myRecord = recordClass.myRecord;
const testNewRecordValueFromDynamic = stsVm.getFunction('Lrecord/ETSGLOBAL;', 'testNewRecordValueFromDynamic');


const mixedTypeRecord = recordClass.mixedTypeRecord;
const falsyRecord = recordClass.falsyRecord;
const largeNumberRecord = recordClass.largeNumberRecord;
const sequenceRecord = recordClass.sequenceRecord;
const modifyRecord = recordClass.modifyRecord;
const specialStringRecord = recordClass.specialStringRecord;

function testGetRecordValue(): void {
  let dayValue = myRecord.day;
  let monthValue = myRecord.month;
  let yearValue = myRecord.year;
  ASSERT_TRUE(dayValue === 199999999);
  ASSERT_TRUE(monthValue === 'two');
  ASSERT_TRUE(yearValue === 3);
}

function testChangeRecordValue(): void {
  myRecord.day = 'one';
  myRecord.month = 2;
  myRecord.year = 'three';
  let dayValue = myRecord.day;
  let monthValue = myRecord.month;
  let yearValue = myRecord.year;
  ASSERT_TRUE(dayValue === 'one');
  ASSERT_TRUE(monthValue === 2);
  ASSERT_TRUE(yearValue === 'three');
}

function testMixedTypeRecord(): void {
  let numValue = mixedTypeRecord.num;
  let strValue = mixedTypeRecord.str;
  let boolValue = mixedTypeRecord.bool;
  let nullValue = mixedTypeRecord.nullVal;
  ASSERT_TRUE(numValue === 42);
  ASSERT_TRUE(strValue === 'hello');
  ASSERT_TRUE(boolValue === true);
  ASSERT_TRUE(nullValue === null);
}

function testFalsyRecord(): void {
  let zero = falsyRecord.zero;
  let falseVal = falsyRecord.false;
  let emptyStr = falsyRecord.emptyStr;
  ASSERT_TRUE(zero === 0);
  ASSERT_TRUE(falseVal === false);
  ASSERT_TRUE(emptyStr === '');
}

function testLargeNumberRecord(): void {
  let intMax = largeNumberRecord.intMax;
  let intMin = largeNumberRecord.intMin;
  let largeNum = largeNumberRecord.largeNum;
  ASSERT_TRUE(intMax === 2147483647);
  ASSERT_TRUE(intMin === -2147483648);
  ASSERT_TRUE(largeNum === 9007199254740991);
}

function testSequentialAccess(): void {
  let sum = 0;
  sum += sequenceRecord.a;
  sum += sequenceRecord.b;
  sum += sequenceRecord.c;
  sum += sequenceRecord.d;
  sum += sequenceRecord.e;
  ASSERT_TRUE(sum === 15);
}

function testModifyRecord(): void {
  let initialX = modifyRecord.x;
  ASSERT_TRUE(initialX === 10);

  modifyRecord.x = 100;
  let newX = modifyRecord.x;
  ASSERT_TRUE(newX === 100);

  modifyRecord.y = modifyRecord.x + modifyRecord.z; 
  let newY = modifyRecord.y;
  ASSERT_TRUE(newY === 130);
}

function testSpecialStringRecord(): void {
  let empty = specialStringRecord.empty;
  let space = specialStringRecord.space;
  let special = specialStringRecord.special;
  ASSERT_TRUE(empty === '');
  ASSERT_TRUE(space === 'hello world');
  ASSERT_TRUE(special === 'a@b#c$');
}

testGetRecordValue();
testChangeRecordValue();
testNewRecordValueFromDynamic();
testMixedTypeRecord();
testFalsyRecord();
testLargeNumberRecord();
testSequentialAccess();
testModifyRecord();
testSpecialStringRecord();
