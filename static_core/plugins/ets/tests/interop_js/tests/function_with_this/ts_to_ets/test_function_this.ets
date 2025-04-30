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

let module = ESObject.load('../../plugins/ets/tests/interop_js/tests/function_with_this/ts_to_ets/function_this')

function testBaseClass() {
    let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
    assertTrue(obj.typeOf() === 'object')
    assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    return true
}

function testChildClass() {
    let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
    assertTrue(obj.typeOf() === 'object')
    assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    assertTrue(obj.getProperty('childVal').toNumber() === 2)
    return true
}

function testBaseFunc() {
    let obj = module.getProperty('baseFunc').invoke()
    assertTrue(ESObject.areStrictlyEqual(obj, ESObject.Undefined))
    return true
}

function testChildFunc() {
    let obj = module.getProperty('childFunc').invoke()
    assertTrue(ESObject.areStrictlyEqual(obj, ESObject.Undefined))
    return true
}

function testFooBaseObjInvoke() {
    let fooBaseObj = module.getProperty('fooBaseObj')
    let result = fooBaseObj.getProperty('foo').invoke()
    assertTrue(ESObject.areStrictlyEqual(result, ESObject.Undefined))
    return true
}

function testFooBaseObjInvokeMethod() {
    let fooBaseObj = module.getProperty('fooBaseObj')
    let result = fooBaseObj.invokeMethod('fooFunc')
    assertTrue(ESObject.areStrictlyEqual(result, fooBaseObj))
    return true
}

function testFooChildObjInvoke() {
    let fooChildObj = module.getProperty('fooChildObj')
    let result = fooChildObj.getProperty('foo').invoke()
    assertTrue(ESObject.areStrictlyEqual(result, ESObject.Undefined))
    return true
}

function testFooChildObjInvokeMethod() {
    let fooChildObj = module.getProperty('fooChildObj')
    let result = fooChildObj.invokeMethod('fooFunc')
    assertTrue(ESObject.areStrictlyEqual(result, fooChildObj))
    return true
}

function testArrowFuncInvoke() {
    let result = module.getProperty('arrowFunc').invoke()
    assertTrue(ESObject.areStrictlyEqual(result, ESObject.Undefined))
    return true
}

