/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

package escompat;

/**
 * `bigint` is an alias for type `BigInt`
 */
export type bigint = BigInt;

type Vec = ArrayAsListInt;

/**
 * BigInt class stub
 */
export final class BigInt {
    private bytes: Vec;
    private sign: boolean;

    private static readonly MASKS_LEFT_SHIFT: int[] = [0b00000000,
                                                       0b10000000,
                                                       0b11000000,
                                                       0b11100000,
                                                       0b11110000,
                                                       0b11111000,
                                                       0b11111100,
                                                       0b11111110,
                                                       0b11111111];

    private static readonly MASKS_RIGHT_SHIFT: int[] = [0b00000000,
                                                        0b00000001,
                                                        0b00000011,
                                                        0b00000111,
                                                        0b00001111,
                                                        0b00011111,
                                                        0b00111111,
                                                        0b01111111,
                                                        0b11111111];

    constructor() {
        this.bytes = new Vec(1);
        this.sign = true;
    }

    constructor(d: byte) {
        let bigint = new BigInt((d as Byte).toString());
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    constructor(d: short) {
        let bigint = new BigInt((d as Short).toString());
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    constructor(d: int) {
        let bigint = new BigInt((d as Int).toString());
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    constructor(d: long) {
        let bigint = new BigInt((d as Long).toString());
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    constructor(d: double) {
        let bigint = new BigInt((d as Double).toString());
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    constructor(d: string) {
        this.bytes = new Vec();
        this.parseFromString(d);
    }

    constructor(d: boolean) {
        throw new Error("BigInt.constructor(boolean): not implemented")
    }

    constructor(d: BigInt) {
        let bigint = d.clone();
        this.bytes = bigint.bytes;
        this.sign = bigint.sign;
    }

    internal static fromULong(val: long): BigInt {
        // NOTE (ikorobkov): needs to check by tests
        // let a: long = (val >> 63) & 1
        // a = a << 63
        // let b = val & 0x7fffffffffffffff
        // let c = a + b
        // return new BigInt(c)
        return new BigInt(val)
    }

    internal getULong(): long {
        return BigInt.asUintN(64, this).longValue()
    }

    internal getLong(): long {
        let val = this.clone()
        let bts = val.asBytes()
        let len = val.length()
        let res: long = 0;

        for (let i = 0; i < len; i++) {
            res = res << 8
            res += bts.at(i)
        }
        if (this.negative()) {
            res *= -1
        }

        return res
    }

    public equals(to: BigInt): boolean {
        return this.toString().equals(to.toString());
    }

    public override equals(to: NullishType): boolean {
        return (to instanceof BigInt) && this.equals(to as BigInt);
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param locales
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object, options: Object): string {
        throw new Error("BigInt.toLocaleString: not implemented")
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @returns string representation
     */
    public toLocaleString(): string {
        return this.toLocaleString(new Object(), new Object())
    }

    public override toString(): string {
        let digits = this.bytesToDigits(this.asBytes(), this.negative());

        if (digits.size() == 1) {
            if (digits.at(0) == 0) {
                return "0";
            }
        }

        let s: string = "";
        if (this.negative()) {
            s += "-";
        }

        for (let i = 0; i < digits.size(); i++) {
            s += digits.at(i).toString();
        }

        return s;
    }

    public toString(radix: double): string {
        if (this.equals(0n)) {
            return "0"
        }
        const radixInt = radix as int
        const q = radixInt as number
        const qBigInt = new BigInt(q)
        let buff = ""
        let absolute = this.abs()
        while (absolute > 0n) {
            // temporary solution, need to optimize divide-mod operations
            let tempDiv = new BigInt((new Number(absolute) / q) as int)
            const mod = absolute - tempDiv * qBigInt
            const digit = new Number(mod.doubleValue())
            buff = digit.toString(radixInt).concat(buff)
            absolute = tempDiv
        }
        buff = (this.negative() ? "-" : "") + buff
        return buff
    }

    public valueOf(): BigInt {
        return this
    }

    public static asIntN(bits: long, num: BigInt): BigInt {
        if (bits == 0) {
            return 0n
        }
        // operatorLeftShift temporary has limitations to 255 on rhs
        // in this case must be bits <= 255
        //  please check `operatorLeftShift` implementation

        // 2^(bits-1)
        const half = 1n << new BigInt(bits-1)
        // 2^(bits)-1 = 2 * half - 1
        let mask = 2n * half - 1n
        const masked = num & mask
        // maxRepresentable = 2^(bits) = mask + 1
        // if masked > 2^(bits-1) : masked - maxRepresentableValue : masked
        return masked >= half ? masked - (mask + 1n): masked
    }

    public static asUintN(bits: long, num: BigInt): BigInt {
        // operatorLeftShift temporary has limitations to 255 on rhs
        // in this case must be bits <= 255
        //  please check `operatorLeftShift` implementation
        return num & ((1n << (new BigInt(bits))) - 1n)
    }

    public negative(): boolean {
        return this.sign == false;
    }

    public positive(): boolean {
        return this.sign == true;
    }

    private getSign(): boolean {
        return this.sign;
    }

    private length(): int {
        return this.bytes.size();
    }

    private setSign(sign: boolean): void {
        if ((this.sign == false) && (sign == true)) {
            this.bytes = BigInt.from2sCompliment(this.bytes);
        }

        if ((this.sign == true) && (sign == false)) {
            this.bytes = BigInt.to2sComplement(this.bytes);
        }

        this.sign = sign;
    }

    private isZero(): boolean {
        if (this.bytes.size() == 0) {
            return false;
        }

        let zero = true;
        for (let i = 0; i < this.bytes.size(); i++) {
            if (this.bytes.at(i) != 0) {
                zero = false;
                break;
            }
        }

        return zero;
    }

    private clone(): BigInt {
        let bigint = new BigInt();
        bigint.setSign(this.sign);
        bigint.setBytes(this.asBytes());

        return bigint;
    }

    public operatorAdd(other: BigInt): BigInt {
        let result = new BigInt();
        let lhs = this.clone();
        let rhs = other.clone();

        if (lhs.negative() && rhs.negative()) {
            return -(-lhs + (-rhs));
        }

        // Different signs - perform subtraction
        if (lhs.negative() && !rhs.negative()) {
            lhs.setSign(true);
            return -(lhs - rhs);
        } else if (!lhs.negative() && rhs.negative()) {
            rhs.setSign(true);
            return lhs - rhs;
        }

        // Pad the shorter number with zeroes to make them of equal length
        let max_len = max(lhs.length(), rhs.length()) as int;
        lhs.addBytes(max_len - lhs.length());
        rhs.addBytes(max_len - rhs.length());

        let l = lhs.asBytes();
        let r = rhs.asBytes();
        let res = result.asBytes();
        res = new Vec();

        let carry = 0;
        for (let i = 0; i < max_len; i++) {
            let sum = l.at(i) + r.at(i) + carry;
            carry = sum / 256;
            res.pushBack(sum % 256);
        }

        if (carry > 0) {
            res.pushBack(carry);
        }

        result.setBytes(res);

        return result;
    }

    public operatorSubtract(other: BigInt): BigInt {
        let result = new BigInt();
        let lhs = this.clone();
        let rhs = other.clone();

        if (lhs.negative() && rhs.negative()) {
            return lhs + (-rhs);
        }

        // Different signs - perform addition
        if (lhs.negative() && !rhs.negative()) {
            lhs.setSign(true);
            return -(lhs + rhs);
        } else if (!lhs.negative() && rhs.negative()) {
            rhs.setSign(true);
            return lhs + rhs;
        }

        if (lhs.abs() > rhs.abs()) {
        } else {
            result.setSign(false);
        }

        // Pad the shorter number with zeroes to make them of equal length
        let max_len = max(lhs.length(), rhs.length()) as int;
        lhs.addBytes(max_len - lhs.length());
        rhs.addBytes(max_len - rhs.length());

        let l = lhs.asBytes();
        let r = rhs.asBytes();
        let res = result.asBytes();
        res = new Vec();

        let borrow = 0;
        for (let i = 0; i < max_len; i++) {
            let byte1 = l.at(i);
            let byte2 = r.at(i);
            let diff = byte1 - byte2 - borrow;

            if (diff < 0) {
                diff += 256;
                borrow = 1;
            } else {
                borrow = 0;
            }

            res.pushFront(diff);
        }

        while (!(res.size() == 0)) {
            if (res.at(0) == 0) {
                res.popFront();
                continue;
            }

            break;
        }

        res = BigInt.reverse(res);
        if (res.size() == 0) {
            res.pushFront(0);
        }

        let zero = true;
        for (let i = 0; i < res.size(); i++) {
            if (res.at(i) != 0) {
                zero = false;
            }
        }

        if (zero) {
            result.setSign(true);
        }

        result.setBytes(res);
        return result;
    }

    public operatorMultiply(other: BigInt): BigInt {
        if (this.isZero() || other.isZero()) {
            return new BigInt();
        }

        let result = new BigInt();
        if (this.getSign() == other.getSign()) {
            result.setSign(true);
        } else {
            result.setSign(false);
        }

        let len1 = this.length();
        let len2 = other.length();
        let num1 = this.asBytes();
        let num2 = other.asBytes();

        if (this.negative()) {
            num1 = BigInt.from2sCompliment(num1);
        }

        if (other.negative()) {
            num2 = BigInt.from2sCompliment(num2);
        }

        num1 = BigInt.reverse(num1);
        num2 = BigInt.reverse(num2);
        let product = new Vec(len1 + len2, 0);
        for (let i = len1 - 1; i >= 0; i--) {
            let mulflag = 0;
            let addflag = 0;
            for (let j = len2 - 1; j >= 0; j--) {
                let temp1 = num1.at(i) * num2.at(j) + mulflag;
                mulflag = temp1 / 256;
                temp1 = temp1 % 256;
                let temp2 = product.at(i + j + 1) + temp1 + addflag;
                product.set(i + j + 1, temp2 % 256);
                addflag = temp2 / 256;
            }

            product.set(i, (product.at(i) + mulflag + addflag) % 256);
        }

        if (!(product.size() == 0)) {
            if (product.at(0) == 0) {
                product.popFront();
            }
        }

        product = BigInt.reverse(product);
        if (result.negative()) {
            product = BigInt.to2sComplement(product);
        }

        result.setBytes(product);

        return result;
    }

    public operatorDivide(other: BigInt): BigInt {
        // NOTE(kkonsw): currently divide operation is very slow
        if (other.isZero()) {
            throw new Error("BigInt: division by zero")
        }

        let result = new BigInt();
        if (this.getSign() == other.getSign()) {
            result.setSign(true);
        } else {
            result.setSign(false);
        }

        let bytes = this.asBytes();
        if (this.negative()) {
            bytes = BigInt.from2sCompliment(bytes);
        }

        let rhs = other.clone();
        if (rhs.negative()) {
            // Change bytes representation
            rhs.setSign(true);
        }

        bytes = BigInt.reverse(bytes);
        let res = new Vec();

        let carry = new BigInt();
        let mask = new BigInt("256");
        for (let i = 0; i < bytes.size(); i++) {
            let current = new BigInt();
            let b = new BigInt(bytes.at(i).toString());
            current = b + carry * mask;

            if (rhs > current) {
                carry = current;
                bytes.set(i, 0);
                continue;
            }

            // Calculate quotient and remainder
            let temp: BigInt[] = current.divideHelper(rhs);
            bytes.set(i, temp[0].asByte());
            carry = temp[1];
            res.pushBack(bytes.at(i));
        }

        res = BigInt.reverse(res);
        if (res.size() == 0) {
            res.pushBack(0);
        }

        if (result.negative()) {
            res = BigInt.to2sComplement(res);
        }

        result.setBytes(res);
        return result;
    }

    public operatorModule(other: BigInt): BigInt {
        // NOTE(kkonsw): currently module operation is very slow
        if (other.isZero()) {
            throw new Error("BigInt: division by zero")
        }

        let result = new BigInt();
        let bytes = this.asBytes();

        if (this.negative()) {
            bytes = BigInt.from2sCompliment(bytes);
        }

        let rhs = other.clone();
        if (rhs.negative()) {
            // Change bytes representation
            rhs.setSign(true);
        }

        bytes = BigInt.reverse(bytes);
        let mask = new BigInt("256");
        for (let i = 0; i < bytes.size(); i++) {
            let current = new BigInt();
            let b = new BigInt(bytes.at(i).toString());
            current = b + result * mask;

            if (rhs > current) {
                result = current;
                bytes.set(i, 0);
                continue;
            }

            // Calculate quotient and remainder
            let temp: BigInt[] = current.divideHelper(rhs);
            bytes.set(i, temp[0].asByte());
            result = temp[1];
        }

        if (this.negative() && rhs.negative()) {
            result.sign = false;
        }

        if (this.negative() && rhs.positive()) {
            result.sign = false;
        }

        if (this.positive() && rhs.positive()) {
            result.sign = true;
        }

        if (result.negative()) {
            result.bytes = BigInt.to2sComplement(result.bytes);
        }

        return result;
    }

    public operatorBitwiseOr(other: BigInt): BigInt {
        let result = new BigInt();
        let num1 = this.asBytes();
        let num2 = other.asBytes();

        let min_len = min(num1.size(), num2.size()) as int;
        let res = result.asBytes();
        res = new Vec();

        for (let i = 0; i < min_len; i++) {
            res.pushBack(num1.at(i) | num2.at(i));
        }

        for (let i = min_len; i < num1.size(); i++) {
            res.pushBack(num1.at(i) | 0x00);
        }

        for (let i = min_len; i < num2.size(); i++) {
            res.pushBack(num2.at(i) | 0x00);
        }

        result.setBytes(res);
        return result;
    }

    public operatorBitwiseAnd(other: BigInt): BigInt {
        let result = new BigInt();
        let num1 = this.asBytes();
        let num2 = other.asBytes();

        let min_len = min(num1.size(), num2.size()) as int;
        let res = result.asBytes();
        res = new Vec();

        for (let i = 0; i < min_len; i++) {
            res.pushBack(num1.at(i) & num2.at(i));
        }

        for (let i = min_len; i < num1.size(); i++) {
            res.pushBack(num1.at(i) & 0x00);
        }

        for (let i = min_len; i < num2.size(); i++) {
            res.pushBack(num2.at(i) & 0x00);
        }

        result.setBytes(res);
        return result;
    }

    public operatorBitwiseXor(other: BigInt): BigInt {
        let result = new BigInt();
        let num1 = this.asBytes();
        let num2 = other.asBytes();

        let min_len = min(num1.size(), num2.size()) as int;
        let res = result.asBytes();
        res = new Vec();

        for (let i = 0; i < min_len; i++) {
            res.pushBack(num1.at(i) ^ num2.at(i));
        }

        for (let i = min_len; i < num1.size(); i++) {
            res.pushBack(num1.at(i) ^ 0x00);
        }

        for (let i = min_len; i < num2.size(); i++) {
            res.pushBack(num2.at(i) ^ 0x00);
        }

        result.setBytes(res);
        return result;
    }

    public operatorGreaterThan(other: BigInt): boolean {
        if (this.positive() && other.negative()) {
            return true;
        }

        if (this.negative() && other.positive()) {
            return false;
        }

        if (this.positive() && other.positive()) {
            const l = this.asBytes();
            const r = other.asBytes();

            const ls = l.size()
            const rs = r.size()
            const maxSize = ls > rs ? ls : rs

            // if |X| = |Y| we need check that forall xi>yi
            // if |X| < |Y| or |Y| < |X| we can add zeros to the smallest of the vectors
            // and reduce to the case when the vector lengths are the same

            for (let i = maxSize - 1; i >= 0; i--) {
                let left_byte = i < ls ? l.at(i) : 0;
                let right_byte = i < rs ? r.at(i) : 0;
                if (left_byte == right_byte) {
                    continue;
                }
                return left_byte > right_byte;
            }
            // in this case forall i: xi=yi => numbers is equals

            return false;
        }

        if (this.negative() && other.negative()) {
            let l = BigInt.from2sCompliment(this.asBytes());
            let r = BigInt.from2sCompliment(other.asBytes());

            const ls = l.size()
            const rs = r.size()
            const maxSize = ls > rs ? ls : rs

            for (let i = maxSize - 1; i >= 0; i--) {
                let left_byte = i < ls ? l.at(i) : 0;
                let right_byte = i < rs ? r.at(i) : 0;
                if (left_byte == right_byte) {
                    continue;
                }

                return left_byte < right_byte;
            }

            return false;
        }

        return false;
    }

    public operatorLessThan(other: BigInt): boolean {
        return other > this;
    }

    public operatorGreaterThanEqual(other: BigInt): boolean {
        return !(other > this);
    }

    public operatorLessThanEqual(other: BigInt): boolean {
        return !(other < this);
    }

    public operatorLeftShift(other: BigInt): BigInt {
        if (other.negative()) {
            throw new Error("BigInt: left shift against negative number")
        }

        // NOTE(kkonsw): temporary limitation,
        // unlimited values here can create really big integers that will require a lot of memory
        let max_shift = new BigInt("255");
        if (other.abs() > max_shift) {
            throw new Error("BigInt: too many digits in integer")
        }

        let result = this.clone();
        let n = (other / new BigInt("8")).asByte();
        let r = (other % new BigInt("8")).asByte();

        let res = result.asBytes();
        if (result.negative()) {
            res = BigInt.from2sCompliment(res);
        }

        let overflow = 0;
        for (let i = 0; i < res.size(); i++) {
            let temp = res.at(i);
            res.set(i, (res.at(i) << r) % 256);
            res.set(i, res.at(i) | overflow);
            overflow = (temp & BigInt.MASKS_LEFT_SHIFT[r]) >>> (8 - r)
            overflow = overflow % 256
        }

        if (overflow > 0) {
            res.pushBack(overflow);
        }

        for (let i = 0; i < n; i++) {
            res.pushFront(0);
        }

        if (result.negative()) {
            res = BigInt.to2sComplement(res);
        }

        result.setBytes(res);
        return result;
    }

    public operatorRightShift(other: BigInt): BigInt {
        if (other.negative()) {
            throw new Error("BigInt: left shift against negative number")
        }

        let result = this.clone();
        let n = other / new BigInt("8");
        let r = (other % new BigInt("8")).asByte();

        let res = result.asBytes();

        let count = new BigInt();
        while (res.size() != 0) {
            if (count.equals(n)) {
                break;
            }

            res.popFront();
            count++;
        }

        res = BigInt.reverse(res);

        let overflow = 0;
        if (this.negative()) {
            overflow = BigInt.MASKS_LEFT_SHIFT[r];
        }

        for (let i = 0; i < res.size(); i++) {
            let temp = res.at(i);
            res.set(i, (res.at(i) >>> r) % 256);
            res.set(i, res.at(i) | overflow);
            overflow = ((temp & BigInt.MASKS_RIGHT_SHIFT[r]) << (8 - r)) % 256;
        }

        res = BigInt.reverse(res);
        if (res.size() > 1 && res.at(res.size() - 1) == 255 && r > 0) {
            res.popBack();
        }
        if (res.size() == 0) {
            res.pushBack(0);
        }

        result.setBytes(res);
        return result;
    }

    public operatorIncrement(): BigInt {
        let result = new BigInt("1") + this;
        this.sign = result.sign;
        this.bytes = result.bytes;
        return result
    }

    public operatorDecrement(): BigInt {
        let result = this.clone();
        result = result - new BigInt("1");
        this.sign = result.sign;
        this.bytes = result.bytes;
        return result
    }

    public operatorBitwiseNot(): BigInt {
        let result = this.clone();
        let bytes = result.bytes;

        for (let i = 0; i < bytes.size(); i++) {
            let a = bytes.at(i);
            let b = ~a;
            if (b < 0) {
                b = 256 + b;
            }

            bytes.set(i, b);
        }

        if (this.positive()) {
            result.sign = false;
        }

        if (this.negative()) {
            result.sign = true;
        }

        result.setBytes(bytes);
        return result;
    }

    private addBytes(n: int): void {
        for (let i = 0; i < n; i++) {
            this.bytes.pushBack(0);
        }
    }

    public negate(): BigInt {
        let result = this.clone();
        if (this.negative()) {
            result.setSign(true);
        } else {
            result.setSign(false);
        }

        return result;
    };

    private divideHelper(other: BigInt): BigInt[] {
        if (other > this) {
            return [new BigInt(), this.clone()];
        }

        let remainder = this.clone();
        let quotient = new BigInt();
        let inc = new BigInt("1");
        while (remainder >= other) {
            remainder = remainder - other;
            quotient = quotient + inc;
        }

        if (remainder.negative()) {
            remainder = new BigInt();
        }

        return [quotient, remainder];
    }

    // NOTE(kkonsw): should return byte
    private asByte(): int {
        if (this.bytes.size() != 0) {
            return this.bytes.at(0) % 256;
        }

        return 0;
    }

    private abs(): BigInt {
        let result = this.clone();
        if (result.negative()) {
            result.setSign(true);
        }

        return result;
    }

    private setBytes(bytes: Vec): void {
        this.bytes = bytes;
    }

    private asBytes(): Vec {
        let bytes = new Vec(this.length(), 0);
        for (let i = 0; i < this.length(); i++) {
            bytes.set(i, this.bytes.at(i));
        }

        return bytes;
    }

    private bytesToDigits(input: Vec, negative: boolean): Vec {
        let bytes = input;
        if (negative) {
            bytes = BigInt.from2sCompliment(bytes);
        }

        let rbytes = BigInt.reverse(bytes);
        let digits = new Vec();
        let mask = new Vec();
        mask.pushBack(2);
        mask.pushBack(5);
        mask.pushBack(6);

        for (let i = 0; i < rbytes.size(); i++) {
            digits = BigInt.multiplyDigits(digits, mask);
            let temp = new Vec();
            let b = rbytes.at(i).toString();
            for (let j = 0; j < b.length; j++) {
                temp.pushBack(b.charAt(j) - c'0');
            }

            digits = BigInt.addDigits(digits, temp);
        }

        return digits;
    }

    private parseFromString(s: string): void {
        if (s.isEmpty()) {
            // Return 0 when provided string is empty
            s = "0"
        }

        if (!this.validate(s)) {
            throw new Error("BigInt.parseFromString(): provided string is invalid")
        }

        let negative: boolean = (s.charAt(0) == c'-');
        if (negative) {
            this.sign = false;
        } else {
            this.sign = true;
        }

        let len = s.length;
        let start = 0;
        if (this.negative()) {
            start = 1;
        }

        let digits = new Vec(0);
        for (let i = start; i < len; i++) {
            if ((s.charAt(i) == c'_')) {
                continue;
            }

            digits.pushBack(s.charAt(i) - c'0');
        }

        this.toBytes(digits);
    }

    private toBytes(digits: Vec) {
        while (!(digits.size() == 0)) {
            let carry = 0;
            for (let i = 0; i < digits.size(); i++) {
                let current = digits.at(i) + (carry as short) * 10;
                if (current < 256) {
                    carry = current;
                    digits.set(i, 0);
                    continue;
                }

                digits.set(i, current / 256);
                carry = current % 256;
            }

            this.bytes.pushBack(carry);
            while (!(digits.size() == 0)) {
                if (digits.at(0) == 0) {
                    digits.popFront();
                } else {
                    break;
                }
            }
        }

        if (this.negative()) {
            this.bytes = BigInt.to2sComplement(this.bytes);
        }
    }

    private validate(s: string): boolean {
        let negative = false;
        for (let i = 0; i < s.length; i++) {
            let c = s.charAt(i);
            if (Char.isDecDigit(c)) {
            } else if ((c == c'_') && (i > 0) && (i < s.length - 1)) {
                // Underscore, allowed only between digits
            } else if ((c == c'-') && (i == 0) && (!negative)) {
                // Minus sign, allowed only at the beginning of the string
                negative = true;
            } else {
                // Invalid character
                return false;
            }
        }

        return true;
    }

    private static multiplyDigits(lhs: Vec, rhs: Vec): Vec {
        if ((lhs.size() == 0) || (rhs.size() == 0)) {
            return new Vec(1, 0);
        }

        if (lhs.at(0) == 0) {
            return new Vec(1, 0);
        }

        if (rhs.at(0) == 0) {
            return new Vec(1, 0);
        }

        let len1 = lhs.size();
        let len2 = rhs.size();
        let product = new Vec(len1 + len2, 0);

        for (let i = len1 - 1; i >= 0; i--) {
            let mulflag = 0;
            let addflag = 0;
            for (let j = len2 - 1; j >= 0; j--) {
                let temp1 = lhs.at(i) * rhs.at(j) + mulflag;
                mulflag = temp1 / 10;
                temp1 = temp1 % 10;
                let temp2 = product.at(i + j + 1) + temp1 + addflag;
                product.set(i + j + 1, temp2 % 10);
                addflag = temp2 / 10;
            }

            product.set(i, (product.at(i) + mulflag + addflag) % 256);
        }

        if (!(product.size() == 0)) {
            if (product.at(0) == 0) {
                product.popFront();
            }
        }

        return product;
    }

    private static reverse(input: Vec): Vec {
        let result = new Vec();
        for (let i = 0; i < input.size(); i++) {
            result.pushFront(input.at(i));
        }

        return result;
    }

    private static from2sCompliment(input: Vec): Vec {
        let bytes = input;
        for (let i = 0; i < bytes.size(); i++) {
            if (bytes.at(i) > 0) {
                bytes.set(i, bytes.at(i) - 1);
                break;
            } else {
                bytes.set(i, 0xff);
            }
        }

        for (let i = 0; i < bytes.size(); i++) {
            bytes.set(i, 255 - bytes.at(i));
        }

        return bytes;
    }

    private static to2sComplement(input: Vec): Vec {
        // Perform 2's complement for negative numbers.
        let bytes = input;
        for (let i = 0; i < bytes.size(); i++) {
            bytes.set(i, 255 - bytes.at(i));
        }

        // Add 1 to complete 2's complement.
        let carry: short = 1;
        for (let i = 0; i < bytes.size(); i++) {
            let current: short = bytes.at(i) + carry as short;
            bytes.set(i, current % 256);
            carry = ((current >>> 8) as short)
        }

        return bytes;
    }

    private static addDigits(lhs: Vec, rhs: Vec): Vec {
        let result = new Vec();
        let l = lhs;
        let r = rhs;

        let max_len = max(l.size(), r.size()) as int;

        // Pad the shorter number with zeroes to make them of equal length
        let l_len = max_len - l.size();
        for (let i = 0; i < l_len; i++) {
            l.pushFront(0);
        }

        let r_len = max_len - r.size();
        for (let i = 0; i < r_len; i++) {
            r.pushFront(0);
        }

        let carry = 0;
        for (let i = max_len - 1; i >= 0; i--) {
            let sum = l.at(i) + r.at(i) + carry;
            carry = sum / 10;
            result.pushFront(sum % 10);
        }

        if (carry > 0) {
            result.pushFront(carry);
        }

        return result;
    }

    /**
     * Returns the value of an object as a double
     *
     * @returns object's value
     */
    public doubleValue(): double {
        let buff = 0 as number
        const bytes = this.abs().bytes
        for (let i = 0; i < bytes.size(); ++i) {
            buff += (bytes.at(i) as double) * Math.pow(256, i)
        }
        return this.negative() ? -buff : buff
    }

    /**
     * Returns the value of an object as a long
     *
     * @returns object's value
     */
    public longValue(): long {
        return this.doubleValue() as long
    }

}
