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

function isRangeError(error: Error): boolean {
    return error instanceof RangeError
}

class NegativeLengthArrayLike implements ArrayLike<number> {
    private getCount: int = 0

    get length(): int {
        return -1
    }

    $_get(index: int): number {
        this.getCount++
        return index
    }

    get count(): int {
        return this.getCount
    }
}

class SingleElementArrayLike implements ArrayLike<number> {
    private getCount: int = 0

    get length(): int {
        return 1
    }

    $_get(index: int): number {
        this.getCount++
        return index + 1
    }

    get count(): int {
        return this.getCount
    }
}

function testFromNegativeArrayLikeLength(): void {
    const source = new NegativeLengthArrayLike()
    let mapCalls = 0
    const int8Array = Int8Array.from<number>(source, (value: number, index: number): number => {
        mapCalls++
        return value + index
    })
    const uint8ClampedArray = Uint8ClampedArray.from(source)

    arktest.assertEQ(int8Array.length, 0)
    arktest.assertEQ(uint8ClampedArray.length, 0)
    arktest.assertEQ(mapCalls, 0)
    arktest.assertEQ(source.count, 0)
}

function testConstructorNegativeArrayLikeLength(): void {
    const source = new NegativeLengthArrayLike()
    const iterable: Iterable<number> = source
    const int8Array = new Int8Array(iterable)
    const uint8Array = new Uint8Array(iterable)

    arktest.assertEQ(int8Array.length, 0)
    arktest.assertEQ(int8Array.byteLength, 0)
    arktest.assertEQ(uint8Array.length, 0)
    arktest.assertEQ(uint8Array.byteLength, 0)
    arktest.assertEQ(source.count, 0)
}

function testManagedSetLargeOffset(): void {
    const signedTarget = new Int8Array([10, 20, 30])
    const unsignedTarget = new Uint8Array([10, 20, 30])
    const arrayLike = new SingleElementArrayLike()
    const byteSource: FixedArray<byte> = [1]
    const intSource: FixedArray<int> = [1]
    const numberSource: FixedArray<number> = [1]

    arktest.expectThrow(() => signedTarget.set(byteSource, Int.MAX_VALUE), isRangeError)
    arktest.expectThrow(() => signedTarget.set(numberSource, Int.MAX_VALUE), isRangeError)
    arktest.expectThrow(() => signedTarget.set(arrayLike, Int.MAX_VALUE), isRangeError)
    arktest.expectThrow(() => unsignedTarget.set(intSource, Int.MAX_VALUE), isRangeError)
    arktest.expectThrow(() => unsignedTarget.set(numberSource, Int.MAX_VALUE), isRangeError)
    arktest.expectThrow(() => unsignedTarget.set(arrayLike, Int.MAX_VALUE), isRangeError)

    arktest.assertEQ(signedTarget.toString(), '10,20,30')
    arktest.assertEQ(unsignedTarget.toString(), '10,20,30')
    arktest.assertEQ(arrayLike.count, 0)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray ArrayLike length tests')
    suite.addTest('from negative ArrayLike length', testFromNegativeArrayLikeLength)
    suite.addTest('constructor negative ArrayLike length', testConstructorNegativeArrayLikeLength)
    suite.addTest('managed set large offset', testManagedSetLargeOffset)
    return suite.run()
}
