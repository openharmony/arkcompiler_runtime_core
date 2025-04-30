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

class TestClass {
    etsNumber: number;
    etsByte: byte;
    etsShort: short;
    etsInt: int;
    etsLong: long;
    etsFloat: float;
    etsDouble: double;
      
    constructor() {
      this.etsNumber;
      this.etsByte = 123;
      this.etsShort = 123;
      this.etsInt = 123;
      this.etsLong = 123;
      this.etsFloat = 12.34;
      this.etsDouble = 12.34;
    }
}

export function getStsObj(): TestClass {
    const stsObj: TestClass = new TestClass();
    return stsObj;
}
