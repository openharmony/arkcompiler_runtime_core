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
desc: Verify the conflict of the same - named constants when a class extends a parent class and implements an interface.
---*/

class ParentClass {
  readonly SAME_NAME_CONSTANT: number = 100;
}

interface MyInterface {
  readonly SAME_NAME_CONSTANT: string;
}

class OnlyExtendParent extends ParentClass {}

class OnlyImplementInterface implements MyInterface {
  readonly SAME_NAME_CONSTANT: string = "test";
}

function main(): void {
  let instanceOfExtend = new OnlyExtendParent();
  let instanceOfImplement = new OnlyImplementInterface();
  assertNE((instanceOfExtend.SAME_NAME_CONSTANT + ""), instanceOfImplement.SAME_NAME_CONSTANT);
}