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

// classGetSuperClass
export class Animal {
  static name: string = 'Animal';
}

export class Dog extends Animal {
  static name: string = 'Dog';
}

// abstract class
export abstract class Shape {
  abstract calArea(): int;
}

export class Square extends Shape {
  length: int = 5;
  calArea(): int {
    return this.length * this.length;
  }
}

// final class
export final class City {
  static name: string = 'Shanghai';
}

// generic class
export class Container<T> {
  calArea(area: T): T {
    return area;
  }
}

// classGet/SetStaticField
export class Person {
  static name: string = 'Person';
  static age: int = 18;
  static male: boolean = true;
  static bits: byte = 0x01;
  static id: long = 1000000000;
  static grade: float = 3.14f;
  static height: double = 1.78;
  static pet: Dog = new Dog();
}

// objectGet/SetProperty
export class Child {
  isChild: boolean = true;
}
export class Student extends Child {
  name: string = 'Student';
  age: int = 18;
  male: boolean = true
  bits: byte = 0x01;
  id: long = 1000000000;
  grade: float = 3.14f;
  height: double = 1.78;
}

export namespace testNameSpace {
  export let student: Student = new Student();
  // fixedArrayGetLength
  export let zeroArray: FixedArray<int> = [];
  export let intArray: FixedArray<int> = [1, 2, 3, 4, 5];
  export let boolArray: FixedArray<boolean> = [true, false, true, false];
  export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
  export let stuArray: FixedArray<Student> = [student];
}

// enumGetIndexByName, enumGetNameByIndex, enumGetValueByIndex, enumGetIndexByValue
export enum COLOR {
  Red = 1,
  Green = 3,
  Blue = 5,
  Yellow = 7,
  White = 9,
}

export enum MEDIA {
  JSON = 'app/json',
  XML = 'app/xml',
  YAML = 'app/yaml'
}

// class and method in namespace
export namespace Language {

  export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean {
    return b1 && b2;
  }

  export class C {
    static name: string = 'c language';
  }

  export namespace DynamicLanguage {

    export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean {
      return b1 || b2;
    }

    export class Javascript {
      static name: string = 'javascript language';
    }

    export namespace Python {
      export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean {
        return b1 && b2;
      }
    }
  }
}

/*
 *
 */
// Lets_namespace.ets
export namespace magicNamespace {
  export let magic_int_n: int = 42;
  export let magic_boolean_n: boolean = true;
  export let magic_byte_n: byte = 72;
  export let magic_short_n: short = 128;
  export let magic_long_n: long = 1024;
  export let magic_float_n: float = 3.14f as float;
  export let magic_double_n: double = 3.141592;
}