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

export final class DataView implements ArrayBufferView {
    /** Underlying buffer */
    private readonly buffer_: ArrayBufferLike
    /** Count of bytes in a view */
    private readonly byteLength_: int
    /** Offset from start of {@link buffer} */
    private readonly byteOffset_: int

    public get buffer(): ArrayBufferLike {
        return this.buffer_
    }

    public get byteLength(): number {
        return this.byteLength_
    }

    public get byteOffset(): number {
        return this.byteOffset_
    }

    private readonly actualBuffer: Buffer

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @throws RangeError if offset is out of array
     */
    public constructor(buffer: ArrayBufferLike, byteOffset: int) {
        this(buffer, byteOffset, (buffer as Buffer).getByteLength() - byteOffset)
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @param byteLength lenth of bytes to take
     * @throws RangeError if provided indicies are invalid
     */
    public constructor(buffer: ArrayBufferLike, byteOffset: int, byteLength: int) {
        this.buffer_ = buffer
        this.actualBuffer = buffer
        const bufLen = this.actualBuffer.getByteLength()
        if (byteOffset < 0 || byteLength < 0 || byteOffset > bufLen || byteOffset + byteLength > bufLen) {
            throw new RangeError("invalid arguments")
        }
        this.byteOffset_ = byteOffset
        this.byteLength_ = byteLength
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @param byteLength lenth of bytes to take
     * @throws RangeError if provided indicies are invalid
     */
    public constructor(buffer: ArrayBufferLike, byteOffset?: Number, byteLength?: Number) {
        this(buffer, asIntOrDefault(byteOffset, 0), asIntOrDefault(byteLength, (buffer as Buffer).getByteLength()))
    }
    // === Int8 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt8(byteOffset: int): int {
        return this.getInt8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: int, value: int): void {
        this.setInt8Big(byteOffset, value)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt8(byteOffset: number): number {
        return this.getInt8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: number, value: number): void {
        this.setInt8Big(byteOffset, value)
    }
    private getInt8Big(byteOffset: int): int {
        if (byteOffset + 1 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = this.actualBuffer.at(startByte + 0 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return (res as byte) as int
    }
    private getInt8Big(byteOffset: number): number {
        let res = this.getInt8Big(byteOffset as int)
        return res;
    }

    private setInt8Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 1.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 0 - i,  byteVal)
        }
    }
    private setInt8Big(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setInt8Big(byteOffset as int, val)
    }
    // === Uint8 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint8(byteOffset: int): int {
        return this.getUint8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: int, value: int): void {
        this.setUint8Big(byteOffset, value)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint8(byteOffset: number): number {
        return this.getUint8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: number, value: number): void {
        this.setUint8Big(byteOffset, value)
    }
    private getUint8Big(byteOffset: int): int {
        if (byteOffset + 1 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = this.actualBuffer.at(startByte + 0 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private getUint8Big(byteOffset: number): number {
        let res = this.getUint8Big(byteOffset as int)
        return res;
    }

    private setUint8Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 1.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 0 - i,  byteVal)
        }
    }
    private setUint8Big(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setUint8Big(byteOffset as int, val)
    }
    // === Int16 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt16(byteOffset: int): int {
        return this.getInt16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt16(byteOffset: int, value: int): void {
        this.setInt16Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setInt16Little(byteOffset, value)
        } else {
            this.setInt16Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setInt16Little(byteOffset, value)
        } else {
            this.setInt16Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt16(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getInt16Little(byteOffset)
        } else {
            return this.getInt16Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt16(byteOffset: int, littleEndian: boolean): int {
        if (littleEndian) {
            return this.getInt16Little(byteOffset)
        } else {
            return this.getInt16Big(byteOffset)
        }
    }
    private getInt16Little(byteOffset: int): int {
        if (byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return (res as short) as int
    }
    private getInt16Little(byteOffset: number): number {
        let res = this.getInt16Little(byteOffset as int)
        return res;
    }

    private setInt16Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setInt16Little(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setInt16Little(byteOffset as int, val)
    }
    private getInt16Big(byteOffset: int): int {
        if (byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.actualBuffer.at(startByte + 1 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return (res as short) as int
    }
    private getInt16Big(byteOffset: number): number {
        let res = this.getInt16Big(byteOffset as int)
        return res;
    }

    private setInt16Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 1 - i,  byteVal)
        }
    }
    private setInt16Big(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setInt16Big(byteOffset as int, val)
    }
    // === Uint16 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint16(byteOffset: int): int {
        return this.getUint16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint16(byteOffset: int, value: int): void {
        this.setUint16Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setUint16Little(byteOffset, value)
        } else {
            this.setUint16Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setUint16Little(byteOffset, value)
        } else {
            this.setUint16Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint16(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getUint16Little(byteOffset)
        } else {
            return this.getUint16Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint16(byteOffset: int, littleEndian: boolean): int {
        if (littleEndian) {
            return this.getUint16Little(byteOffset)
        } else {
            return this.getUint16Big(byteOffset)
        }
    }
    private getUint16Little(byteOffset: int): int {
        if (byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private getUint16Little(byteOffset: number): number {
        let res = this.getUint16Little(byteOffset as int)
        return res;
    }

    private setUint16Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setUint16Little(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setUint16Little(byteOffset as int, val)
    }
    private getUint16Big(byteOffset: int): int {
        if (byteOffset + 2 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.actualBuffer.at(startByte + 1 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private getUint16Big(byteOffset: number): number {
        let res = this.getUint16Big(byteOffset as int)
        return res;
    }

    private setUint16Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 2.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 1 - i,  byteVal)
        }
    }
    private setUint16Big(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setUint16Big(byteOffset as int, val)
    }
    // === Int32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt32(byteOffset: int): int {
        return this.getInt32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt32(byteOffset: int, value: int): void {
        this.setInt32Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt32(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setInt32Little(byteOffset, value)
        } else {
            this.setInt32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt32(byteOffset: int, value: int, littleEndian: boolean): void {
        if (littleEndian) {
            this.setInt32Little(byteOffset, value)
        } else {
            this.setInt32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt32(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getInt32Little(byteOffset)
        } else {
            return this.getInt32Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt32(byteOffset: int, littleEndian: boolean): int {
        if (littleEndian) {
            return this.getInt32Little(byteOffset)
        } else {
            return this.getInt32Big(byteOffset)
        }
    }
    private getInt32Little(byteOffset: int): int {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return (res as int) as int
    }
    private getInt32Little(byteOffset: number): number {
        let res = this.getInt32Little(byteOffset as int)
        return res;
    }

    private setInt32Little(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setInt32Little(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setInt32Little(byteOffset as int, val)
    }
    private getInt32Big(byteOffset: int): int {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + 3 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return (res as int) as int
    }
    private getInt32Big(byteOffset: number): number {
        let res = this.getInt32Big(byteOffset as int)
        return res;
    }

    private setInt32Big(byteOffset: int, value: int): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }
    private setInt32Big(byteOffset: number, value: number): void {
        const val: int = value as int;
        this.setInt32Big(byteOffset as int, val)
    }
    // === Uint32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint32(byteOffset: int): long {
        return this.getUint32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint32(byteOffset: int, value: long): void {
        this.setUint32Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setUint32Little(byteOffset, value)
        } else {
            this.setUint32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setUint32Little(byteOffset, value)
        } else {
            this.setUint32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint32(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getUint32Little(byteOffset)
        } else {
            return this.getUint32Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint32(byteOffset: int, littleEndian: boolean): long {
        if (littleEndian) {
            return this.getUint32Little(byteOffset)
        } else {
            return this.getUint32Big(byteOffset)
        }
    }
    private getUint32Little(byteOffset: int): long {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private getUint32Little(byteOffset: number): number {
        let res = this.getUint32Little(byteOffset as int)
        return res;
    }

    private setUint32Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setUint32Little(byteOffset: number, value: number): void {
        const val: long = value as long;
        this.setUint32Little(byteOffset as int, val)
    }
    private getUint32Big(byteOffset: int): long {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + 3 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private getUint32Big(byteOffset: number): number {
        let res = this.getUint32Big(byteOffset as int)
        return res;
    }

    private setUint32Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }
    private setUint32Big(byteOffset: number, value: number): void {
        const val: long = value as long;
        this.setUint32Big(byteOffset as int, val)
    }
    // === Float32 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat32(byteOffset: int): float {
        return this.getFloat32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat32(byteOffset: int, value: float): void {
        this.setFloat32Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat32(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setFloat32Little(byteOffset, value)
        } else {
            this.setFloat32Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat32(byteOffset: int, value: float, littleEndian: boolean): void {
        if (littleEndian) {
            this.setFloat32Little(byteOffset, value)
        } else {
            this.setFloat32Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat32(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getFloat32Little(byteOffset)
        } else {
            return this.getFloat32Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat32(byteOffset: int, littleEndian: boolean): float {
        if (littleEndian) {
            return this.getFloat32Little(byteOffset)
        } else {
            return this.getFloat32Big(byteOffset)
        }
    }
    private getFloat32Little(byteOffset: int): float {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return Float.bitCastFromInt(res)
    }
    private getFloat32Little(byteOffset: number): number {
        let res = this.getFloat32Little(byteOffset as int)
        return res;
    }

    private setFloat32Little(byteOffset: int, value: float): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setFloat32Little(byteOffset: number, value: number): void {
        const val: float = value as float;
        this.setFloat32Little(byteOffset as int, val)
    }
    private getFloat32Big(byteOffset: int): float {
        if (byteOffset + 4 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: int = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.actualBuffer.at(startByte + 3 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return Float.bitCastFromInt(res)
    }
    private getFloat32Big(byteOffset: number): number {
        let res = this.getFloat32Big(byteOffset as int)
        return res;
    }

    private setFloat32Big(byteOffset: int, value: float): void {
        if (byteOffset < 0 || byteOffset + 4.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 3 - i,  byteVal)
        }
    }
    private setFloat32Big(byteOffset: number, value: number): void {
        const val: float = value as float;
        this.setFloat32Big(byteOffset as int, val)
    }
    // === Int64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigInt64(byteOffset: int): long {
        return this.getBigInt64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setBigInt64(byteOffset: int, value: long): void {
        this.setBigInt64Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigInt64(byteOffset: number, value: bigint, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setBigInt64Little(byteOffset, value)
        } else {
            this.setBigInt64Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigInt64(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setBigInt64Little(byteOffset, value)
        } else {
            this.setBigInt64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigInt64(byteOffset: number, littleEndian?: boolean): bigint {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getBigInt64Little(byteOffset)
        } else {
            return this.getBigInt64Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigInt64(byteOffset: int, littleEndian: boolean): long {
        if (littleEndian) {
            return this.getBigInt64Little(byteOffset)
        } else {
            return this.getBigInt64Big(byteOffset)
        }
    }
    private getBigInt64Little(byteOffset: int): long {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return (res as long) as long
    }
    private getBigInt64Little(byteOffset: number): bigint {
        let res = this.getBigInt64Little(byteOffset as int)
        return new BigInt(res)
    }

    private setBigInt64Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setBigInt64Little(byteOffset: number, value: bigint): void {
        const val: long = value.getLong();
        this.setBigInt64Little(byteOffset as int, val)
    }
    private getBigInt64Big(byteOffset: int): long {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return (res as long) as long
    }
    private getBigInt64Big(byteOffset: number): bigint {
        let res = this.getBigInt64Big(byteOffset as int)
        return new BigInt(res)
    }

    private setBigInt64Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }
    private setBigInt64Big(byteOffset: number, value: bigint): void {
        const val: long = value.getLong();
        this.setBigInt64Big(byteOffset as int, val)
    }
    // === Uint64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigUint64(byteOffset: int): long {
        return this.getBigUint64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setBigUint64(byteOffset: int, value: long): void {
        this.setBigUint64Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigUint64(byteOffset: number, value: bigint, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setBigUint64Little(byteOffset, value)
        } else {
            this.setBigUint64Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setBigUint64(byteOffset: int, value: long, littleEndian: boolean): void {
        if (littleEndian) {
            this.setBigUint64Little(byteOffset, value)
        } else {
            this.setBigUint64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigUint64(byteOffset: number, littleEndian?: boolean): bigint {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getBigUint64Little(byteOffset)
        } else {
            return this.getBigUint64Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getBigUint64(byteOffset: int, littleEndian: boolean): long {
        if (littleEndian) {
            return this.getBigUint64Little(byteOffset)
        } else {
            return this.getBigUint64Big(byteOffset)
        }
    }
    private getBigUint64Little(byteOffset: int): long {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private getBigUint64Little(byteOffset: number): bigint {
        let res = this.getBigUint64Little(byteOffset as int)
        return DataView.bigintFromULong(res)
    }

    private setBigUint64Little(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setBigUint64Little(byteOffset: number, value: bigint): void {
        const val: long = value.getLong();
        this.setBigUint64Little(byteOffset as int, val)
    }
    private getBigUint64Big(byteOffset: int): long {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private getBigUint64Big(byteOffset: number): bigint {
        let res = this.getBigUint64Big(byteOffset as int)
        return DataView.bigintFromULong(res)
    }

    private setBigUint64Big(byteOffset: int, value: long): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }
    private setBigUint64Big(byteOffset: number, value: bigint): void {
        const val: long = value.getLong();
        this.setBigUint64Big(byteOffset as int, val)
    }
    // === Float64 ===

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat64(byteOffset: int): number {
        return this.getFloat64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat64(byteOffset: int, value: number): void {
        this.setFloat64Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat64(byteOffset: number, value: number, littleEndian?: boolean): void {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            this.setFloat64Little(byteOffset, value)
        } else {
            this.setFloat64Big(byteOffset, value)
        }
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat64(byteOffset: int, value: number, littleEndian: boolean): void {
        if (littleEndian) {
            this.setFloat64Little(byteOffset, value)
        } else {
            this.setFloat64Big(byteOffset, value)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat64(byteOffset: number, littleEndian?: boolean): number {
        if (littleEndian !== undefined && littleEndian.valueOf() == true) {
            return this.getFloat64Little(byteOffset)
        } else {
            return this.getFloat64Big(byteOffset)
        }
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat64(byteOffset: int, littleEndian: boolean): number {
        if (littleEndian) {
            return this.getFloat64Little(byteOffset)
        } else {
            return this.getFloat64Big(byteOffset)
        }
    }
    private getFloat64Little(byteOffset: int): number {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return Double.bitCastFromLong(res)
    }
    private getFloat64Little(byteOffset: number): number {
        let res = this.getFloat64Little(byteOffset as int)
        return res;
    }

    private setFloat64Little(byteOffset: int, value: number): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + i,  byteVal)
        }
    }
    private setFloat64Little(byteOffset: number, value: number): void {
        const val: number = value as number;
        this.setFloat64Little(byteOffset as int, val)
    }
    private getFloat64Big(byteOffset: int): number {
        if (byteOffset + 8 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let res: long = 0;
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.actualBuffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return Double.bitCastFromLong(res)
    }
    private getFloat64Big(byteOffset: number): number {
        let res = this.getFloat64Big(byteOffset as int)
        return res;
    }

    private setFloat64Big(byteOffset: int, value: number): void {
        if (byteOffset < 0 || byteOffset + 8.0 > this.byteLength_) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset_ + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.actualBuffer.set(startByte + 7 - i,  byteVal)
        }
    }
    private setFloat64Big(byteOffset: number, value: number): void {
        const val: number = value as number;
        this.setFloat64Big(byteOffset as int, val)
    }

    private static bigintFromULong(x: long): bigint {
        const noSignMask: long = ((1 as long) << 63) - 1
        return new BigInt(x & noSignMask) + (new BigInt((x >> 63) & 0x1) << new BigInt(63))
    }
}
