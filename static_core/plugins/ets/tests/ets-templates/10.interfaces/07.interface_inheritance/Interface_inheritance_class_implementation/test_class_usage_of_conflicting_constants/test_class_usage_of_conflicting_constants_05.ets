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
desc: Verify the conflict of the same - named constants in multi - level inheritance and interface implementation.
---*/

class GrandParentClass {
  readonly SAME_NAME_CONSTANT: number = 10;
}

class ParentClass extends GrandParentClass {}

interface MyInterface {
  readonly SAME_NAME_CONSTANT: string;
}

class OnlyMultiLevelInheritance extends ParentClass {}

class OnlyImplementInterface implements MyInterface {
  readonly SAME_NAME_CONSTANT: string = "constant - string";
}

function main(): void {
  let instanceOfInheritance = new OnlyMultiLevelInheritance();
  let instanceOfImplementation = new OnlyImplementInterface();
  assertNE((instanceOfInheritance.SAME_NAME_CONSTANT + ""), instanceOfImplementation.SAME_NAME_CONSTANT);
}