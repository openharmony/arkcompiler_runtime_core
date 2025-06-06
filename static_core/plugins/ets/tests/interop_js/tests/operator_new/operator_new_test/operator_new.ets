/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import {primitive, TestObject, testfn, ManyArgsClass,fnTestUser,SubClass,TwoArgClass,OneArgClass, NoArgClass} from "js_import"

function noArgsClass():boolean{
    const testObj = new NoArgClass;
    return testObj.message == 'Class without args'
}

function noArgsClassBrackes():boolean{
    const testObj = new NoArgClass();
    return testObj.message == 'Class without args'
}

function oneArgsClass():boolean{
    const testObj = new OneArgClass('TestName');
    return testObj.name == 'TestName'
}

function twoArgsClass():boolean{
    const testObj = new TwoArgClass('TestName','TestCity');
    return testObj.city == 'TestCity' && testObj.name == 'TestName'
}

function manyArgsClass():boolean{
    const testObj = new ManyArgsClass('TestName',30,'US','Boston');
    return testObj.showInfo() == 'Name: TestName, age:30, country: US, city: Boston'
}

function newWithSubClass():boolean{
    const testObj = new SubClass('TestName','Test surname')
    return testObj.name == 'TestName' && testObj.surname == 'Test surname'
}

function newWithException():boolean{
    const testException = new Error('Test error')
    return testException.message == 'Test error'
}
function newWithFunction():boolean{
    const testObj = new fnTestUser('Test name','Surname test')
    return testObj.name == 'Test name' && testObj.surname == 'Surname test'
}

function newWithArrey():boolean{
    let testArr:Array<String> = new Array<String>()
    testArr.push('Test')
    return testArr[0] == 'Test'
}

function newWithPrototype():boolean{
    const testObj = new fnTestUser('Fill','Smith');
    return testObj.showInfo() == 'Name: Fill, surname: Smith'
}

// Negative cases
function newWithJustFunction():boolean{
   try {
    const instance = new testfn();
    return false
   }catch(e){
    return true
   }
}

function newWithObj():boolean{
    try {
     const instance = new TestObject();
     return false
    }catch(e){
     return true
    }
 }

 function newWithPrimitive():boolean{
    try{
        const newPrimitive = new primitive()
        return false
    }catch(e){
        return true
    }
    
 }