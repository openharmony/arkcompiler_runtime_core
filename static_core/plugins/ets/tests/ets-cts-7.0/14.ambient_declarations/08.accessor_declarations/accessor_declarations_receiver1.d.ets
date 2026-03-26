/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Ambient Accessor with Receiver declarations.
tags: [compile-only, not-a-test]
---*/

// getter and setter
export class ReceiverTest {
  value: int
  constructor (value: int) {
    this.value = value
  }
}

export declare get valuePlusOne(this: ReceiverTest): int

export declare set valueNew(this: ReceiverTest, newValue: int)

// only getter
export class ReceiverTest1 {
  value: int
  constructor (value: int) {
    this.value = value
  }
}

export declare get valuePlusOneGetter(this: ReceiverTest1): int

// only setter
export class ReceiverTest2 {
  value: int
  constructor (value: int) {
    this.value = value
  }
}

export declare set valueNewSetter(this: ReceiverTest2, newValue: int)
