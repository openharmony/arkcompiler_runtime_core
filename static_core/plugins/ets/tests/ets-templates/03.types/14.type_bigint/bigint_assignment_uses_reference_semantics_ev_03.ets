/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Bigint assignment preserves the value and reference semantics required by the specification.
test_unit: TEST-UNIT-03-0052
assertion: GAP-03-0082 / GRC-03-0108 / CI-03-14-008-02
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('bigint assignment reference semantics')
    suite.addTest('assignment retains bigint value', () => {
        let source: bigint = 12345678901234567890n
        let target: bigint = source
        let sourceObject: Object = source
        let targetObject: Object = target
        arktest.assertTrue(targetObject === sourceObject)
    })
    return suite.run()
}
