/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

 export namespace json {

    export type Transformer = (key: string, value: NullishType) => NullishType

    export enum BigIntMode {
        DEFAULT = 0,
        PARSE_AS_BIGINT = 1,
        ALWAYS_PARSE_AS_BIGINT = 2
    }

    export interface ParseOptions {
        bigIntMode: BigIntMode
    }

    /**
     * Checks an object to have a property
     *
     * @param obj: Object An object to be converted.
     *
     * @param property string property name to be checked
     *
     * @returns boolean
     */
    export function has(obj: Object, property: string): boolean {
        if (!Object.hasOwn(obj, property)) {
            const t = Type.of(obj)
            if (t instanceof ClassType) {
                const ct = t as ClassType
                const mnum = ct.getMethodsNum()
                for (let i = 0; i < mnum; ++i) {
                    const m = ct.getMethod(i)
                    if (m.isStatic()) {
                        continue
                    }

                    const methodName = m.getName()
                    if (methodName == property) {
                        return true
                    } else if (methodName == "<get>" + property) {
                        return true
                    } else if (methodName == "<set>" + property) {
                        return true
                    }
                }
            } else if (t instanceof StringType || t instanceof ArrayType || t instanceof LambdaType) {
                return (property == OBJECT_TO_STRING_MEMBER_NAME ||
                    property == OBJECT_TO_LOCALE_STRING_MEMBER_NAME ||
                    property == OBJECT_HAS_OWN_PROPERTY_MEMBER_NAME)
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
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: NullishType An object to be converted.
     *
     * @param replacer An array with elements indicating names of the properties in the object
     * or Transformer function changing field values
     * that should be included in the resulting JSON string
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object
     */
    export function stringify(value: NullishType, replacer?: Transformer | ((double | string)[]) | null,
                              space?: string | int): string {
        if ((replacer === null) || (replacer === undefined)) {
            return JSON.stringify(value, undefined, space)
        } else if (replacer! instanceof Transformer) {
            return JSON.stringify(value, replacer! as Transformer, space)
        }
        return JSON.stringify(value, replacer! as (double | string)[], space)
    }

    /**
     * Converts a JavaScript Object Notation (JSON) string to an Object.
     *
     * @param text: string JSON representation of an object.
     *
     * @param type: Type of parsed object
     *
     * @param reviver A function that transforms the results
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns Object - parsed object
     */
    export function parse<T>(text: string, type: Type, reviver?: Transformer, options?: ParseOptions): T | null | undefined {
        let opt: int = options ? options.bigIntMode as int : 0
        return JSON.parse<T>(text, reviver, type, opt)
    }

 }
