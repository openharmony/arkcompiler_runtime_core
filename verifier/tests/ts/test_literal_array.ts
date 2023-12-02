/*
 Copyright (c) 2023 Huawei Device Co., Ltd.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 *
 http://www.apache.org/licenses/LICENSE-2.0
 *
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

class ClassB {
  bProperty: string;

  constructor(bProp: string) {
    this.bProperty = bProp;
  }

  bMethod() {
    console.log("Method from ClassB");
  }
}

class ClassA {
  classBInstance: ClassB;

  constructor() {
    this.classBInstance = new ClassB("Some value for ClassB");
  }

  aMethod() {
    console.log("Method from ClassA");
    this.classBInstance.bMethod();
  }
}

const objA = new ClassA();

objA.aMethod();

console.log(objA.classBInstance.bProperty);