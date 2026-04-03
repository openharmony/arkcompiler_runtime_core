/*
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


/*---
desc: |-
    Value array type is the built-in type written as ValueArray<T> and characterized by the fact:
    Any instance of array type contains elements of type T. T must be a value type
    - ArrayIndexOutOfBoundsError exception checks.
---*/


function main(): void {
    let arr: ValueArray<number> = [1, 2, 3];
    let x = -1
    
    arktest.expectThrow(() => {
        arr[x] //expect ArrayIndexOutOfBoundsError
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError;
    }); 
    
    arktest.expectThrow(() => {
        arr[arr.length] //expect ArrayIndexOutOfBoundsError
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError;
    }); 
    
}
