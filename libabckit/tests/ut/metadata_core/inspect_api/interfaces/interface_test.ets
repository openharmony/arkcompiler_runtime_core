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

namespace MyNamespace {
    interface MyInterface {
        fieldA: string;
        readonly fieldB: number;
        bar(): void;
    }
}

interface ParentA {
  methodA(): void;
}

interface ParentB {
  methodB(): string;
}

interface ChildInterface extends ParentA, ParentB {
  childMethod(): number;
}

interface Animal {
  name: string;
  eat(): void;
}

interface Dog extends Animal {
  breed: string;
  bark(): void;
}

interface Clickable {
  disabled: boolean;
}

class MyButton implements Clickable {
  disabled: boolean = false;
}