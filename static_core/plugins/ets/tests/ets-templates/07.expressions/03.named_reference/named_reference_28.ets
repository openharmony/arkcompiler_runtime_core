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
desc: Restricting access Interface by namespace.
files:
- ./named_reference_export6.ets
- ./named_reference_export7.ets
---*/

import {ShapeInterfaces} from './named_reference_export6.ets';
import {Circle} from './named_reference_export7.ets';

function main(): void {
  const calculableShape: ShapeInterfaces.Calculable = new Circle(5);

  const area = calculableShape.calculateArea();
  assertEQ(area, 7);
}