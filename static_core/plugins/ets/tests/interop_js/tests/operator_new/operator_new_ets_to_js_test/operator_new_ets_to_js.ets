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

 
package operator_new_ets_to_js_test

export class NoArgClass {
    message:string
    constructor() {
        this.message = 'Class without args';
    }
}

export class OneArgClass {
    name:string
    constructor(name:string) {
        this.name = name;
    }
}

export class SubClass extends OneArgClass{
    surname:string;
    constructor(name:string, surname:string){
        super(name);
        this.surname = surname
    }
}

export class TwoArgClass {
    name:string;
    city:string;
    constructor(name:string, city:string) {
        this.name = name;
        this.city = city;
    }
}

export class ManyArgsClass {
    name:string;
    age:number;
    country:string;
    city:string;
    constructor(name:string, age:number, country:string, city:string) {
        this.name = name;
        this.age = age;
        this.country = country;
        this.city = city;
    }
    showInfo(){
        return `Name: ${this.name}, age:${this.age}, country: ${this.country}, city: ${this.city}`
    }
}


function GCJSRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assertTrue(false,  "Unexpected exception during GC");
    }
}