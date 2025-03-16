/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package interop_class_extension

import { DEFAULT_STRING_VALUE, TestNativeClass, TestUserClass, helperIsJSInstanceOf } from ".";

// NOTE Should be commented out until #17693 is resolved
// function extendUserClass() {
//     class ExtendedUserClass extends TestUserClass {}
//     return true;
// }

// function extendNativeClass(){
//     class ExtendedNativeClass extends TestNativeClass{}
// }

// function extendedUserClassIsInstanceOf(){
//     class ExtendedUserClass extends TestUserClass{}
//     const classInstance = new ExtendedUserClass('init');
//     return TestUserClass.isMyInstance(classInstance) && classInstance instanceof TestUserClass;
// }

// function extendedNativeClassIsInstanceOf(){
//     class ExtendedNativeClass extends TestNativeClass{}
//     const classInstance = new ExtendedNativeClass();
//     return helperIsJSInstanceOf(classInstance, TestNativeClass) && classInstance instanceof TestNativeClass;
// }

// function canAccessSuperMethod(){
//     const customSuperValue: string = "this is a custom message";
//     class ExtendedUserClassWithSuperCall extends TestUserClass {
//         constructor(){
//             super(customSuperValue)
//         }
//     }
//     const classInstance = new ExtendedUserClassWithSuperCall();
//     return classInstance.constructorSetValue === customSuperValue && TestUserClass.isMyInstance(classInstance);
// }

// function canAddGetter(){
//     class ExtendedUserClassWithGetter extends TestUserClass {
//         constructor(constructorArg: string){
//             super(constructorArg)
//         }

//         get valueGetter(){
//             return this.readonlyValue;
//         }
//     }
//     const classInstance = new ExtendedUserClassWithGetter('getter');
//     return classInstance.valueGetter === DEFAULT_STRING_VALUE && TestUserClass.isMyInstance(classInstance);
// }

// function canSetProtectedValue(){
//     const redefinedProtectedValue: string = 'redefined protected value';
//     class ExtendedUserClassWithProtectedFieldRedefined extends TestUserClass{
//         constructor(constructorArg: string){
//             super(constructorArg)
//             this.protectedValue = redefinedProtectedValue;
//         }

//         get protectedValueOut(){
//             return this.protectedValue;
//         }
//     }

//     const classInstance = new ExtendedUserClassWithProtectedFieldRedefined('protected');
//     return classInstance.protectedValueOut === redefinedProtectedValue && TestUserClass.isMyInstance(classInstance);
// }

class TSTestUserClass {
    private privateProperty: string = "private";
    protected protectedProperty: string = "protected";
    public publicProperty: string = "public";
    public readonly publicPropertyRO: string = 'public_readonly';
    static staticProperty: string = "static"
}