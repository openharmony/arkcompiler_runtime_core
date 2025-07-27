/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export function testFunction() {
    return 'testFunction';
}

export let testArray = ['testArrayElement'];
export let testInstance = new TestClass();

class TestClass {
    static testStaticMethod() {
        return 'testStaticMethod';
    }
    public static staticProp: String = 'staticProp';

    public prop: String = 'prop';
    testMethod() {
        return 'testMethod';
    }
}

export function checkAsync(ptr: bigint) {
    let w = new EAWorker(true);
    w.start();
    let cb = (): int => {
        let module = ESValue.load('./serialize_interop_tests_helpers_js');
        let method = module.getProperty('checkResult');
        try {
            method.invoke(ESValue.wrap(ptr));
        } catch (e: Error) {
            return 1;
        }
        return 0;
    }
    let p = w.run<int>(cb);
    w.join();
    let result = p.Await();
    arktest.assertEQ(result, 0);
}
