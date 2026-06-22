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

function ObjectKeysTestDate(): void {
    let date: Date = new Date()
    let keys: string[] = Object.keys(date)
    arktest.assertTrue(keys.length === 0, 'Object.keys should return 0 keys for Date')
}

class A {
    a: string = 'hello'
    private b: string = 'world'
    protected c: string = 'arkts'
}

function ObjectKeysTestObject(): void {
    let a = new A()
    let entries = Object.entries(a)
    arktest.assertTrue(entries.length === 1, 'Object.keys should return 1 key for Object')
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Object.keys tests")
    suite.addTest('Object.keys for Date', ObjectKeysTestDate);
    suite.addTest('Object.keys for Object', ObjectKeysTestObject);
    return suite.run()
}