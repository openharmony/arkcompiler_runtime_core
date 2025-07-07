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

package std.interop;

// NOTE(from InteropTransferHelper.ets): #IC8LO3 remove local declaration after Any type is supported
type Any = Object | null | undefined

export type StaticOrESValue = ESValue | Object | null | undefined
export type ESValueCallbackData = object
export type ESValueCallback = (argThis: ESValue, argNewTgt: ESValue, args: FixedArray<ESValue>, data?: ESValueCallbackData) => ESValue
export declare class ESValuePropertyDescriptor {
    key: string | ESValue
    call?: ESValueCallback
    getter?: ESValueCallback
    setter?: ESValueCallback
    value?: ESValue
    data?: ESValueCallbackData
}

export final class ESValue {
    // #IC51G4: this.ev should be able to storage anything
    private ev: JSValue
    private isStatic: boolean

    private static OBJECT_CLASS: Class = Class.of(new Object());
    private static NUMBER_CLASS: Class = Class.of(new Number());
    private static BOOLEAN_CLASS: Class = Class.of(new Boolean());
    private static STRING_CLASS: Class = Class.of(new String());
    private static BIGINT_CLASS: Class = Class.of(new BigInt());

    constructor(ev: JSValue) {
        this.ev = ev
        this.isStatic = false;
    }

    constructor(ev: JSValue, isStatic: boolean) {
        this.ev = ev
        this.isStatic = isStatic;
    }

    public static get Undefined(): ESValue {
        return new ESValue(JSRuntime.getUndefined());
    }

    public static get Null(): ESValue {
        return new ESValue(JSRuntime.getNull());
    }

    public static wrapBoolean(b: boolean): ESValue {
        return new ESValue(JSRuntime.newJSValueBoolean(b));
    }

    public static wrapString(s: string): ESValue {
        return new ESValue(JSRuntime.newJSValueString(s));
    }

    public static wrapNumber(n: number): ESValue {
        return new ESValue(JSRuntime.newJSValueDouble(n));
    }

    public static wrapBigInt(bi: bigint): ESValue {
        return new ESValue(JSRuntime.newJSValueBigInt(bi));
    }

    public static wrapByte(b: byte): ESValue {
        return new ESValue(JSRuntime.newJSValueByte(b));
    }

    public static wrapShort(s: short): ESValue {
        return new ESValue(JSRuntime.newJSValueShort(s));
    }

    public static wrapInt(i: int): ESValue {
        return new ESValue(JSRuntime.newJSValueInt(i));
    }

    public static wrapLong(l: long): ESValue {
        return new ESValue(JSRuntime.newJSValueLong(l));
    }

    public static wrapLongLossy(l: long): ESValue {
        return new ESValue(JSRuntime.newJSValueLong(l));
    }

    public static wrapFloat(f: float): ESValue {
        return new ESValue(JSRuntime.newJSValueFloat(f));
    }

    public static wrapDouble(d: double): ESValue {
        return new ESValue(JSRuntime.newJSValueDouble(d));
    }

    public static wrap(o: Any): ESValue {
        if (o == null) {
            return new ESValue(JSRuntime.getNull());
        }
        if (o == undefined) {
            return new ESValue(JSRuntime.getUndefined());
        }
        o = o as object
        return new ESValue(JSRuntime.newJSValueObject(o), !JSRuntime.isJSValue(o));
    }

    public unwrap(): Any {
        if (this.isStaticObject()) {
            return this.toStaticObject();
        }
        return this.ev;
    }

    public isBoolean(): boolean {
        return this.typeOf() == "boolean";
    }

    public isString(): boolean {
        return this.typeOf() == "string";
    }

    public isNumber(): boolean {
        return this.typeOf() == "number";
    }

    public isBigInt(): boolean {
        return this.typeOf() == "bigint";
    }

    public isUndefined(): boolean {
        return this.typeOf() == "undefined";
    }

    public isNull(): boolean {
        return this.isStrictlyEqualTo(new ESValue(JSRuntime.getNull()));
    }

    public isStaticObject(): boolean {
        return this.isStatic;
    }

    public isECMAObject(): boolean {
        return !this.isStaticObject() &&
            !this.isBoolean() &&
            !this.isString() &&
            !this.isNumber() &&
            !this.isBigInt() &&
            !this.isUndefined() &&
            !this.isNull() &&
            !this.isFunction();
    }

    public isObject(): boolean {
        return this.isECMAObject() || this.isStaticObject();
    }

    public isFunction(): boolean {
        return this.typeOf() == "function";
    }

    public toBoolean(): boolean {
        if (!this.isBoolean()) {
            throw new TypeError("Boolean expected");
        }
        return JSRuntime.getValueObject(this.ev, ESValue.BOOLEAN_CLASS) as boolean;
    }

    public toString(): string {
        if(this.isObject()) {
            return JSRuntime.getValueObject(this.ev, ESValue.STRING_CLASS) as string;
        }
        if (!this.isString()) {
            throw new TypeError("String expected");
        }
        return JSRuntime.getValueString(this.ev);
    }

    public toNumber(): number {
        if (!this.isNumber()) {
            throw new TypeError("Number expected");
        }
        return JSRuntime.getValueObject(this.ev, ESValue.NUMBER_CLASS) as number;
    }

    public toBigInt(): bigint {
        if (!this.isBigInt()) {
            throw new TypeError("bigint expected");
        }
        return JSRuntime.getValueObject(this.ev, ESValue.BIGINT_CLASS) as bigint;
    }

    public toUndefined(): undefined {
        return this.ev as undefined;
    }

    public toNull(): null {
        return this.ev as null;
    }

    public toStaticObject(): object {
        if (this.isStatic) {
            return JSRuntime.getValueObject(this.ev, ESValue.OBJECT_CLASS);
        }
        throw Error("Can not cast to a static object")
    }

    public static areEqual(ev1: ESValue, ev2: ESValue): boolean {
        return JSRuntime.strictEqual(ev1.ev, ev2.ev);
    }

    public static areStrictlyEqual(ev1: ESValue, ev2: ESValue): boolean {
        return JSRuntime.strictEqual(ev1.ev, ev2.ev);
    }

    public static areEqualSafe(ev1: ESValue, ev2: ESValue): boolean {
        if (ev1.isUndefined() || ev1.isNull() &&
            (ev2.isUndefined() || ev2.isNull())) {
            return true;
        }
        return ESValue.areStrictlyEqual(ev1, ev2);
    }

    public isEqualTo(other: ESValue): boolean {
        return ESValue.areEqual(this, other);
    }

    public isStrictlyEqualTo(other: ESValue): boolean {
        return ESValue.areStrictlyEqual(this, other);
    }

    public isEqualToSafe(other: ESValue): boolean {
        return ESValue.areEqualSafe(this, other);
    }

    public instantiate(...args: FixedArray<StaticOrESValue>): ESValue {
        let argJSVal: FixedArray<JSValue> = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argJSVal[i] = arg.ev;
            } else {
                argJSVal[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.instantiate(this.ev, ...argJSVal))
    }

    public static instantiateEmptyObject(): ESValue {
        return new ESValue(JSRuntime.createObject());
    }

    public static instantiateEmptyArray(): ESValue {
        return new ESValue(JSRuntime.createArray());
    }

    public getProperty(name: string): ESValue {
        return new ESValue(JSRuntime.getPropertyJSValue(this.ev, name));
    }

    public getProperty(index: number): ESValue {
        if (Number.isInteger(index) && index >= 0) {
            return new ESValue(JSRuntime.getElementJSValue(this.ev, index as int));
        }
        let numObj = ESValue.wrapNumber(index);
        return this.getProperty(numObj);
    }

    public getProperty(property: ESValue): ESValue {
        return new ESValue(JSRuntime.getProperty(this.ev, property.ev));
    }

    public getPropertySafe(name: string): ESValue {
        if (this.hasProperty(name)) {
            return this.getProperty(name);
        }
        return ESValue.Undefined;
    }

    public getPropertySafe(index: number): ESValue {
        if (JSRuntime.hasElement(this.ev, index as int)) {
            return this.getProperty(index);
        }
        return ESValue.Undefined;
    }

    public getPropertySafe(property: ESValue): ESValue {
        if (this.hasProperty(property)) {
            return this.getProperty(property);
        }
        return ESValue.Undefined;
    }

    public setProperty(name: string, value: StaticOrESValue): void {
        if (value instanceof ESValue) {
            JSRuntime.setPropertyJSValue(this.ev, name, (value as ESValue).ev);
            return;
        }
        JSRuntime.setPropertyJSValue(this.ev, name, ESValue.wrap(value).ev);
    }

    public setProperty(index: number, value: StaticOrESValue): void {
        if (value instanceof ESValue) {
            JSRuntime.setElementJSValue(this.ev, index as int, (value as ESValue).ev);
            return;
        }
        JSRuntime.setElementJSValue(this.ev, index as int, ESValue.wrap(value).ev);
    }

    public setProperty(property: ESValue, value: StaticOrESValue): void {
        let type = property.typeOf();
        switch (type) {
            case "string":
                this.setProperty(property.ev as string, value);
                break;
            case "number":
                this.setProperty(property.ev as number, value);
                break;
            default:
                throw Error("type can't be set as key of property");
        }
    }

    public hasProperty(property: ESValue): boolean {
        return JSRuntime.hasPropertyJSValue(this.ev, property.ev);
    }

    public hasProperty(name: string): boolean {
        return JSRuntime.hasProperty(this.ev, name);
    }

    public hasOwnProperty(property: ESValue): boolean {
        return JSRuntime.hasOwnPropertyJSValue(this.ev, property.ev)
    }

    public hasOwnProperty(name: string): boolean {
        return JSRuntime.hasOwnProperty(this.ev, name);
    }

    public invoke(...args: FixedArray<StaticOrESValue>): ESValue {
        let argJSVal: FixedArray<JSValue> = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argJSVal[i] = arg.ev;
            } else {
                argJSVal[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(ESValue.Undefined.ev, this.ev, ...argJSVal));
    }

    internal invokeWithRecv(recv: ESValue, ...args: FixedArray<StaticOrESValue>): ESValue {
        let argJSVal: FixedArray<JSValue> = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argJSVal[i] = arg.ev;
            } else {
                argJSVal[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(recv.ev, this.ev, ...argJSVal));
    }

    public invokeMethod(method: string, ...args: FixedArray<StaticOrESValue>): ESValue {
        let callable = this.getPropertySafe(method);
        if (callable.isUndefined()) {
            throw Error("failed to get property");
        }
        let argJSVal: FixedArray<JSValue> = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argJSVal[i] = arg.ev;
            } else {
                argJSVal[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(this.ev, callable.ev, ...argJSVal));
    }

    public keys(): IterableIterator<ESValue> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const keysFunc = globalObject.getProperty("keys");

        let keysIter = keysFunc.invoke(this);
        return new ESValueIterator(keysIter);
    }

    public values(): IterableIterator<ESValue> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const valuesFunc = globalObject.getProperty("values");

        let valuesIter = valuesFunc.invoke(this);
        return new ESValueIterator(valuesIter);
    }

    public entries(): IterableIterator<[ESValue, ESValue]> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const entriesFunc = globalObject.getProperty("entries");

        let entries = entriesFunc.invoke(this);
        return new ESValueEntriesIterator(entries);
    }

    public instanceOf(type: ESValue | Type): boolean {
        if (type instanceof ESValue) {
            return JSRuntime.instanceOfDynamic(this.ev, (type as ESValue).ev);
        }
        if ((type) instanceof ByteType
            || (type) instanceof ShortType
            || (type) instanceof IntType
            || (type) instanceof LongType
            || (type) instanceof FloatType
            || (type) instanceof DoubleType
        ) {
            return this.typeOf() == "number";
        } else if (type.isPrimitive()) {
            return this.typeOf() == type.getLiteral().toLowerCase();
        } else if (type instanceof ClassType) {
            return JSRuntime.instanceOfStaticType(this.ev, type);
        }
        return false;
    }

    public typeOf(): String {
        if (this.ev == undefined) {
            return "undefined";
        }
        return JSRuntime.typeOf(this.ev);
    }

    public static load(module: string): ESValue {
        return new ESValue(JSRuntime.loadModule(module));
    }

    public static getGlobal(): ESValue {
        return new ESValue(JSRuntime.getGlobal());
    }

    public isPromise(): boolean {
        return JSRuntime.isPromise(this.ev);
    }

    public toPromise(): Promise<ESValue> {
        if (!this.isPromise()) {
            throw new TypeError("Promise expected");
        }
        return new Promise<ESValue>((resolve, reject) => {
            let jsPromise = this.ev as Promise<JSValue>;
            jsPromise.then((value: JSValue) => {
                resolve(ESValue.wrap(value));
            }).catch((error) => {
                reject(error);
            });
        });
    }

    static defineClass(
        name: String,
        ctor: ESValueCallback,
        data?: ESValueCallbackData,
        properties?: FixedArray<ESValuePropertyDescriptor>,
        superC?: ESValue
    ): ESValue {
        throw Error("not implemented yet");
    }

    public $_iterator(): IterableIterator<ESValue> {
        return new ESValueIterator(this);
    }

    public iterator(): IterableIterator<ESValue> {
        return this.$_iterator();
    }
}

final class ESValueIterator implements IterableIterator<ESValue> {
    private iterator: ESValue;

    constructor(iterableObj: ESValue) {
        const global = ESValue.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<ESValue> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<ESValue>();
        }

        return new IteratorResult<ESValue>(nextObj.getProperty("value"));
    }

    override $_iterator(): IterableIterator<ESValue> {
        return this;
    }
}

final class ESValueEntriesIterator implements IterableIterator<[ESValue, ESValue]> {
    private iterator: ESValue;

    constructor(iterableObj: ESValue) {
        const global = ESValue.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<[ESValue, ESValue]> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<[ESValue, ESValue]>();
        }
        let iterValue = nextObj.getProperty("value");
        let key = iterValue.getProperty(0);
        let value = iterValue.getProperty(1);

        return new IteratorResult<[ESValue, ESValue]>([key, value]);
    }

    override $_iterator(): IterableIterator<[ESValue, ESValue]> {
        return this;
    }
}
