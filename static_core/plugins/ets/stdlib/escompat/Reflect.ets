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

package escompat

export class Reflect {

    private constructor () {}

    /**
    * Gets the field of target, equivalent to target.key
    *
    * @param target the target object on which to get the field
    *
    * @param key the string name of the field to get
    *
    * @returns the value of the field
    */
    public static get(target: Object, key: string): Object | undefined {
        if (target instanceof Char ||
            target instanceof Boolean ||
            target instanceof Byte ||
            target instanceof Short ||
            target instanceof Int ||
            target instanceof Long ||
            target instanceof Float ||
            target instanceof Double ||
            target instanceof String) {
            throw new Error("`target` argument of Reflect.get must have fields")
        }
        if (!Reflect.has(target, key)) {
            return undefined
        }
        const t = Type.of(target)
        if (t instanceof ClassType) {
            return (Value.of(target) as ClassValue).getFieldByName(key).getData()!
        } else if (t instanceof ArrayType) {
            if (key == ARRAY_LENGTH_MEMBER_NAME) {
                return new Number((Value.of(target) as ArrayValue).getLength())
            }
        } else if (t instanceof LambdaType) {
            if (key == FUNCTION_LENGTH_MEMBER_NAME) {
                return new Number((t as FunctionType).getParametersNum())
            } else if (key == FUNCTION_NAME_MEMBER_NAME) {
                return ""
            }
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
        return undefined
    }

    /**
    * Gets the element of target, equivalent to target[index]
    *
    * @param target the target object on which to get the element
    *
    * @param key the number index of the element to get
    *
    * @returns the value of the element
    */
    public static get(target: Object, index: number): Object | undefined {
        if (target instanceof Char ||
            target instanceof Boolean ||
            target instanceof Byte ||
            target instanceof Short ||
            target instanceof Int ||
            target instanceof Long ||
            target instanceof Float ||
            target instanceof Double ||
            target instanceof String) {
            throw new Error("`target` argument of Reflect.get must be indexed")
        }
        if (!Reflect.has(target, index)) {
            return undefined
        }
        let t = Type.of(target)
        if (t instanceof ArrayType) {
            let av = Value.of(target) as ArrayValue
            return av.getElement(index as long).getData()!
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
        return undefined
    }

    /**
    * Sets the field of target, equivalent to target.key = value
    *
    * @param target the target object on which to set the field
    *
    * @param key the name of the field to set
    *
    * @param value the value to set.
    *
    * @returns a Boolean indicating whether or not setting the field was successful
    */
    public static set(target: Object, key: string, value: Object): Boolean {
        if (!Reflect.has(target, key)) {
            return false
        }
        let v = Value.of(target)
        let vt = v.getType()
        if (vt instanceof ClassType) {
            let cv = v as ClassValue
            try {
                cv.setFieldByName(key, Value.of(value))
            } catch (e: Error) {
                return false
            }
            return true
        }
        return false
    }

    /**
    * Sets the element of target, equivalent to target[index] = value
    *
    * @param target the target object on which to set the element
    *
    * @param index the index of the element to set
    *
    * @param value the value to set.
    *
    * @returns a boolean indicating whether or not setting the element was successful
    */
    public static set(target: Object, index: number, value: Object): Boolean {
        let v = Value.of(target)
        let vt = v.getType()
        if (vt instanceof ArrayType) {
            if (!Reflect.has(target, index)) {
                return false
            }
            let av = v as ArrayValue
            try {
                av.setElement(index as long, Value.of(value))
            } catch (e: Error) {
                return false
            }
            return true
        }
        return false
    }

    /**
    * Returns the names of the own fields of an object
    *
    * @param o object that contains the own fields
    *
    * @returns array representation of names
    */
    public static ownKeys(target: Object): string[] {
        return Object.getOwnPropertyNames(target)
    }

    /**
    * Determines whether an object has a member with the specified name
    *
    * @param target an object
    *
    * @param key a member name
    *
    * @returns a boolean indicating whether or not the target has the field
    */
    public static has(target: Object, key: string): boolean {
        if (!Object.hasOwn(target, key)) {
            const t = Type.of(target)
            if (t instanceof ClassType) {
                const ct = t as ClassType
                const mnum = ct.getMethodsNum()
                for (let i = 0; i < mnum; ++i) {
                    const m = ct.getMethod(i)
                    if (m.isStatic()) {
                        continue
                    }
                    if (m.getName() == key) {
                        return true
                    }
                }
            } else if (t instanceof StringType || t instanceof ArrayType || t instanceof LambdaType) {
                return (key == OBJECT_TO_STRING_MEMBER_NAME ||
                    key == OBJECT_TO_LOCALE_STRING_MEMBER_NAME ||
                    key == OBJECT_HAS_OWN_PROPERTY_MEMBER_NAME)
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
        return true
    }

    /**
    * Determines whether an object has a field with the specified number
    *
    * @param target an object
    *
    * @param index an element index
    *
    * @returns a boolean indicating whether or not the target has the element
    */
    public static has(target: Object, index: number): boolean {
        return Object.hasOwn(target, index)
    }
}

