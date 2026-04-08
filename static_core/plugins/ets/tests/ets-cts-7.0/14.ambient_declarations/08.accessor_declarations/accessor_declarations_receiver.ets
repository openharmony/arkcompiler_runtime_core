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
desc: Ambient accessor declaration is an ambient version of Accessor Declarations or Accessors with Receiver.
files: ['./accessor_declarations_receiver1.ets']
---*/

// Note test breaks if file name is different in "accessor_declarations_receiver1.ets" and "accessor_declarations_receiver1.d"
import { ReceiverTest, valuePlusOne, valueNew, ReceiverTest1, valuePlusOneGetter, ReceiverTest2, valueNewSetter } from './accessor_declarations_receiver1.d'

// modeled after similar tests in 14.ambient_declarations/01.constant_or_variable_declarations
function main() {
    let r: ReceiverTest = new ReceiverTest(1)
    arktest.assertEQ(r.valuePlusOne, 2, "expect 2")
    r.valueNew = 3
    arktest.assertEQ(r.valuePlusOne, 4, "expect 4")

    let r1: ReceiverTest1 = new ReceiverTest1(5)
    arktest.assertEQ(r1.valuePlusOneGetter, 6, "expect 6")

    let r2: ReceiverTest2 = new ReceiverTest2(7)
    r2.valueNewSetter = 8
    arktest.assertEQ(r2.value, 8, "expect 8")
}
