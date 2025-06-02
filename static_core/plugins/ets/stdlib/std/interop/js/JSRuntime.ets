/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

package std.interop.js;

// Subtype of Shareable enables reference tracking for ETS objects
// TODO(ivagin): This is a workaround and should be removed if the future
class Shareable { private internal_ : long; };

class MimicJSValueIteratableIterator implements IterableIterator<Object> {
    private iterator: JSValue;
    private nextMethod: JSValue;

    constructor(jsIterator: JSValue) {
        this.iterator = jsIterator;
        this.nextMethod = JSRuntime.getPropertyJSValue(jsIterator, 'next');
        if (this.nextMethod === undefined) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }
    }

    override next(): IteratorResult<Object> {
        const nextObj = JSRuntime.invoke(this.iterator, this.nextMethod);
        if (JSRuntime.getPropertyJSValue(nextObj, 'done') as boolean) {
            return new IteratorResult<Object>();
        }
        const iterValue = JSRuntime.getPropertyJSValue(nextObj, 'value');
        if (iterValue == undefined) {
            throw new Error('`value` field is expected in IteratorResult.');
        }
        const valueObject = JSRuntime.getValueObject(iterValue, Class.of(new Object()));
        // convert to Tuple2 for array&map&set
        // NOTE(#IC68BA) Need to create special iterable iterator object at function level 
        if (valueObject instanceof Array && valueObject.length == 2) {
            return new IteratorResult<Object>(new Tuple2<Object, Object>(valueObject[0] as Object,valueObject[1] as Object));
        } else {
            return new IteratorResult<Object>(valueObject);
        }
    }
}

export final class JSRuntime {
    // ==================
    // newJSValue<TYPE>()
    // ==================
    public static newJSValueBoolean(value: boolean): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static newJSValueByte(value: byte): JSValue {
        return JSRuntime.newJSValueInt(value);
    }

    public static newJSValueChar(value: char): JSValue {
        return JSRuntime.newJSValueInt(value);
    }

    public static newJSValueShort(value: short): JSValue {
        return JSRuntime.newJSValueInt(value);
    }

    public static newJSValueInt(value: int): JSValue {
        return JSRuntime.newJSValueDouble(value.toDouble());
    }

    public static newJSValueLong(value: long): JSValue {
        return JSRuntime.newJSValueDouble(value.toDouble());
    }

    public static newJSValueFloat(value: float): JSValue {
        return JSRuntime.newJSValueDouble(value);
    }

    public static newJSValueDouble(value: double): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static newJSValueString(value: String): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static newJSValueObject(value: Object): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static isJSValue(value: Object): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static newJSValueBigInt(bi: BigInt): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    // ================
    // getValue<TYPE>()
    // ================
    public static getValueBoolean(value: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getValueDouble(value: JSValue): double {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getValueString(value: JSValue): String {
        JSRuntime.checkIntrinsicMethod();
    }

    // 2nd argument is ClassClass
    public static getValueObject(value: JSValue, t: Class): Object {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    // getProperty<TYPE>()
    // ===================
    public static getPropertyBoolean(object: JSValue, name: String): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getPropertyByte(object: JSValue, name: String): byte {
        return (JSRuntime.getPropertyInt(object, name)).toByte();
    }

    public static getPropertyChar(object: JSValue, name: String): char {
        return (JSRuntime.getPropertyInt(object, name)).toChar();
    }

    public static getPropertyShort(object: JSValue, name: String): short {
        return (JSRuntime.getPropertyInt(object, name)).toShort();
    }

    public static getPropertyInt(object: JSValue, name: String): int {
        return (JSRuntime.getPropertyDouble(object, name)).toInt();
    }

    public static getPropertyLong(object: JSValue, name: String): long {
        return (JSRuntime.getPropertyDouble(object, name)).toLong();
    }

    public static getPropertyFloat(object: JSValue, name: String): float {
        return (JSRuntime.getPropertyDouble(object, name)).toFloat();
    }

    public static getPropertyDouble(object: JSValue, name: String): double {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getPropertyString(object: JSValue, name: String): String {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getPropertyJSValue(object: JSValue, name: String): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getPropertyJSValue(object: JSValue, key: JSValue): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    // setProperty<TYPE>()
    // ===================
    public static setPropertyBoolean(object: JSValue, name: String, value: boolean): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static setPropertyByte(object: JSValue, name: String, value: byte): void {
        JSRuntime.setPropertyInt(object, name, value);
    }

    public static setPropertyChar(object: JSValue, name: String, value: char): void {
        JSRuntime.setPropertyInt(object, name, value);
    }

    public static setPropertyShort(object: JSValue, name: String, value: short): void {
        JSRuntime.setPropertyInt(object, name, value);
    }

    public static setPropertyInt(object: JSValue, name: String, value: int): void {
        JSRuntime.setPropertyDouble(object, name, value.toDouble());
    }

    public static setPropertyLong(object: JSValue, name: String, value: long): void {
        JSRuntime.setPropertyDouble(object, name, value.toDouble());
    }

    public static setPropertyFloat(object: JSValue, name: String, value: float): void {
        JSRuntime.setPropertyDouble(object, name, value);
    }

    public static setPropertyDouble(object: JSValue, name: String, value: double): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static setPropertyString(object: JSValue, name: String, value: String): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static setPropertyJSValue(object: JSValue, name: String, value: JSValue): void {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    //
    // ===================
    public static hasProperty(object: JSValue, name: String): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static hasPropertyJSValue(object: JSValue, property: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static hasElement(object: JSValue, index: int): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static hasOwnProperty(object: JSValue, name: String): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static hasOwnPropertyJSValue(object: JSValue, property: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static getProperty(object: JSValue, property: JSValue): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }
    public static typeOf(object: JSValue): string {
        JSRuntime.checkIntrinsicMethod();
    }
    public static isPromise(object: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }
    public static invoke(recv: JSValue, func: JSValue, ...args: FixedArray<JSValue>): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }
    public static instanceOfStaticType(recv: JSValue, type: Type): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    //
    // ===================
    public static getUndefined(): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getNull(): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getGlobal(): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static createObject(): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static createArray(): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static instanceOfDynamic(object: JSValue, ctor: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static instanceOfStatic(object: JSValue, cls: Class): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static strictEqual(lhs: JSValue, rhs: JSValue): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    // getElement<TYPE>()
    // ===================
    public static getElementBoolean(object: JSValue, index: int): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getElementByte(object: JSValue, index: int): byte {
        return (JSRuntime.getElementInt(object, index)).toByte();
    }

    public static getElementChar(object: JSValue, index: int): char {
        return (JSRuntime.getElementInt(object, index)).toChar();
    }

    public static getElementShort(object: JSValue, index: int): short {
        return (JSRuntime.getElementInt(object, index)).toShort();
    }

    public static getElementInt(object: JSValue, index: int): int {
        return (JSRuntime.getElementDouble(object, index)).toInt();
    }

    public static getElementLong(object: JSValue, index: int): long {
        return (JSRuntime.getElementDouble(object, index)).toLong();
    }

    public static getElementFloat(object: JSValue, index: int): float {
        return (JSRuntime.getElementDouble(object, index)).toFloat();
    }

    public static getElementDouble(object: JSValue, index: int): double {
        JSRuntime.checkIntrinsicMethod();
    }

    public static getElementJSValue(object: JSValue, index: int): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    public static instantiate(callable: JSValue, ...args: FixedArray<JSValue>): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    // setElement<TYPE>()
    // ===================
    public static setElementBoolean(object: JSValue, index: int, value: boolean): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static setElementByte(object: JSValue, index: int, value: byte): void {
        JSRuntime.setElementInt(object, index, value);
    }

    public static setElementChar(object: JSValue, index: int, value: char): void {
        JSRuntime.setElementInt(object, index, value);
    }

    public static setElementShort(object: JSValue, index: int, value: short): void {
        JSRuntime.setElementInt(object, index, value);
    }

    public static setElementInt(object: JSValue, index: int, value: int): void {
        JSRuntime.setElementDouble(object, index, value.toDouble());
    }

    public static setElementLong(object: JSValue, index: int, value: long): void {
        JSRuntime.setElementDouble(object, index, value.toDouble());
    }

    public static setElementFloat(object: JSValue, index: int, value: float): void {
        JSRuntime.setElementDouble(object, index, value);
    }

    public static setElementDouble(object: JSValue, index: int, value: double): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static setElementJSValue(object: JSValue, index: int, value: JSValue): void {
        JSRuntime.checkIntrinsicMethod();
    }

    public static loadModule(m: String): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    // ===================
    // Public API
    // ===================

    public static iterate(object: JSValue, callback: (arrElem: JSValue) => void): void
    {
        const global = JSRuntime.getGlobal();
        const symbol = JSRuntime.getPropertyJSValue(global, "Symbol");
        const symbolIterator = JSRuntime.getPropertyJSValue(symbol, "iterator");

        const symbolIteratorMethod = JSRuntime.getPropertyJSValue(object, symbolIterator);
        if (symbolIteratorMethod === undefined) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        const iterator = symbolIteratorMethod.call(object);

        let next: JSValue;
        let done: JSValue;
        let value: JSValue;

        while (true) {
            next = iterator.next();
            if (next === undefined) {
                throw new Error('`next() return undefined, but IterationResult was expected.`')
            }

            done = next.done;
            if (done === undefined) {
                throw new Error('`done` field is expected in IteratorResult.');
            }

            if ((done as boolean) === true) {
                return;
            }

            value = next.value;
            if (value === undefined) {
                throw new Error('`value` field is expected in IteratorResult when `done` = false.');
            }

            callback(value);
        }
    }

    // ===================
    // Internal api
    // ===================
    public static getNumSharedJSValues(): int {
        return JSRuntime.finQueue!.getSize();
    }

    private static finalizationRegistryCallback(obj: Object): void {
        JSRuntime.checkIntrinsicMethod();
    }

    private static createFinalizationRegistry(): FinalizationRegistry<Object> {
        let fq = new FinalizationRegistry<Object>((obj: Object): void => { JSRuntime.finalizationRegistryCallback(obj); });
        if (JSRuntime.finQueue == null)
        {
            JSRuntime.finQueue = fq;
        }
        return fq;
    }

    public static __initJSCallClass(): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static __initJSNewClass(): boolean {
        JSRuntime.checkIntrinsicMethod();
    }

    public static __createLambdaProxy(lambda: Object): JSValue {
        JSRuntime.checkIntrinsicMethod();
    }

    /**
     * The function triggers cross-reference GC
     * @returns gc task id
     * @see GC.waitForFinishGC
     */
    public static native xgc(): long;

    private static checkIntrinsicMethod(): never {
        let msg = "This method must not be called";
        // TODO(dmitrys99): Call to console gives error here:
        // SyntaxError: Variable 'console' is accessed before it's initialization. [JSRuntime.ets:229:9]
        //console.print("ERROR: " + msg + "\n");
        throw new Error(msg);
    }

    // NOTE(konstanting): currently we keep the reference to the finQueue for the first interop context only.
    // As this field is used only in the tests, it is okay for now, because the Interop Context instances obtain
    // the correct finreg values during their initialization. But of course we should keep all the finreg references
    // here and use the tests to verify that they function correctly.
    private static finQueue: FinalizationRegistry<Object> | null = null;

    public static CreateIterator(val: JSValue): MimicJSValueIteratableIterator {
        return new MimicJSValueIteratableIterator(val);
    }
}
