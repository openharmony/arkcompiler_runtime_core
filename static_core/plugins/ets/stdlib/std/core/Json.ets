/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

export namespace jsonx {
    /**
     * Enumeration of possible JSON value types.
     * Used to identify the type of a JsonElement at runtime.
     */
    export enum JsonType {
        // There is no value (as distinct from Null).
        JsonUndefined = 0,
        // A JSON object.
        JsonObject = 1,
        // A JSON array.
        JsonArray = 2,
        // A JSON string.
        JsonString = 3,
        // A JSON number.
        JsonNumber = 4,
        // A JSON value true.
        JsonTrue = 5,
        // A JSON value false.
        JsonFalse = 6,
        // A JSON value null.
        JsonNull = 7,
    }

    /**
     * Base error class for JSON-related errors.
     * Thrown when general JSON parsing or manipulation errors occur.
     */
    export class JsonError extends Error {
        public constructor(msg: string) {
            super(msg)
        }
    }

    /**
     * Error thrown when attempting to access a JSON element with an incompatible type.
     * For example, trying to get a string value from a number element.
     */
    export class JsonTypeError extends JsonError {
        public constructor(msg: string) {
            super(msg)
        }
    }

    /**
     * Iterator implementation for JSON object entries.
     * Provides iteration over key-value pairs in a JSON object.
     *
     * @internal
     */
    class JsonObjectIteratorEntries implements IterableIterator<[string, JsonElement]> {
        private parent: Map<string, JsonElement>
        private iterator: IterableIterator<[string, JsonElement]>
        private idx: int = 0

        constructor(parent: Map<string, JsonElement>) {
            this.parent = parent
            this.iterator = parent.entries()
        }

        public override $_iterator(): IterableIterator<[string, JsonElement]> {
            return this
        }

        override next(): IteratorResult<[string, JsonElement]> {
            return this.iterator.next()
        }
    }

    /**
     * Core class representing a JSON element that can hold any valid JSON value.
     * Provides type-safe access to JSON values with both strict and lenient APIs.
     *
     * The class maintains an invariant that only one type of value can be set at a time.
     * Attempting to set multiple values will result in a JsonTypeError.
     *
     * @example
     * ```ets
     * const element = new JsonElement();
     * element.setString("Hello");
     * console.log(element.asString()); // "Hello"
     * ```
     */
    export class JsonElement {
        private key: string = ""

        /// Can be in the following states:
        ///
        /// 0. Undefined (Null) <=> forall F in M: F is undefined
        ///
        /// where M is the following set:
        /// - maybeStringValue (ms)
        /// - maybeDoubleValue (md)
        /// - maybeIntegerValue (mi)
        /// - maybeBoolValue (mb)
        /// - maybeObjectValues (mo)
        /// - maybeArrayValues (ma)
        ///
        /// 1. Object <=> maybeObjectValues is defined and
        ///     forall f in M: f / {mo} is undefined
        ///
        /// 2. Array <=> maybeArrayValues is defined and
        ///     forall f in M: f / {ma} is undefined
        ///
        /// 3. String <=> maybeStringValue is defined and
        ///     forall f in M: f / {ms} is undefined
        ///
        /// 4. Double <=> maybeDoubleValue is defined and
        ///     forall f in M: f / {md} is undefined
        ///
        /// 5. Integer <=> maybeIntegerValue is defined and
        ///     forall f in M: f / {mi} is undefined
        ///
        /// 6. Boolean <=> maybeBoolValue is defined and
        ///     forall f in M: f / {mb} is undefined
        ///
        /// 7. Null <=> maybeNullValue is defined and
        ///     forall f in M: f / {mn} is undefined
        ///
        /// Invariant:
        ///     forall f in M: f is undefined (undefined state)
        /// Or:
        ///     exists f in M: f is defined (defined state) and forall g in M / {f} is undefined
        ///
        private maybeNullValue?:    null
        private maybeStringValue?:  string
        private maybeDoubleValue?:  double
        private maybeIntegerValue?: int
        private maybeBoolValue?:    boolean
        private maybeObjectValues?: Map<string, JsonElement>
        private maybeArrayValues?:  Array<JsonElement>

        /**
         * Gets the type of the JSON element.
         * @returns {JsonType} The type of the current JSON value
         */
        get jsonType(): JsonType {
            if (this.maybeNullValue === null) {
                return JsonType.JsonNull
            }
            if (this.maybeObjectValues != undefined) {
                return JsonType.JsonObject
            }
            if (this.maybeArrayValues != undefined) {
                return JsonType.JsonArray
            }
            if (this.maybeStringValue != undefined) {
                return JsonType.JsonString
            }
            if (this.maybeDoubleValue != undefined) {
                return JsonType.JsonNumber
            }
            if (this.maybeIntegerValue != undefined) {
                return JsonType.JsonNumber
            }
            if (this.maybeBoolValue != undefined) {
                return this.maybeBoolValue ? JsonType.JsonTrue : JsonType.JsonFalse
            }
            return JsonType.JsonUndefined
        }

        /**
         * Gets the key associated with this JSON element.
         * @returns {string} The key of this element
         */
        get jsonKey(): string {
            return this.key
        }

        /**
         * Gets a JSON element from an array by index.
         * @param {int} index - The index to look up
         * @returns {JsonElement} The JSON element associated with the index
         * @throws {JsonTypeError} If this element is not an array
         * @throws {RangeError} If the index is out of range
         */
        $_get(index: int): JsonElement {
            if (this.jsonType != JsonType.JsonArray) {
                throw new JsonTypeError("This value's ValueKind is not Array.")
            }
            if (this.maybeArrayValues == undefined || index < 0 || index >= this.maybeArrayValues!.length) {
                throw new RangeError(`Index ${index} is out of range.`)
            }
            return this.maybeArrayValues![index]
        }

        /**
         * Default parameterless constructor.
         * Creates an undefined JSON element.
         */
        constructor() {}

        /**
         * Copy constructor (deep copy)
         * Creates a new JSON element by copying the values from another JSON element.
         * @param {JsonElement} other - The JSON element to copy from
         */
        constructor(other: JsonElement) {
            this._deepCopy(other)
        }

        /**
         * Creates a new JSON element from a key-value structure.
         * @param {Record<string, JsonElement>} elements - The key-value structure to create from
         */
        constructor(elements: Record<string, JsonElement>) {
            this.maybeObjectValues = new Map<string, JsonElement>()
            for (const key of elements.keys()) {
                this.maybeObjectValues!.set(key, elements[key]!)
            }
        }

        /**
         * Creates a new JSON element containing a string value.
         * @param {string} value - The string value to store
         * @returns {JsonElement} A new JsonElement containing the string value
         */
        static createString(value: string): JsonElement {
            const element = new JsonElement()
            element.maybeStringValue = value
            return element
        }

        /**
         * Creates a new JSON element containing a double value.
         * @param {double} value - The double value to store
         * @returns {JsonElement} A new JsonElement containing the double value
         */
        static createDouble(value: double): JsonElement {
            const element = new JsonElement()
            element.maybeDoubleValue = value
            return element
        }

        /**
         * Creates a new JSON element containing an integer value.
         * @param {int} value - The integer value to store
         * @returns {JsonElement} A new JsonElement containing the integer value
         */
        static createInteger(value: int): JsonElement {
            const element = new JsonElement()
            element.maybeIntegerValue = value
            return element
        }

        /**
         * Creates a new JSON element containing a boolean value.
         * @param {boolean} value - The boolean value to store
         * @returns {JsonElement} A new JsonElement containing the boolean value
         */
        static createBoolean(value: boolean): JsonElement {
            const element = new JsonElement()
            element.maybeBoolValue = value
            return element
        }

        /**
         * Creates a new JSON element containing a null value.
         * @returns {JsonElement} A new JsonElement containing null
         */
        static createNull(): JsonElement {
            const element = new JsonElement()
            element.maybeNullValue = null
            return element
        }

        /**
         * Creates a new JSON element containing an undefined value.
         * @returns {JsonElement} A new JsonElement containing undefined
         */
        static createUndefined(): JsonElement {
            const element = new JsonElement()
            return element
        }

        /**
         * Creates a new JSON element containing an array of JSON elements.
         * @param {Array<JsonElement>} elements - The array of JSON elements to store
         * @returns {JsonElement} A new JsonElement containing the array
         */
        static createArray(elements: Array<JsonElement>): JsonElement {
            const element = new JsonElement()
            element.maybeArrayValues = Array.from<JsonElement>(elements)
            return element
        }

        /**
         * Creates a new JSON element containing an object with key-value pairs.
         * @param {Map<string, JsonElement>} map - The map of key-value pairs to store
         * @returns {JsonElement} A new JsonElement containing the object
         */
        static createObject(map: Map<string, JsonElement>): JsonElement {
            const element = new JsonElement()
            element.maybeObjectValues = new Map<string, JsonElement>(map)
            return element
        }

        /**
         * Gets a JSON element from an object by key.
         * @param {string} key - The key to look up
         * @returns {JsonElement} The JSON element associated with the key
         * @throws {JsonTypeError} If this element is not an object
         * @throws {RangeError} If the key is not found in the object
         */
        $_get(key: string): JsonElement {
            if (this.jsonType != JsonType.JsonObject) {
                throw new JsonTypeError("This value's ValueKind is not Object.")
            }
            if (this.maybeObjectValues == undefined || !this.maybeObjectValues!.has(key)) {
                throw new RangeError(`The key '${key}' was not found in the JSON object.`)
            }
            return this.maybeObjectValues!.get(key)!
        }

        /**
         * Gets a JSON element from an object by key.
         * @param {string} key - The key to look up
         * @returns {JsonElement} The JSON element associated with the key
         * @throws {JsonTypeError} If this element is not an object
         * @throws {RangeError} If the key is not found in the object
         */
        getElement(key: string): JsonElement {
            return this.$_get(key)
        }

        /**
         * Attempts to get a JSON element from an object by key.
         * Returns undefined if the key is not found or if this element is not an object.
         * @param {string} key - The key to look up
         * @returns {JsonElement | undefined} The JSON element if found, undefined otherwise
         */
        tryGetElement(key: string): JsonElement | undefined {
            return this.jsonType == JsonType.JsonObject
                ? this.maybeObjectValues!.get(key)
                : undefined
        }

        /**
         * Iterator over object properties if jsonType == JsonType.JsonObject
         * on type error `JsonTypeError` is raised
         * @returns {IterableIterator<[string, JsonElement]>} An iterator over the object properties
         */
        $_iterator(): IterableIterator<[string, JsonElement]> {
            if (this.jsonType != JsonType.JsonObject) {
                throw new JsonTypeError("This value's ValueKind is not Object.")
            }
            return new JsonObjectIteratorEntries(this.maybeObjectValues!)
        }

        /**
         * Gets a JSON element from an object by key and ensures it is an array.
         * @param {string} key - The key to look up
         * @returns {JsonElement} The JSON element associated with the key
         * @throws {JsonTypeError} If this element is not an object
         * @throws {RangeError} If the key is not found in the object
         */
        getArray(key: string): Array<JsonElement> {
            const element = this.getElement(key)
            if (element.jsonType != JsonType.JsonArray) {
                throw new JsonTypeError("The value associated with the key is not an array.")
            }
            return element.asArray()
        }

        /**
         * Attempts to get a JSON element from an object by key and ensures it is an array.
         * Returns an empty array if the key is not found or if the value is not an array.
         * @param {string} key - The key to look up
         * @returns {Array<JsonElement>} The JSON element if found, an empty array otherwise
         */
        tryGetArray(key: string): Array<JsonElement> {
            const element = this.tryGetElement(key)
            return element?.jsonType == JsonType.JsonArray
                ? element!.asArray()
                : Array<JsonElement>()
        }

        /**
         * Removes a JSON element from an object by key.
         * @param {string} key - The key to remove
         * @returns {boolean} True if the element was removed, false otherwise
         */
        removeElement(key: string): boolean {
            return this.jsonType === JsonType.JsonObject
                ? this.maybeObjectValues!.delete(key)
                : false
        }

        /**
         * Sets a JSON element in an object by key.
         * @param {string} key - The key to set
         * @param {JsonElement} value - The JSON element to set
         * @throws {JsonTypeError} If this element is not an object
         */
        setElement(key: string, value: JsonElement) {
            if (this.jsonType != JsonType.JsonObject) {
                throw new JsonTypeError("This value's ValueKind is not Object.")
            }
            this.maybeObjectValues!.set(key, value)
        }

        /**
         * Gets a double value from an object by key and ensures it is a double.
         * @param {string} key - The key to look up
         * @returns {double} The double value
         * @throws {JsonTypeError} If the value is not a double or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        getDouble(key: string): double {
            return this.getElement(key).asDouble()
        }

        /**
         * Attempts to get a double value from an object by key and ensures it is a double.
         * Returns the fallback value if the key is not found or if the value is not a double.
         * @param {string} key - The key to look up
         * @param {double | undefined} [fallback=undefined] - The fallback value to return if the key is not found
         * @returns {double} The double value if found, fallback value otherwise
         */
        tryGetDouble(key: string, fallback?: double): double | undefined {
            return this.tryGetElement(key)?.tryAsDouble() ?? fallback
        }

        /**
         * Gets an integer value from an object by key and ensures it is an integer.
         * @param {string} key - The key to look up
         * @returns {int} The integer value
         * @throws {JsonTypeError} If the value is not an integer or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        getInteger(key: string): int {
            return this.getElement(key).asInteger()
        }

        /**
         * Attempts to get an integer value from an object by key.
         * Returns the fallback value if the key is not found or if the value is not an integer.
         * @param {string} key - The key to look up
         * @param {int} [fallback=0] - The fallback value to return if the key is not found
         * @returns {int} The integer value if found, fallback value otherwise
         */
        tryGetInteger(key: string, fallback: int = 0): int {
            return this.tryGetElement(key)?.tryAsInteger() ?? fallback
        }

        /**
         * Gets a string value from an object by key.
         * @param {string} key - The key to look up
         * @returns {string} The string value
         * @throws {JsonTypeError} If the value is not a string or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        getString(key: string): string {
            return this.getElement(key).asString()
        }

        /**
         * Attempts to get a string value from an object by key.
         * Returns the fallback value if the key is not found or if the value is not a string.
         * @param {string} key - The key to look up
         * @param {string} [fallback=""] - The fallback value to return if the key is not found
         * @returns {string} The string value if found, fallback value otherwise
         */
        tryGetString(key: string, fallback: string = ""): string {
            return this.tryGetElement(key)?.tryAsString() ?? fallback
        }

        /**
         * Gets a boolean value from an object by key.
         * @param {string} key - The key to look up
         * @returns {boolean} The boolean value
         * @throws {JsonTypeError} If the value is not a boolean or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        getBoolean(key: string): boolean {
            return this.getElement(key).asBoolean()
        }

        /**
         * Attempts to get a boolean value from an object by key.
         * Returns the fallback value if the key is not found or if the value is not a boolean.
         * @param {string} key - The key to look up
         * @param {boolean} [fallback=false] - The fallback value to return if the key is not found
         * @returns {boolean} The boolean value if found, fallback value otherwise
         */
        tryGetBoolean(key: string, fallback: boolean = false): boolean {
            return this.tryGetElement(key)?.tryAsBoolean() ?? fallback
        }

        /**
         * Gets a double value from an object by key.
         * @param {string} key - The key to look up
         * @returns {double} The double value
         * @throws {JsonTypeError} If the value is not a double or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        asDouble(): double {
            if (this.jsonType != JsonType.JsonNumber) {
                throw new JsonTypeError("This value's ValueKind is not Number.")
            }
            if (this.maybeDoubleValue == undefined) {
                throw new JsonTypeError("This value's ValueKind is Number, but the value contains no double value.")
            }
            return this.maybeDoubleValue!
        }

        /**
         * Attempts to get a double value from an object by key.
         * Returns undefined if the key is not found or if the value is not a double.
         * @param {string} key - The key to look up
         * @returns {double} The double value if found, undefined otherwise
         */
        tryAsDouble(): double | undefined {
            return this.jsonType == JsonType.JsonNumber
                ? this.maybeDoubleValue
                : undefined
        }

        /**
         * Gets an integer value from an object by key.
         * @param {string} key - The key to look up
         * @returns {int} The integer value
         * @throws {JsonTypeError} If the value is not an integer or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        asInteger(): int {
            if (this.jsonType != JsonType.JsonNumber) {
                throw new JsonTypeError("This value's ValueKind is not Number.")
            }
            if (this.maybeIntegerValue == undefined) {
                throw new JsonTypeError("This value's ValueKind is Number, but the value contains no integer value.")
            }
            return this.maybeIntegerValue!
        }

        /**
         * Attempts to get an integer value from an object by key.
         * Returns undefined if the key is not found or if the value is not an integer.
         * @param {string} key - The key to look up
         * @returns {int} The integer value if found, undefined otherwise
         */
        tryAsInteger(): int | undefined {
            return this.jsonType == JsonType.JsonNumber
                ? this.maybeIntegerValue
                : undefined
        }

        /**
         * Gets a string value from an object by key.
         * @param {string} key - The key to look up
         * @returns {string} The string value
         * @throws {JsonTypeError} If the value is not a string or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        asString(): string {
            if (this.jsonType != JsonType.JsonString) {
                throw new JsonTypeError("This value's ValueKind is not String.")
            }
            return this.maybeStringValue!
        }

        /**
         * Attempts to get a string value from an object by key.
         * Returns undefined if the key is not found or if the value is not a string.
         * @param {string} key - The key to look up
         * @returns {string} The string value if found, undefined otherwise
         */
        tryAsString(): string | undefined {
            return this.jsonType == JsonType.JsonString
                ? this.maybeStringValue
                : undefined
        }

        /**
         * Gets a boolean value from an object by key.
         * @param {string} key - The key to look up
         * @returns {boolean} The boolean value
         * @throws {JsonTypeError} If the value is not a boolean or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        asBoolean(): boolean {
            if (this.jsonType != JsonType.JsonTrue && this.jsonType != JsonType.JsonFalse) {
                throw new JsonTypeError("This value's ValueKind is neither True nor False.")
            }
            return this.jsonType == JsonType.JsonTrue
        }

        /**
         * Attempts to get a boolean value from an object by key.
         * Returns undefined if the key is not found or if the value is not a boolean.
         * @param {string} key - The key to look up
         * @returns {boolean} The boolean value if found, undefined otherwise
         */
        tryAsBoolean(): boolean | undefined {
            return this.jsonType == JsonType.JsonTrue || this.jsonType == JsonType.JsonFalse
                ? this.maybeBoolValue
                : undefined
        }

        /**
         * Gets an array value from an object by key.
         * @param {string} key - The key to look up
         * @returns {Array<JsonElement>} The array value
         * @throws {JsonTypeError} If the value is not an array or if this element is not an object
         * @throws {RangeError} If the key is not found
         */
        asArray(): Array<JsonElement> {
            if (this.jsonType != JsonType.JsonArray) {
                throw new JsonTypeError("This value's ValueKind is not Array.")
            }
            return this.maybeArrayValues!
        }

        /**
         * Attempts to get an array value from an object by key.
         * Returns undefined if the key is not found or if the value is not an array.
         * @param {string} key - The key to look up
         * @returns {Array<JsonElement>} The array value if found, undefined otherwise
         */
        tryAsArray(): Array<JsonElement> | undefined {
            return this.jsonType == JsonType.JsonArray
                ? this.maybeArrayValues
                : undefined
        }

        /**
         * Sets a current JsonElement to a double value.
         * @param {double} value - The double value to set
         */
        setDouble(value: double): void {
            this.setUndefined()
            this.maybeDoubleValue = value
        }

        /**
         * Sets a current JsonElement to an integer value.
         * @param {int} value - The integer value to set
         */
        setInteger(value: int): void {
            this.setUndefined()
            this.maybeIntegerValue = value
        }

        /**
         * Sets a current JsonElement to a string value.
         * @param {string} value - The string value to set
         */
        setString(value: string): void {
            this.setUndefined()
            this.maybeStringValue = value
        }

        /**
         * Sets a current JsonElement to a boolean value.
         * @param {boolean} value - The boolean value to set
         */
        setBoolean(value: boolean): void {
            this.setUndefined()
            this.maybeBoolValue = value
        }

        /**
         * Sets a current JsonElement to an array value.
         * @param {Array<JsonElement>} value - The array value to set
         */
        setArray(value: Array<JsonElement>): void {
            this.setUndefined()
            this.maybeArrayValues = value
        }

        /**
         * Sets a current JsonElement to a null value.
         */
        setNull(): void {
            this.setUndefined()
            this.maybeNullValue = null
        }

        /**
         * Sets a current JsonElement to an undefined value.
         */
        setUndefined(): void {
            this.maybeStringValue  = undefined
            this.maybeDoubleValue  = undefined
            this.maybeIntegerValue = undefined
            this.maybeBoolValue    = undefined
            this.maybeObjectValues = undefined
            this.maybeArrayValues  = undefined
            this.maybeNullValue    = undefined
        }

        private _ensureInvariants(): void {
            const typeMap = new Map<string, boolean>([
                ["Null",    this.jsonType == JsonType.JsonNull],
                ["Object",  this.jsonType == JsonType.JsonObject],
                ["Array",   this.jsonType == JsonType.JsonArray],
                ["Number",  this.jsonType == JsonType.JsonNumber],
                ["String",  this.jsonType == JsonType.JsonString],
                ["Boolean", this.jsonType == JsonType.JsonTrue || this.jsonType == JsonType.JsonFalse]
            ])
            const activeTypes = Array.from(typeMap.entries())
                .filter((entry: [string, boolean]): boolean => entry[1])
                .map((entry: [string, boolean]): string => entry[0])
            if (activeTypes.length == 0 && this.jsonType != JsonType.JsonUndefined) {
                throw new JsonTypeError(`Invalid JsonElement type. No valid type detected.`)
            }
            if (activeTypes.length > 1) {
                throw new JsonTypeError(`Invalid JsonElement type. Multiple types detected: ${activeTypes.join(", ")}`)
            }
        }

        /**
         * Creates a deep copy of a JsonElement.
         * @param {JsonElement} source - The JsonElement to copy from
         * @private
         */
        private _deepCopy(source: JsonElement): void {
            this.key = source.key
            this.maybeStringValue = source.maybeStringValue
            this.maybeDoubleValue = source.maybeDoubleValue
            this.maybeIntegerValue = source.maybeIntegerValue
            this.maybeBoolValue = source.maybeBoolValue

            // Deep copy object values recursively
            if (source.maybeObjectValues) {
                this.maybeObjectValues = new Map<string, JsonElement>()
                source.maybeObjectValues!.forEach((value, key) => {
                    const copiedElement = new JsonElement()
                    copiedElement._deepCopy(value)
                    this.maybeObjectValues!.set(key, copiedElement)
                })
            } else {
                this.maybeObjectValues = undefined
            }

            // Deep copy array values recursively
            if (source.maybeArrayValues) {
                this.maybeArrayValues = Array<JsonElement>()
                for (const element of source.maybeArrayValues!) {
                    const copiedElement = new JsonElement()
                    copiedElement._deepCopy(element)
                    this.maybeArrayValues!.push(copiedElement)
                }
            } else {
                this.maybeArrayValues = undefined
            }

            this._ensureInvariants()
        }
    }

    /**
     * Interface for types that can be deserialized from JSON.
     * Classes implementing this interface can be converted from a JsonElement.
     */
    interface JsonElementSerializable {
        /**
         * Deserializes an object from a JsonElement.
         * @param {JsonElement} jsonElem - The JSON element to deserialize from
         */
        fromJson(jsonElem: JsonElement)
    }

    /**
     * Interface for types that can be serialized to JSON.
     * Classes implementing this interface can be converted to a JsonElement.
     */
    interface JsonElementDeserializable {
        /**
         * Serializes an object to a JsonElement.
         * @returns {JsonElement} The JSON representation of the object
         */
        toJson(): JsonElement
    }
}
