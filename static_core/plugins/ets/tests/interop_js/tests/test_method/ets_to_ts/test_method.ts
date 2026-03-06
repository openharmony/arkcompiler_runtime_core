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

const stsVm = globalThis.gtest.etsVm;
const student = stsVm.getClass('Lmethod/ETSGLOBAL;').student;

function testMethodEtsUseInjs(): void {
  let name = student.getName();
  ASSERT_EQ(name, 'Alice');

  let age = student.getAge();
  ASSERT_EQ(age, 20);
  return;
}

function testUndefinedCallMethodEtsUseInJS(): void {
  try {
    let func = student.pet.getName;
    func.call(undefined);
  }  catch (e) {
    ASSERT_EQ(e.message, 'ets this in instance method cannot be null or undefined');
  }
  return;
}

testMethodEtsUseInjs();
testUndefinedCallMethodEtsUseInJS();