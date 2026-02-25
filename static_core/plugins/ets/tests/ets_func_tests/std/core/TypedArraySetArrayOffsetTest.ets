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

function fullTestName(arrName: string): string {
	return arrName + '.set(empty array, offset)'
}

function expectRangeError(err: Error): boolean {
	return err instanceof RangeError
}

function testInt8ArraySetWithEmptyArrayAndOffset(): void {
	const target = new Int32Array(4)
	const source = new Int32Array(0)
	arktest.expectThrow(() => {
		target.set(source, target.length + 1)
	}, expectRangeError)
}

function testUint8ArraySetWithEmptyArrayAndOffset(): void {
	const target = new Uint8Array(4)
	const source = new Uint8Array(0)
	arktest.expectThrow(() => {
		target.set(source, target.length + 1)
	}, expectRangeError)
}

function testInt16ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Int16Array(4)
    const source = new Int16Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testUint16ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Uint16Array(4)
    const source = new Uint16Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testInt32ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Int32Array(4)
    const source = new Int32Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testUint32ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Uint32Array(4)
    const source = new Uint32Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testFloat32ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Float32Array(4)
    const source = new Float32Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testFloat64ArraySetWithEmptyArrayAndOffset(): void {
    const target = new Float64Array(4)
    const source = new Float64Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testBigInt64ArraySetWithEmptyArrayAndOffset(): void {
	const target = new BigInt64Array(4)
	const source = new BigInt64Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function testBigUint64ArraySetWithEmptyArrayAndOffset(): void {
	const target = new BigUint64Array(4)
	const source = new BigUint64Array(0)
    arktest.expectThrow(() => {
        target.set(source, target.length + 1)
    }, expectRangeError)
}

function main(): int {
	const suite = new arktest.ArkTestsuite('TypedArraySetArrayOffset')
	suite.addTest(fullTestName("Int8Array"), testInt8ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Uint8Array'), testUint8ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Int16Array'), testInt16ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Uint16Array'), testUint16ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Int32Array'), testInt32ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Uint32Array'), testUint32ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Float32Array'), testFloat32ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('Float64Array'), testFloat64ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('BigInt64Array'), testBigInt64ArraySetWithEmptyArrayAndOffset)
	suite.addTest(fullTestName('BigUint64Array'), testBigUint64ArraySetWithEmptyArrayAndOffset)
	return suite.run()
}
