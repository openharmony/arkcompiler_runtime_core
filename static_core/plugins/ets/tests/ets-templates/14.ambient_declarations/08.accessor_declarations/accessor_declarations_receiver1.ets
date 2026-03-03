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
desc: Ambient Accessor with Receiver declarations - implementation.
tags: [compile-only, not-a-test]
---*/

import { ReceiverTest, ReceiverTest1, ReceiverTest2 } from './accessor_declarations_receiver1.d'

// setter and getter
export get valuePlusOne(this: ReceiverTest): int {
  return this.value + 1
}

export set valueNew(this: ReceiverTest, newValue: int) {
  this.value = newValue
}

// only getter
export get valuePlusOneGetter(this: ReceiverTest1): int {
  return this.value + 1
}

// only setter
export set valueNewSetter(this: ReceiverTest2, newValue: int) {
  this.value = newValue
}
