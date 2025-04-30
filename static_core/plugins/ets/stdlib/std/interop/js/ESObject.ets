/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

export type StaticOrESObject = ESObject | Object | null | undefined
export type ESObjectCallbackData = object
export type ESObjectCallback = (argThis: ESObject, argNewTgt: ESObject, args: ESObject[], data?: ESObjectCallbackData) => ESObject
export declare class ESObjectPropertyDescriptor {
    key: string | ESObject
    call?: ESObjectCallback
    getter?: ESObjectCallback
    setter?: ESObjectCallback
    value?: ESObject
    data?: ESObjectCallbackData
}

export final class ESObject {
    // #IC51G4: this.eo should be able to storage anything
    private eo: JSValue
    private isStatic: boolean

    constructor(eo: JSValue) {
        this.eo = eo
        this.isStatic = false;
    }

    constructor(eo: JSValue, isStatic: boolean) {
        this.eo = eo
        this.isStatic = isStatic;
    }

    public static get Undefined(): ESObject {
        return new ESObject(JSRuntime.getUndefined());
    }

    public static get Null(): ESObject {
        return new ESObject(JSRuntime.getNull());
    }

    public static wrapBoolean(b: boolean): ESObject {
        return new ESObject(JSRuntime.newJSValueBoolean(b));
    }

    public static wrapString(s: string): ESObject {
        return new ESObject(JSRuntime.newJSValueString(s));
    }

    public static wrapNumber(n: number): ESObject {
        return new ESObject(JSRuntime.newJSValueDouble(n));
    }

    public static wrapBigInt(bi: bigint): ESObject {
        return new ESObject(JSRuntime.newJSValueBigInt(bi));
    }

    public static wrapByte(b: byte): ESObject {
        return new ESObject(JSRuntime.newJSValueByte(b));
    }

    public static wrapShort(s: short): ESObject {
        return new ESObject(JSRuntime.newJSValueShort(s));
    }

    public static wrapInt(i: int): ESObject {
        return new ESObject(JSRuntime.newJSValueInt(i));
    }

    public static wrapLong(l: long): ESObject {
        return new ESObject(JSRuntime.newJSValueLong(l));
    }

    public static wrapLongLossy(l: long): ESObject {
        return new ESObject(JSRuntime.newJSValueLong(l));
    }

    public static wrapFloat(f: float): ESObject {
        return new ESObject(JSRuntime.newJSValueFloat(f));
    }

    public static wrapDouble(d: double): ESObject {
        return new ESObject(JSRuntime.newJSValueDouble(d));
    }

    public static wrap(o: object | null | undefined ): ESObject {
        if (o == null) {
            return new ESObject(JSRuntime.getNull());
        }
        if (o == undefined) {
            return new ESObject(JSRuntime.getUndefined());
        }
        return new ESObject(JSRuntime.newJSValueObject(o), true);
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
        return this.isStrictlyEqualTo(new ESObject(JSRuntime.getNull()));
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
        return this.eo as boolean;
    }

    public toString(): string {
        return this.eo as string;
    }

    public toNumber(): number {
        return this.eo as number;
    }

    public toBigInt(): bigint {
        return this.eo as bigint;
    }

    public toUndefined(): undefined {
        return this.eo as undefined;
    }

    public toNull(): null {
        return this.eo as null;
    }

    public toStaticObject(): object {
        if (this.isStatic) {
            return this.eo as Object;
        }
        throw Error("Can not cast to a static object")
    }

    public static areEqual(eo1: ESObject, eo2: ESObject): boolean {
        return JSRuntime.strictEqual(eo1.eo, eo2.eo);
    }

    public static areStrictlyEqual(eo1: ESObject, eo2: ESObject): boolean {
        return JSRuntime.strictEqual(eo1.eo, eo2.eo);
    }

    public static areEqualSafe(eo1: ESObject, eo2: ESObject): boolean {
        if (eo1.isUndefined() || eo1.isNull() &&
            (eo2.isUndefined() || eo2.isNull())) {
                return true;
        }
        return ESObject.areStrictlyEqual(eo1, eo2);
    }

    public isEqualTo(other: ESObject): boolean {
        return ESObject.areEqual(this, other);
    }

    public isStrictlyEqualTo(other: ESObject): boolean {
        return ESObject.areStrictlyEqual(this, other);
    }

    public isEqualToSafe(other: ESObject): boolean {
        return ESObject.areEqualSafe(this, other);
    }

    public instantiate(...args: StaticOrESObject[]): ESObject {
        let argJSVal: JSValue[] = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESObject) {
                let arg = args[i] as ESObject;
                argJSVal[i] = arg.eo;
            } else {
                argJSVal[i] = ESObject.wrap(args[i]).eo;
            }
        }
        return new ESObject(JSRuntime.instantiate(this.eo, ...argJSVal))
    }

    public static instantiateEmptyObject(): ESObject {
        return new ESObject(JSRuntime.createObject());
    }

    public static instantiateEmptyArray(): ESObject {
        return new ESObject(JSRuntime.createArray());
    }

    public getProperty(name: string): ESObject {
        return new ESObject(JSRuntime.getPropertyJSValue(this.eo, name));
    }

    public getProperty(index: number): ESObject {
        if (Number.isInteger(index) && index >= 0) {
            return new ESObject(JSRuntime.getElementJSValue(this.eo, index as int));
        }
        let numObj = ESObject.wrapNumber(index);
        return this.getProperty(numObj);
    }

    public getProperty(property: ESObject): ESObject {
        return new ESObject(JSRuntime.getProperty(this.eo, property.eo));
    }

    public getPropertySafe(name: string): ESObject {
        if (this.hasProperty(name)) {
            return this.getProperty(name);
        }
        return ESObject.Undefined;
    }

    public getPropertySafe(index: number): ESObject {
        if (JSRuntime.hasElement(this.eo, index as int)) {
            return this.getProperty(index);
        }
        return ESObject.Undefined;
    }

    public getPropertySafe(property: ESObject): ESObject {
        if (this.hasProperty(property)) {
            return this.getProperty(property);
        }
        return ESObject.Undefined;
    }

    public setProperty(name: string, value: StaticOrESObject): void {
        if (value instanceof ESObject) {
            JSRuntime.setPropertyJSValue(this.eo, name, (value as ESObject).eo);
            return;
        }
        JSRuntime.setPropertyJSValue(this.eo, name, ESObject.wrap(value).eo);
    }

    public setProperty(index: number, value: StaticOrESObject): void {
        if (value instanceof ESObject) {
            JSRuntime.setElementJSValue(this.eo, index as int, (value as ESObject).eo);
            return;
        }
        JSRuntime.setElementJSValue(this.eo, index as int, ESObject.wrap(value).eo);
    }

    public setProperty(property: ESObject, value: StaticOrESObject): void {
        let type = property.typeOf();
        switch (type) {
            case "string":
                this.setProperty(property.eo as string, value);
                break;
            case "number":
                this.setProperty(property.eo as number, value);
                break;
            default:
                throw Error("type can't be set as key of property");
        }
    }

    public hasProperty(property: ESObject): boolean {
        return JSRuntime.hasPropertyJSValue(this.eo, property.eo);
    }

    public hasProperty(name: string): boolean {
        return JSRuntime.hasProperty(this.eo, name);
    }

    public hasOwnProperty(property: ESObject): boolean {
        return JSRuntime.hasOwnPropertyJSValue(this.eo, property.eo)
    }

    public hasOwnProperty(name: string): boolean {
        return JSRuntime.hasOwnProperty(this.eo, name);
    }

    public invoke(...args: StaticOrESObject[]): ESObject {
        let argJSVal: JSValue[] = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESObject) {
                let arg = args[i] as ESObject;
                argJSVal[i] = arg.eo;
            } else {
                argJSVal[i] = ESObject.wrap(args[i]).eo;
            }
        }
        return new ESObject(JSRuntime.invoke(ESObject.Undefined.eo, this.eo, ...argJSVal));
    }

    internal invokeWithRecv(recv: ESObject, ...args: StaticOrESObject[]): ESObject {
        let argJSVal: JSValue[] = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESObject) {
                let arg = args[i] as ESObject;
                argJSVal[i] = arg.eo;
            } else {
                argJSVal[i] = ESObject.wrap(args[i]).eo;
            }
        }
        return new ESObject(JSRuntime.invoke(recv.eo, this.eo, ...argJSVal));
    }

    public invokeMethod(method: string, ...args: StaticOrESObject[]): ESObject {
        let callable = this.getPropertySafe(method);
        if (callable.isUndefined()) {
            throw Error("failed to get property");
        }
        let argJSVal: JSValue[] = new JSValue[args.length];
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESObject) {
                let arg = args[i] as ESObject;
                argJSVal[i] = arg.eo;
            } else {
                argJSVal[i] = ESObject.wrap(args[i]).eo;
            }
        }
        return new ESObject(JSRuntime.invoke(this.eo, callable.eo, ...argJSVal));
    }

    public keys(): IterableIterator<ESObject> {
        const global = ESObject.getGlobal();
        const globalObject = global.getProperty("Object");
        const keysFunc = globalObject.getProperty("keys");

        let keysIter = keysFunc.invoke(this);
        return new ESObjectIterator(keysIter);
    }

    public values(): IterableIterator<ESObject> {
        const global = ESObject.getGlobal();
        const globalObject = global.getProperty("Object");
        const valuesFunc = globalObject.getProperty("values");

        let valuesIter = valuesFunc.invoke(this);
        return new ESObjectIterator(valuesIter);
    }

    public entries(): IterableIterator<[ESObject, ESObject]> {
        const global = ESObject.getGlobal();
        const globalObject = global.getProperty("Object");
        const entriesFunc = globalObject.getProperty("entries");

        let entries = entriesFunc.invoke(this);
        return new ESObjectEntriesIterator(entries);
    }

    public instanceOf(type: ESObject | Type): boolean {
        if (type instanceof ESObject) {
            return JSRuntime.instanceOfDynamic(this.eo, (type as ESObject).eo);
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
            return JSRuntime.instanceOfStaticType(this.eo, type);
        }
        return false;
    }

    public typeOf(): String {
        if (this.eo == undefined) {
            return "undefined";
        }
        return JSRuntime.typeOf(this.eo);
    }

    public static load(module: string): ESObject {
        return new ESObject(JSRuntime.loadModule(module));
    }

    public static getGlobal(): ESObject {
        return new ESObject(JSRuntime.getGlobal());
    }

    public isPromise(): boolean {
        return JSRuntime.isPromise(this.eo);
    }

    public toPromise(): Promise<ESObject> {
        throw Error("not implemented yet");
    }

    static defineClass(
        name: String,
        ctor: ESObjectCallback,
        data?: ESObjectCallbackData,
        properties?: ESObjectPropertyDescriptor[],
        superC?: ESObject
    ): ESObject {
        throw Error("not implemented yet");
    }

    public $_iterator(): IterableIterator<ESObject> {
        return new ESObjectIterator(this);
    }

    public iterator(): IterableIterator<ESObject> {
        return this.$_iterator();
    }
}

final class ESObjectIterator implements IterableIterator<ESObject> {
    private iterator: ESObject;

    constructor(iterableObj: ESObject) {
        const global = ESObject.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<ESObject> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<ESObject>();
        }

        return new IteratorResult<ESObject>(nextObj.getProperty("value"));
    }

    override $_iterator(): IterableIterator<ESObject> {
        return this;
    }
}

final class ESObjectEntriesIterator implements IterableIterator<[ESObject, ESObject]> {
    private iterator: ESObject;

    constructor(iterableObj: ESObject) {
        const global = ESObject.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<[ESObject, ESObject]> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<[ESObject, ESObject]>();
        }
        let iterValue = nextObj.getProperty("value");
        let key = iterValue.getProperty(0);
        let value = iterValue.getProperty(1);

        return new IteratorResult<[ESObject, ESObject]>([key, value]);
    }

    override $_iterator(): IterableIterator<[ESObject, ESObject]> {
        return this;
    }
}
