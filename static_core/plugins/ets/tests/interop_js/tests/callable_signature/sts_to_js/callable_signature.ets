
/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

package callable_signature.test;

class InvokeClass {
    static $_invoke(a: number, b: number): number { return a + b }
}

class InstantiateClass {
    name = ""
    static $_instantiate(factory: () => InstantiateClass, name: string): InstantiateClass {
        let x = factory()
        x.name = name
        return x
    }
}

function invokeMethodFromSts(a: number, b: number): number {
    return InvokeClass(a, b);
}

function instantiateMethodFromSts(name: string): InstantiateClass {
    return InstantiateClass(name);
}
