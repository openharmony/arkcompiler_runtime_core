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

package std.core;

/**
 * @deprecated
*/
export type NullishType = Any

/**
 * @deprecated
*/
export type NullableType = Object | null

/**
 * @deprecated
*/
export type Nullish<T> = T | null | undefined

export type PropertyKey = string

export const ARRAY_LENGTH_MEMBER_NAME      = "length"
export const FUNCTION_LENGTH_MEMBER_NAME   = "length"
export const FUNCTION_NAME_MEMBER_NAME     = "name"

export const OBJECT_TO_STRING_MEMBER_NAME: string          = "toString"
export const OBJECT_TO_LOCALE_STRING_MEMBER_NAME: string   = "toLocaleString"
export const OBJECT_HAS_OWN_PROPERTY_MEMBER_NAME: string   = "hasOwnProperty"

type EntryType = [PropertyKey, NullishType]
type NullishEntryType = EntryType | null | undefined

/**
 * `object` is an alias for type `Object`
 */
export type object = Object;

/**
 * Common ancestor amongst all other classes
 */
export class Object {
    /**
    * Constructs a new blank Object
    */
    constructor () {};

    /**
    * Converts this object to a string
    *
    * @returns result of the conversion
    */
    public toString(): String {
        return Value.of(this).toString()
    }

    /**
    * Converts this object to locale-specific string representation
    *
    * @returns result of the conversion
    */
    public toLocaleString(): String {
        return Value.of(this).toLocaleString()
    }

    /**
    * Returns a hash code (integer representation) for this instance
    *
    * @returns representation of this instance
    */
    public $_hashCode(): int {
        return Runtime.getHashCode(this);
    }

    /**
     * Returns an array of a given record property names
     *
     * @param rec a record
     *
     * @returns an array of strings representing the given record's property names
     */
    public static keys(rec: Record<PropertyKey, NullishType>): string[] {
        const keys: string[] = new string[rec.size.toInt()]

        let i = 0
        for (const key of rec.keys()) {
            keys[i] = key
            i++
        }

        return keys
    }

    private static generateKeyArray(length: Number): string[] {
        const len = length.toInt();
        if (len == 0) {
            return new string[0]
        }
        let res: string[] = new string[len]
        for (let i = 0; i < len; i++) {
            res[i] = new Int(i).toString()
        }
        return res
    }

    /**
    * Returns the names of the fields of an object
    *
    * @param o an object
    *
    * @returns an array of strings representing the given object's own string-keyed field keys.
    */
    public static keys(o: Object): string[] {
        // Char, Boolean and Numeric types doesn't have keys
        if (o instanceof Char ||
            o instanceof Boolean ||
            o instanceof Byte ||
            o instanceof Short ||
            o instanceof Int ||
            o instanceof Long ||
            o instanceof Float ||
            o instanceof Double) {
            return new string[0]
        }
        // "Keys" for the string type is enumeration from 0 to str.length - 1
        if (o instanceof String) {
            return Object.generateKeyArray(o.getLength())
        } else if (o instanceof Array) {
            return Object.generateKeyArray(o.length)
        }
        const t = Type.of(o)
        if (t instanceof ClassType) {
            const ct = t as ClassType
            const fnum = ct.getFieldsNum()
            if (fnum == 0) {
                return new string[0]
            }
            let n: int = 0
            for (let i = 0; i < fnum; i++) {
                if (!ct.getField(i).isStatic()) {
                    n++
                }
            }
            let res: string[] = new string[n]
            let j: int = 0
            for (let i = 0; i < fnum; i++) {
                let f = ct.getField(i)
                if (!f.isStatic()) {
                    res[j] = f.getName()
                    j++
                }
            }
            return res
        } else if (t instanceof ArrayType) {
            const av = Value.of(o) as ArrayValue
            return Object.generateKeyArray(av.getLength())
        } else if (t instanceof LambdaType) {
            return new string[0]
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }

        throw new AssertionError("Invalid object type");
    }

    public static values(rec: Record<PropertyKey, NullishType>): NullishType[] {
        const vals: NullishType[] = new NullishType[rec.size.toInt()]

        let i = 0
        for (let val of rec.values()) {
            vals[i] = val
            i++
        }

        return vals
    }

    /**
    * Returns the values of the fields of an object
    *
    * @param o an object
    *
    * @returns an array containing the given object's own string-keyed field values
    */
    public static values(o: Object): NullishType[] {
        if (o instanceof Char ||
            o instanceof Boolean ||
            o instanceof Byte ||
            o instanceof Short ||
            o instanceof Int ||
            o instanceof Long ||
            o instanceof Float ||
            o instanceof Double) {
            return new NullishType[0]
        }
        if (o instanceof String) {
            const sv = o as string
            const len = sv.getLength()
            if (len == 0) {
                return new NullishType[0]
            }
            let res: NullishType[] = new NullishType[len]
            for (let i = 0; i < len; i++) {
                // NOTE(shumilov-petr): must be replaced by `sv.charAt(i) as string` when #15731 will be fixed
                res[i] = new Char(sv.charAt(i)).toString()
            }
            return res
        }
        const t = Type.of(o)
        if (t instanceof ClassType) {
            const cv = Value.of(o) as ClassValue
            if (cv.getFieldsNum() == 0) {
                return new NullishType[0]
            }
            const keys = Object.keys(o)
            const len = keys.length
            let res: NullishType[] = new NullishType[len]
            for (let i = 0; i < len; i++) {
                res[i] = cv.getFieldByName(keys[i]).getData()
            }
            return res
        } else if (t instanceof ArrayType) {
            const av = Value.of(o) as ArrayValue
            const len = av.getLength()
            if (len == 0) {
                return new NullishType[0]
            }
            let res: NullishType[] = new NullishType[len.toInt()]
            for (let i = 0; i < len; i++) {
                res[i] = av.getElement(i).getData()
            }
            return res
        } else if (t instanceof LambdaType) {
            return new NullishType[0]
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }

        throw new AssertionError("Invalid object type")
    }

    /**
    * Returns an array of key/values of properties of a record
    *
    * @param rec record that contains the fields
    *
    * @returns array representation of key/value
    */
    public static entries(rec: Record<PropertyKey, NullishType>): NullishEntryType[] {
        const entries: NullishEntryType[] = new NullishEntryType[rec.size.toInt()]

        let i = 0
        for (const entry of rec.entries()) {
            entries[i] = entry
            i++
        }

        return entries
    }

    /**
    * Returns an array of key/values of properties of an object
    *
    * @param o object that contains the fields
    *
    * @returns array representation of key/value
    */
    public static entries(o: Object): NullishEntryType[] {
        if (o instanceof Char ||
            o instanceof Boolean ||
            o instanceof Byte ||
            o instanceof Short ||
            o instanceof Int ||
            o instanceof Long ||
            o instanceof Float ||
            o instanceof Double) {
            return new NullishEntryType[0]
        }
        if (o instanceof String) {
            const sv = o as string
            const len = sv.getLength()
            if (len == 0) {
                return new NullishEntryType[0]
            }
            let res: NullishEntryType[] = new NullishEntryType[len]
            for (let i = 0; i < len; i++) {
                res[i] = [new Int(i).toString(), new Char(sv.charAt(i)).toString()] as EntryType
            }
            return res
        }
        const t = Type.of(o)
        if (t instanceof ClassType) {
            const cv = Value.of(o) as ClassValue
            if (cv.getFieldsNum() == 0) {
                return new NullishEntryType[0]
            }
            const keys = Object.keys(o)
            const len = keys.length
            let res: NullishEntryType[] = new NullishEntryType[len]
            for (let i = 0; i < len; i++) {
                res[i] = [keys[i], cv.getFieldByName(keys[i]).getData()] as EntryType
            }
            return res
        } else if (t instanceof ArrayType) {
            const av = Value.of(o) as ArrayValue
            const len = av.getLength()
            if (len == 0) {
                return new NullishEntryType[0]
            }
            let res: NullishEntryType[] = new NullishEntryType[len.toInt()]
            for (let i = 0; i < len; i++) {
                res[i] = [new Int(i).toString(), av.getElement(i).getData()] as EntryType
            }
            return res
        } else if (t instanceof LambdaType) {
            return new NullishEntryType[0]
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }

        throw new AssertionError("Invalid object type")
    }

    public static getOwnPropertyNames(rec: Record<PropertyKey, NullishType>): string[] {
        return Object.keys(rec)
    }

    /**
    * Returns the names of the own fields of an object
    *
    * @param o object that contains the own fields
    *
    * @returns array representation of names
    */
    public static getOwnPropertyNames(o: Object): string[] {
        if (o instanceof Char ||
            o instanceof Boolean ||
            o instanceof Byte ||
            o instanceof Short ||
            o instanceof Int ||
            o instanceof Long ||
            o instanceof Float ||
            o instanceof Double) {
            return new string[0]
        }
        const t = Type.of(o)
        if (t instanceof StringType || t instanceof ArrayType || o instanceof Array) {
            const keys = Object.keys(o)
            const len = keys.length
            let res: string[] = new string[len + 1]
            for (let i = 0; i < len; i++) {
                res[i] = keys[i]
            }
            res[len] = ARRAY_LENGTH_MEMBER_NAME
            return res
        }
        if (t instanceof ClassType) {
            return Object.keys(o)
        } else if (t instanceof LambdaType) {
            return [FUNCTION_LENGTH_MEMBER_NAME, FUNCTION_NAME_MEMBER_NAME]
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }

        throw new AssertionError("Invalid object type")
    }

    /**
    * Determines whether an object has a field with the specified name
    *
    * @param key the string name of the field to test
    *
    * @returns true if the object has the specified field; false otherwise
    */
    public hasOwnProperty(key: string): boolean {
        const keys = Object.getOwnPropertyNames(this)
        const len = keys.length
        for(let i = 0; i < len; i++) {
            if (keys[i] == key) {
                return true
            }
        }
        return false
    }

    /**
    * Determines whether an object has a element with the specified index
    *
    * @param index the number index of the element to test
    *
    * @returns true if the object has the specified element; false otherwise
    */
    public hasOwnProperty(index: number): boolean {
        if ((this) instanceof String) {
            const sv = this as String
            const len = sv.getLength()
            const idx = index.toLong()
            return (0 <= idx && idx < len)
        }
        const t = Type.of(this)
        if (t instanceof ArrayType) {
            const av = Value.of(this) as ArrayValue
            const len = av.getLength()
            const idx = index.toLong()
            return (0 <= idx && idx < len)
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }
        return false
    }

    /**
    * Determines whether an object has a field with the specified name
    *
    * @param target an object
    *
    * @param key the string name of the field to test
    *
    * @returns true if the object has the specified field; false otherwise
    */
    public static hasOwn(target: Object, key: PropertyKey): boolean {
        return target.hasOwnProperty(key)
    }

    /**
    * Determines whether an object has a element with the specified index
    *
    * @param target an object
    *
    * @param index the number index of the element to test
    *
    * @returns true if the object has the specified element; false otherwise
    */
    public static hasOwn(target: Object, index: number): boolean {
        return target.hasOwnProperty(index)
    }

    /**
    * Transforms key-value pairs into a Record.
    *
    * @param entries an Iterable object that contains key-value pairs
    *
    * @returns new Record whose properties are given by the elements of the Iterable
    */
    public static fromEntries<T = NullishType>(entries: Iterable<[PropertyKey, T]>): Record<PropertyKey, T> {
        const result = new Record<PropertyKey, T>()

        const entriesIter = entries.$_iterator()

        let entriesIterResult = entriesIter.next()
        while (!entriesIterResult.done) {
            const entry: [PropertyKey, T] | undefined = entriesIterResult.value
            if (entry != undefined) {
                result[entry[0] as PropertyKey] = entry[1] as T
            }

            entriesIterResult = entriesIter.next()
        }

        return result
    }

    /**
    * Copies all own properties from one or more source objects to a target Record.
    *
    * @param target target Record -- what to apply the sources' properties to
    *
    * @param source objects containing the properties you want to apply
    *
    * @returns the modified target Record
    */
    public static assign(target: Record<PropertyKey, NullishType>,  ...source: FixedArray<Object>): Record<PropertyKey, NullishType> {
        for (const s of source) {
            const entries = Object.entries(s)
            for (const e of entries) {
                target.set(e![0] as PropertyKey, e![1] as NullishType)
            }
        }
        return target
    }
}
