/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Array access expression. The array access expression's result is a type T variable
    (i.e., a variable within the array selected by the value of that index expression).
---*/

let arr: (Int|null)[] = [new Int(1), new Int(2), null];

function getArrElem(i: int): Object|null {
    return arr[i];
}

function main(): void {
  assertTrue( getArrElem(0) instanceof Int )
  assertTrue( getArrElem(1) instanceof Object )
  assertTrue( getArrElem(2) == null )
}
