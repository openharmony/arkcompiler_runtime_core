/*
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

class Person {
   name: string;
   constructor() {
      this.name = "string";
   }
}

export class Values {
   public static etsNum: number = 42;
   public static etsByte: byte = 123;
   public static etsShort: short = 123;
   public static etsInt: int = 123;
   public static etsLong: long = 123;
   public static etsFloat: float = 12.34;
   public static etsDouble: double = 12.34;
   public static etsBoolean: boolean = true;
   public static etsNull: null = null;
   public static etsUndefined: undefined = undefined;
   public static etsBigint: bigint = 1234567890123456789012345678901234567890n;
   public static etsString: string = "Test";
   public static etsObj: Person = new Person();
   public static etsObjCopy: Person = Values.etsObj;
   public static etsObjOther: Person = new Person();
}
