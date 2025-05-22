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

/*---
desc: Verify that a child class can access an array of Date constants from an interface.
---*/

interface DateArrayConstantInterface {
  readonly CONST: [Date, Date];
}

class DateArrayConstantChild implements DateArrayConstantInterface {
  readonly CONST: [Date, Date] = [new Date('2023-01-01'), new Date('2023-02-02')];
}

function main(): void {
  const instance = new DateArrayConstantChild();
  assertTrue(instance.CONST[0] instanceof Date);
  assertTrue(instance.CONST[1] instanceof Date);
}