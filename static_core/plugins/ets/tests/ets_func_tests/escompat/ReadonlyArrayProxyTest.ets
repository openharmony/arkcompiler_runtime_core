/*
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

class ProxyHandler1 extends DefaultReadonlyArrayProxyHandler<string> {
    getTrapped = false

    override get(target: ReadonlyArray<string>, index: int): string {
        this.getTrapped = true
        return super.get(target, index)
    }
}

 
function testReadonlyArrayProxyOps(): void {
    const array: ReadonlyArray<string> = Array.of<string>("I", "II", "III")
    const handler = new ProxyHandler1()
    const arrayProxy = Proxy.create(array, handler)

    assertEQ(arrayProxy.length, 3, "unexpected ReadonlyArray proxy length")
    assertEQ(arrayProxy[0], "I", "ReadonlyArray proxy first element unexpected value")
    assertTrue(handler.getTrapped)
}

function main(): int {
    const suite = new ArkTestsuite("Proxy API")

    suite.addTest("ReadonlyArray proxy operations", testReadonlyArrayProxyOps)

    return suite.run()
}
