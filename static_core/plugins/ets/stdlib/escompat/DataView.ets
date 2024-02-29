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

export final class DataView {
    /** Underlying buffer */
    public readonly buffer: Buffer
    /** Count of bytes in a view */
    public readonly byteLength: int
    /** Offset from start of {@link buffer} */
    public readonly byteOffset: int

    /**
     * Constructs view
     * @param buffer underlying buffer
     */
    public constructor(buffer: Buffer) {
        this(buffer, 0)
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @throws RangeError if offset is out of array
     */
    public constructor(buffer: Buffer, byteOffset: int) {
        this(buffer, byteOffset, buffer.getByteLength() - byteOffset)
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @throws RangeError if offset is out of array
     */
    public constructor(buffer: ArrayBuffer, byteOffset: number) {
        this(buffer, byteOffset as int)
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @param byteLength lenth of bytes to take
     * @throws RangeError if provided indicies are invalid
     */
    public constructor(buffer: ArrayBuffer, byteOffset: number, byteLength: number) {
        this(buffer, byteOffset as int, byteLength as int)
    }

    /**
     * Constructs view
     * @param buffer underlying buffer
     * @param byteOffset offset to start from
     * @param byteLength lenth of bytes to take
     * @throws RangeError if provided indicies are invalid
     */
    public constructor(buffer: Buffer, byteOffset: int, byteLength: int) {
        if (byteOffset < 0 || byteLength < 0 || byteOffset > buffer.getByteLength() || byteOffset + byteLength > buffer.getByteLength()) {
            throw new RangeError("invalid arguments")
        }
        this.buffer = buffer
        this.byteOffset = byteOffset
        this.byteLength = byteLength
    }
    // === Int8 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt8(byteOffset: number): number {
        return this.getInt8(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt8(byteOffset: int): byte {
        return this.getInt8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: double, value: number): void {
        this.setInt8(byteOffset as int, value as byte)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt8(byteOffset: int, value: byte): void {
        this.setInt8Big(byteOffset, value)
    }
    private getInt8Big(byteOffset: int): byte {
        if (byteOffset + 1 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: byte = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = this.buffer.at(startByte + 0 - i) as byte;
            res = (res | byteVal << (8 * i)) as byte;
        }
        return res
    }
    private setInt8Big(byteOffset: int, value: byte): void {
        if (byteOffset + 1.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 0 - i,  byteVal)
        }
    }
    // === Uint8 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint8(byteOffset: number): number {
        return this.getUint8(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint8(byteOffset: int): byte {
        return this.getUint8Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: double, value: number): void {
        this.setUint8(byteOffset as int, value as byte)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint8(byteOffset: int, value: byte): void {
        this.setUint8Big(byteOffset, value)
    }
    private getUint8Big(byteOffset: int): byte {
        if (byteOffset + 1 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: byte = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = this.buffer.at(startByte + 0 - i) as byte;
            res = (res | byteVal << (8 * i)) as byte;
        }
        return res
    }
    private setUint8Big(byteOffset: int, value: byte): void {
        if (byteOffset + 1.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 1; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 0 - i,  byteVal)
        }
    }
    // === Int16 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt16(byteOffset: number): number {
        return this.getInt16(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt16(byteOffset: int): short {
        return this.getInt16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt16(byteOffset: double, value: number): void {
        this.setInt16(byteOffset as int, value as short)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setInt16(byteOffset: int, value: short): void {
        this.setInt16Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setInt16(byteOffset as int, value as short)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setInt16(byteOffset: int, value: short, littleEndian: boolean): void {
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
    public getInt16(byteOffset: number, littleEndian: boolean): number {
        return this.getInt16(byteOffset as int, littleEndian)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getInt16(byteOffset: int, littleEndian: boolean): short {
        if (littleEndian) {
            return this.getInt16Little(byteOffset)
        } else {
            return this.getInt16Big(byteOffset)
        }
    }
    private getInt16Little(byteOffset: int): short {
        if (byteOffset + 2 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: short = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.buffer.at(startByte + i) as short;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as short;
        }
        return res
    }
    private setInt16Little(byteOffset: int, value: short): void {
        if (byteOffset + 2.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getInt16Big(byteOffset: int): short {
        if (byteOffset + 2 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: short = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.buffer.at(startByte + 1 - i) as short;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as short;
        }
        return res
    }
    private setInt16Big(byteOffset: int, value: short): void {
        if (byteOffset + 2.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 1 - i,  byteVal)
        }
    }
    // === Uint16 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint16(byteOffset: number): number {
        return this.getUint16(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint16(byteOffset: int): short {
        return this.getUint16Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint16(byteOffset: double, value: number): void {
        this.setUint16(byteOffset as int, value as short)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint16(byteOffset: int, value: short): void {
        this.setUint16Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setUint16(byteOffset as int, value as short)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint16(byteOffset: int, value: short, littleEndian: boolean): void {
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
    public getUint16(byteOffset: number, littleEndian: boolean): number {
        return this.getUint16(byteOffset as int, littleEndian)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint16(byteOffset: int, littleEndian: boolean): short {
        if (littleEndian) {
            return this.getUint16Little(byteOffset)
        } else {
            return this.getUint16Big(byteOffset)
        }
    }
    private getUint16Little(byteOffset: int): short {
        if (byteOffset + 2 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: short = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.buffer.at(startByte + i) as short;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as short;
        }
        return res
    }
    private setUint16Little(byteOffset: int, value: short): void {
        if (byteOffset + 2.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getUint16Big(byteOffset: int): short {
        if (byteOffset + 2 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: short = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = this.buffer.at(startByte + 1 - i) as short;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as short;
        }
        return res
    }
    private setUint16Big(byteOffset: int, value: short): void {
        if (byteOffset + 2.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 2; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 1 - i,  byteVal)
        }
    }
    // === Int32 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getInt32(byteOffset: number): number {
        return this.getInt32(byteOffset as int)
    }

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
    public setInt32(byteOffset: double, value: number): void {
        this.setInt32(byteOffset as int, value as int)
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
    public setInt32(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setInt32(byteOffset as int, value as int)
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
    public getInt32(byteOffset: number, littleEndian: boolean): number {
        return this.getInt32(byteOffset as int, littleEndian)
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
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private setInt32Little(byteOffset: int, value: int): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getInt32Big(byteOffset: int): int {
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + 3 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private setInt32Big(byteOffset: int, value: int): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 3 - i,  byteVal)
        }
    }
    // === Uint32 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint32(byteOffset: number): number {
        return this.getUint32(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getUint32(byteOffset: int): int {
        return this.getUint32Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint32(byteOffset: double, value: number): void {
        this.setUint32(byteOffset as int, value as int)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setUint32(byteOffset: int, value: int): void {
        this.setUint32Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setUint32(byteOffset as int, value as int)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setUint32(byteOffset: int, value: int, littleEndian: boolean): void {
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
    public getUint32(byteOffset: number, littleEndian: boolean): number {
        return this.getUint32(byteOffset as int, littleEndian)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getUint32(byteOffset: int, littleEndian: boolean): int {
        if (littleEndian) {
            return this.getUint32Little(byteOffset)
        } else {
            return this.getUint32Big(byteOffset)
        }
    }
    private getUint32Little(byteOffset: int): int {
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private setUint32Little(byteOffset: int, value: int): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getUint32Big(byteOffset: int): int {
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + 3 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return res
    }
    private setUint32Big(byteOffset: int, value: int): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 3 - i,  byteVal)
        }
    }
    // === Float32 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat32(byteOffset: number): number {
        return this.getFloat32(byteOffset as int)
    }

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
    public setFloat32(byteOffset: double, value: number): void {
        this.setFloat32(byteOffset as int, value as float)
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
    public setFloat32(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setFloat32(byteOffset as int, value as float)
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
    public getFloat32(byteOffset: number, littleEndian: boolean): number {
        return this.getFloat32(byteOffset as int, littleEndian)
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
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return Float.bitCastFromInt(res)
    }
    private setFloat32Little(byteOffset: int, value: float): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getFloat32Big(byteOffset: int): float {
        if (byteOffset + 4 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: int = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = this.buffer.at(startByte + 3 - i) as int;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as int;
        }
        return Float.bitCastFromInt(res)
    }
    private setFloat32Big(byteOffset: int, value: float): void {
        if (byteOffset + 4.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = Float.bitCastToInt(value);
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 4; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 3 - i,  byteVal)
        }
    }
    // === Int64 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigInt64(byteOffset: number): Long {
        return this.getBigInt64(byteOffset as int)
    }

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
    public setBigInt64(byteOffset: double, value: Long): void {
        this.setBigInt64(byteOffset as int, value as long)
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
    public setBigInt64(byteOffset: double, value: Long, littleEndian: boolean): void {
        this.setBigInt64(byteOffset as int, value as long)
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
    public getBigInt64(byteOffset: number, littleEndian: boolean): Long {
        return this.getBigInt64(byteOffset as int, littleEndian)
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
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private setBigInt64Little(byteOffset: int, value: long): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getBigInt64Big(byteOffset: int): long {
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private setBigInt64Big(byteOffset: int, value: long): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 7 - i,  byteVal)
        }
    }
    // === Uint64 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getBigUint64(byteOffset: number): Long {
        return this.getBigUint64(byteOffset as int)
    }

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
    public setBigUint64(byteOffset: double, value: Long): void {
        this.setBigUint64(byteOffset as int, value as long)
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
    public setBigUint64(byteOffset: double, value: Long, littleEndian: boolean): void {
        this.setBigUint64(byteOffset as int, value as long)
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
    public getBigUint64(byteOffset: number, littleEndian: boolean): Long {
        return this.getBigUint64(byteOffset as int, littleEndian)
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
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private setBigUint64Little(byteOffset: int, value: long): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getBigUint64Big(byteOffset: int): long {
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return res
    }
    private setBigUint64Big(byteOffset: int, value: long): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = value;
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 7 - i,  byteVal)
        }
    }
    // === Float64 ===
    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat64(byteOffset: number): number {
        return this.getFloat64(byteOffset as int)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @returns read value (big endian)
     */
    public getFloat64(byteOffset: int): double {
        return this.getFloat64Big(byteOffset)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat64(byteOffset: double, value: number): void {
        this.setFloat64(byteOffset as int, value as double)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write (big endian)
     */
    public setFloat64(byteOffset: int, value: double): void {
        this.setFloat64Big(byteOffset, value)
    }
    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat64(byteOffset: double, value: number, littleEndian: boolean): void {
        this.setFloat64(byteOffset as int, value as double)
    }

    /**
     * Sets bytes as they represent given type
     * @param byteOffset zero index to write
     * @param littleEndian read as little or big endian
     */
    public setFloat64(byteOffset: int, value: double, littleEndian: boolean): void {
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
    public getFloat64(byteOffset: number, littleEndian: boolean): number {
        return this.getFloat64(byteOffset as int, littleEndian)
    }

    /**
     * Read bytes as they represent given type
     * @param byteOffset zero index to read
     * @param littleEndian read as little or big endian
     * @returns read value
     */
    public getFloat64(byteOffset: int, littleEndian: boolean): double {
        if (littleEndian) {
            return this.getFloat64Little(byteOffset)
        } else {
            return this.getFloat64Big(byteOffset)
        }
    }
    private getFloat64Little(byteOffset: int): double {
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return Double.bitCastFromLong(res)
    }
    private setFloat64Little(byteOffset: int, value: double): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + i,  byteVal)
        }
    }
    private getFloat64Big(byteOffset: int): double {
        if (byteOffset + 8 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let res: long = 0
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = this.buffer.at(startByte + 7 - i) as long;
            byteVal &= 0xff
            res = (res | byteVal << (8 * i)) as long;
        }
        return Double.bitCastFromLong(res)
    }
    private setFloat64Big(byteOffset: int, value: double): void {
        if (byteOffset + 8.0 >= this.byteLength) {
            throw new RangeError("wrong index")
        }
        let bits = Double.bitCastToLong(value);
        const startByte = this.byteOffset + byteOffset
        for (let i = 0; i < 8; i++) {
            let byteVal = ((bits >>> (i * 8)) & 0xff) as byte
            this.buffer.set(startByte + 7 - i,  byteVal)
        }
    }
}
