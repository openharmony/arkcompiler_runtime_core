/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function testBigIntToString1() {
    let n = 10n;
    let result = n.toString(undefined);
    arktest.assertEQ(result, "10")
}

function testBigIntToString2() {
    let n = -10n;
    let result = n.toString(undefined);
    arktest.assertEQ(result, "-10")
}

function testBigIntToString3() {
    let n = new bigint(0b10);
    let result = n.toString(undefined);
    arktest.assertEQ(result, "2")
}

function testBigIntToString4() {
    let n = new bigint(0x10);
    let result = n.toString(undefined);
    arktest.assertEQ(result, "16")
}

function testBigIntToString5() {
    let n = 10n;
    let r: number|undefined = undefined;
    let result = n.toString(r);
    arktest.assertEQ(result, "10")
    r = 16;
    result = n.toString(r);
    arktest.assertEQ(result, "a")
}

function testBigIntConstructor1() {
    let n = BigInt(0x0102030405060708)
    arktest.assertEQ(n, BigInt("72623859790382848"))

    const errorcheck = (e: Error | Exception) => { 
        return (e instanceof RangeError) &&
            e.message === 'the number 72623859.79038285 cannot be converted to a BigInt because it is not an integer'
        }
    arktest.expectThrow((): void => { n = BigInt(72623859.790382848) }, errorcheck)
}

function testBigIntConstructor2() {
    let n = BigInt(0x010203040506)
    arktest.assertEQ(n,  BigInt("1108152157446"))
    n = BigInt(72623859)
    arktest.assertEQ(n, BigInt("72623859"))
}

function testBigIntConstructor3() {
    let n = BigInt("72623859790382856")
    arktest.assertEQ(n.toString(), "72623859790382856")
    n = BigInt("72623859790382848")
    arktest.assertEQ(n.toString(), "72623859790382848")
}

function testBigIntConstructor4() {
    let n = BigInt(0.0)
    arktest.assertEQ(n, BigInt("0.0"))
    arktest.assertEQ(n.toString(), "0")
}

function main(): int {
	const suite = new arktest.ArkTestsuite("test bigint toString by undefined")

	suite.addTest("convert 10 to toString(undefined)", testBigIntToString1)
    suite.addTest("convert -10 to toString(undefined)", testBigIntToString2)
    suite.addTest("convert 0b10 to toString(undefined)", testBigIntToString3)
    suite.addTest("convert 0x10 to toString(undefined)", testBigIntToString4)
    suite.addTest("test union number|undefined as params for toString", testBigIntToString5)
    suite.addTest("test large numbers and decimals", testBigIntConstructor1)
    suite.addTest("test ordinary numbers", testBigIntConstructor2)
    suite.addTest("test strings", testBigIntConstructor3)
    suite.addTest("test 0.0", testBigIntConstructor4)

	return suite.run()
}