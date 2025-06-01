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
desc: Test the array index expression to modification of fields in an array of reference types.
---*/

class RefType {
   field: number = 100;
}

function main(): void {
  const objects: RefType[] = [new RefType(), new RefType()];
  const obj = objects[1];
  obj.field = 110;
  assertEQ(objects[0].field, 100);
  assertEQ(objects[1].field, 110);
}