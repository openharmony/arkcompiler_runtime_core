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

class StringArrayProxyHandler extends DefaultArrayProxyHandler<string> {
    getTrapped: boolean = false
    setTrapped: boolean = false
    setLengthTrapped: boolean = false
    popTrapped: boolean = false
    spliceTrapped: boolean = false
    shiftTrapped: boolean = false

    override get(target: Array<string>, index: int): string {
        this.getTrapped = true
        return (index == 0) ? "zero" : super.get(target, index)
    }

    override set(target: Array<string>, index: int, value: string): boolean {
        this.setTrapped = true

        if (index == 2) {
            return super.set(target, index, "two")
        }

        return super.set(target, index, value)
    }

    override get(target: Array<string>, name: string): NullishType {
        return super.get(target, name)
    }

    override set(target: Array<string>, name: string, value: NullishType): boolean {
        if (name == "length") {
            this.setLengthTrapped = true
            // ignore set new length request
            return true
        }

        return super.set(target, name, value)
    }

    override invoke(target: Array<string>, method: Method, args: FixedArray<NullishType>): NullishType {
        if (method.getName() == "pop") {
            this.popTrapped = true
        } else if (method.getName() == "splice") {
            this.spliceTrapped = true
        } else if (method.getName() == "shift") {
            this.shiftTrapped = true
        }

        return super.invoke(target, method, args)
    }
}

function verifyArrayProxyOps(array: Array<string>, handler: StringArrayProxyHandler): void {
    let arrayProxy: Array<string> = Proxy.create(array, handler)

    assertEQ(arrayProxy.length, 3, "unexpected arrayProxy.length")

    assertFalse(handler.getTrapped)
    assertEQ(arrayProxy[0],  "zero")
    assertTrue(handler.getTrapped)

    assertFalse(handler.setTrapped)
    arrayProxy[2] = "3"
    assertTrue(handler.setTrapped)
    assertEQ(arrayProxy[2],  "two")

    assertFalse(handler.setLengthTrapped)
    arrayProxy.length = 2
    assertTrue(handler.setLengthTrapped)
    assertEQ(arrayProxy.length, 3, "arrayProxy.length updated")

    assertFalse(handler.popTrapped)
    const popped = arrayProxy.pop()
    assertEQ(popped, "two", "unexpected Array.pop() result")
    assertTrue(handler.popTrapped)

    arrayProxy.push("5")
    const lastElem = arrayProxy.pop()
    assertEQ(lastElem, "5", "unexpected Array.pop() after Array.push() result")

    assertFalse(handler.spliceTrapped)
    let arrayTail = arrayProxy.splice(1, 0, "2")
    assertEQ(arrayTail.length, 0, "unexpected Array.splice() result length")
    assertEQ(arrayProxy.length, 3, "unexpected array length after splice")
    assertTrue(handler.spliceTrapped)

    assertFalse(handler.shiftTrapped)
    const shifted = arrayProxy.shift()
    assertEQ(shifted, "0")
    assertEQ(arrayProxy.length, 2)
    assertTrue(handler.shiftTrapped)

    arrayTail = arrayProxy.splice(1)
    assertEQ(arrayTail.length, 1, "unexpected Array.splice() result length")
    assertEQ(arrayProxy.length, 1, "unexpected array length after splice")
}

function testArrayProxyOps(): void {
    verifyArrayProxyOps(Array.of<string>("0", "1", "2"), new StringArrayProxyHandler())
}

function main(): int {
    const suite = new ArkTestsuite("Proxy API")

    suite.addTest("Array proxy operations", testArrayProxyOps)

    return suite.run()
}