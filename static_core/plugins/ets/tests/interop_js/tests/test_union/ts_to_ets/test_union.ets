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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/test_union/ts_to_ets/union');
let dynamicObj = module.getProperty('dynamicObj');
let TestClass = module.getProperty("TestClass");

class NewTestClass {
  prop: number = 1;
}

function testUnion() {
  let result = false;
  try {
    dynamicObj.setProperty('unionProp', ESValue.wrapNumber(123));
    assertTrue(dynamicObj.getProperty('unionProp').toNumber() === 123);

    dynamicObj.setProperty('unionProp', ESValue.wrapString('123'));
    assertTrue(dynamicObj.getProperty('unionProp').toString() === '123');

    dynamicObj.setProperty('unionProp', ESValue.wrapBoolean(true));
    assertTrue(dynamicObj.getProperty('unionProp').toBoolean() === true);

    let testInstance = TestClass.instantiate();
    dynamicObj.setProperty('unionProp', testInstance);
    assertTrue(dynamicObj.getProperty('unionProp').getProperty('name').toString() === 'aaa');

    dynamicObj.setProperty('unionProp', ESValue.wrap(new NewTestClass()));
    assertTrue(dynamicObj.getProperty('unionProp').getProperty('prop').toNumber() === 1);

    result = true;
  } catch (error) {
    assertFalse(result);
  }
  return result;
}

