/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite('NumericType API tests');

    suite.addTest('ByteType: primitives & reference', testByteType);
    suite.addTest('ShortType: primitives & reference', testShortType);
    suite.addTest('IntType: primitives & reference', testIntType);
    suite.addTest('LongType: primitives & reference', testLongType);
    suite.addTest('FloatType: primitives & reference', testFloatType);
    suite.addTest('DoubleType: primitives & reference', testDoubleType);
    suite.addTest('Numeric assignableFrom widening', testNumericAssignableFromAnyNumeric);

    return suite.run();
}

function testByteType() {
    const val = ByteType.VAL;
    const ref = ByteType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('byte', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Byte', ref.getLiteral());

    arktest.assertEQ(true, val.equals(ByteType.VAL));
    arktest.assertEQ(true, ref.equals(ByteType.REF));
    arktest.assertEQ(false, val.equals(ref));
    arktest.assertEQ(false, ref.equals(val));
}

function testShortType() {
    const val = ShortType.VAL;
    const ref = ShortType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('short', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Short', ref.getLiteral());

    arktest.assertEQ(true, val.equals(ShortType.VAL));
    arktest.assertEQ(true, ref.equals(ShortType.REF));
    arktest.assertEQ(false, val.equals(ref));
}

function testIntType() {
    const val = IntType.VAL;
    const ref = IntType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('int', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Int', ref.getLiteral());

    arktest.assertEQ(true, val.equals(IntType.VAL));
    arktest.assertEQ(true, ref.equals(IntType.REF));
    arktest.assertEQ(false, val.equals(ref));
}

function testLongType() {
    const val = LongType.VAL;
    const ref = LongType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('long', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Long', ref.getLiteral());

    arktest.assertEQ(true, val.equals(LongType.VAL));
    arktest.assertEQ(true, ref.equals(LongType.REF));
    arktest.assertEQ(false, val.equals(ref));
}

function testFloatType() {
    const val = FloatType.VAL;
    const ref = FloatType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('float', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Float', ref.getLiteral());

    arktest.assertEQ(true, val.equals(FloatType.VAL));
    arktest.assertEQ(true, ref.equals(FloatType.REF));
    arktest.assertEQ(false, val.equals(ref));
}

function testDoubleType() {
    const val = DoubleType.VAL;
    const ref = DoubleType.REF;

    arktest.assertEQ(true, val.isPrimitive());
    arktest.assertEQ(false, val.isReference());
    arktest.assertEQ(true, ref.isPrimitive());
    arktest.assertEQ(true, ref.isReference());

    arktest.assertEQ(false, val.hasName());
    arktest.assertEQ('double', val.getLiteral());
    arktest.assertEQ(true, ref.hasName());
    arktest.assertEQ('Double', ref.getLiteral());

    arktest.assertEQ(true, val.equals(DoubleType.VAL));
    arktest.assertEQ(true, ref.equals(DoubleType.REF));
    arktest.assertEQ(false, val.equals(ref));
}

function testNumericAssignableFromAnyNumeric() {
    const nums = [
        ByteType.VAL,
        ShortType.VAL,
        IntType.VAL,
        LongType.VAL,
        FloatType.VAL,
        DoubleType.VAL,
    ];

    for (let i = 0; i < nums.length; ++i) {
        for (let j = 0; j < nums.length; ++j) {
        arktest.assertEQ(
            true,
            nums[i].assignableFrom(nums[j]),
            `expected ${nums[i].getLiteral()} ← ${nums[j].getLiteral()} to be true`
        );
        }
    }
}
