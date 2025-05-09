/*
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

import { DependencyClass } from './bedependent'

export class User {
    private name: string;
    private email: string;
 
    constructor(name: string, email: string) {
        this.name = name;
        this.email = email;
    }
 
    introduce(): void {
        console.log(`Hello, I'm ${this.name}, my email is ${this.email}`);
    }
}

export class AnotherClass {
    constructor() {
        console.log("AnotherClass is initialized");
    }
 
    doSomething(): void {
        const dependency = new DependencyClass();
        dependency.methodDependency();
    }
}

export function main(){
    console.log("Starting the application");
    const user = new User("John Doe", "john@example.com");
    user.introduce();
}