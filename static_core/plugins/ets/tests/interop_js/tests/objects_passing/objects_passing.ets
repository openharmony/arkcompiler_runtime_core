/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

// Pass ArkTS object to JS (wrapping) (ArkTS calls TS)

import {
  updateObjectId,
  getOuterObj,
  testOuterObject,
  testSecondObjCls,
  testObjCls,
  testObject,
  getName,
  getDetails,
  changeName,
  getEdu,
  getObjectId,
  getId,
  getObjectName
  } from "js_import"

//Test 1 Get public property
 function getPublicProperty():string{
    return getName(testObjCls)

 }

 // Test 2 Use public method

 function usePublicMethod():string{
  return getDetails(testObjCls)
 }

// Test 3 Change public propeerty

 function changePublicProperty():string{
  const newName = changeName(testObjCls, 'NewTestName')
  changeName(testObjCls, 'TestName')
  return newName
 }

 // Test 4 Get readonly property 

 function getReadonlyProperty():string{
  return getEdu(testObjCls)

 }


 //Test 5 Get protected property

 function getProtectedProperty():number{

  return getObjectId(testSecondObjCls)

 }

 //Test 6 Use protected method from subclass

 function useProtectedMethod():number{

  return getId(testSecondObjCls)
 }

 //Test 7 get property from object

 function getPropertyFromObject():string{

  return getObjectName(testObject)
 }

 // Test 8 Test with outer object

 function getOuterObject():number{
 
   return getOuterObj(testOuterObject)

 }

 // Test 8 Update object value

 function updateObjectValue():number{
  updateObjectId(testObject,333)
  return testObject.id
 }

