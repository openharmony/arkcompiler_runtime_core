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

// Before AOP:
class c2{
    a:string;
    constructor(str:string){
        this.a = str;
    }
}

class c1{
    field1:int =1;
    field2: number = 2.4;
    field3: boolean =true;
    field4:c2 = new c2("c2 created by c1");
}

function loadfield1() : int{
    let klass = new c1();
   return 0;
}
function loadfield2() : number{
    let klass = new c1();
   return 0.0;
}
function loadfield3() : boolean{
    let klass = new c1();
   return false;
}
function loadfield4(): c2 {
    let klass = new c1();
    return new c2("test");
}

function main() {
    console.log(loadfield1());
    console.log(loadfield2());
    console.log(loadfield3());
    console.log(loadfield4());
}

