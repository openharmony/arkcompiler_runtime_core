/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

package class_proxy.test;

class jsvars {
    static global = JSRuntime.getGlobal();
};

class ListNode {
    constructor(tag: double, next: ListNode) {
        this.next = next;
        this.tag = tag;
    }
    public get_next(): ListNode {
        return this.next;
    }
    public set_next(v: ListNode): void {
        this.next = v;
    }

    public next: ListNode;
    public tag: double;
};

class ProxyEscapeCtor {
    constructor(cb: JSValue) {
        cb(this);
    }
};

class ProxyClassCalls {
    constructor(cb: JSValue) {
        cb();
    }
    static call(cb: JSValue) {
        cb();
    }
};

class PrimitivesAccess {
    public getf_byte(): byte { return this.f_byte; }
    public getf_short(): short { return this.f_short; }
    public getf_int(): int { return this.f_int; }
    public getf_long(): long { return this.f_long; }
    public getf_float(): float { return this.f_float; }
    public getf_double(): double { return this.f_double; }
    public getf_char(): char { return this.f_char; }
    public getf_boolean(): boolean { return this.f_boolean; }

    public setf_byte(v: byte): void { this.f_byte = v; }
    public setf_short(v: short): void { this.f_short = v; }
    public setf_int(v: int): void { this.f_int = v; }
    public setf_long(v: long): void { this.f_long = v; }
    public setf_float(v: float): void { this.f_float = v; }
    public setf_double(v: double): void { this.f_double = v; }
    public setf_char(v: char): void { this.f_char = v; }
    public setf_boolean(v: boolean): void { this.f_boolean = v; }

    public f_byte: byte;
    public f_short: short;
    public f_int: int;
    public f_long: long;
    public f_float: float;
    public f_double: double;
    public f_char: char;
    public f_boolean: boolean;
};

class PrimitivesAccessStatic {
    public static getf_byte(): byte { return PrimitivesAccessStatic.f_byte; }
    public static getf_short(): short { return PrimitivesAccessStatic.f_short; }
    public static getf_int(): int { return PrimitivesAccessStatic.f_int; }
    public static getf_long(): long { return PrimitivesAccessStatic.f_long; }
    public static getf_float(): float { return PrimitivesAccessStatic.f_float; }
    public static getf_double(): double { return PrimitivesAccessStatic.f_double; }
    public static getf_char(): char { return PrimitivesAccessStatic.f_char; }
    public static getf_boolean(): boolean { return PrimitivesAccessStatic.f_boolean; }

    public static setf_byte(v: byte): void { PrimitivesAccessStatic.f_byte = v; }
    public static setf_short(v: short): void { PrimitivesAccessStatic.f_short = v; }
    public static setf_int(v: int): void { PrimitivesAccessStatic.f_int = v; }
    public static setf_long(v: long): void { PrimitivesAccessStatic.f_long = v; }
    public static setf_float(v: float): void { PrimitivesAccessStatic.f_float = v; }
    public static setf_double(v: double): void { PrimitivesAccessStatic.f_double = v; }
    public static setf_char(v: char): void { PrimitivesAccessStatic.f_char = v; }
    public static setf_boolean(v: boolean): void { PrimitivesAccessStatic.f_boolean = v; }

    public static f_byte: byte;
    public static f_short: short;
    public static f_int: int;
    public static f_long: long;
    public static f_float: float;
    public static f_double: double;
    public static f_char: char;
    public static f_boolean: boolean;
}

class UClass1 { };
class UClass2 { };

class ReferencesAccess {
    public getf_UClass1(): UClass1 { return this.f_UClass1; }
    public getf_UClass2(): UClass2 { return this.f_UClass2; }
    public getf_String(): String { return this.f_String; }
    public getf_JSValue(): JSValue { return this.f_JSValue; }
    public getf_Promise(): Promise<Object> { return this.f_Promise; }
    public getf_A_String(): String[] { return this.f_A_String; }
    public getf_A_UClass1(): UClass1[] { return this.f_A_UClass1; }

    public setf_UClass1(v: UClass1): void { this.f_UClass1 = v; }
    public setf_UClass2(v: UClass2): void { this.f_UClass2 = v; }
    public setf_String(v: String): void { this.f_String = v; }
    public setf_JSValue(v: JSValue): void { this.f_JSValue = v; }
    public setf_Promise(v: Promise<Object>): void { this.f_Promise = v; }
    public setf_A_String(v: String[]): void { this.f_A_String = v; }
    public setf_A_UClass1(v: UClass1[]): void { this.f_A_UClass1 = v; }

    public f_UClass1: UClass1;
    public f_UClass2: UClass2;
    public f_String: String;
    public f_JSValue: JSValue;
    public f_Promise: Promise<Object>;
    public f_A_String: String[];
    public f_A_UClass1: UClass1[];
};

class ReferencesAccessStatic {
    public static getf_UClass1(): UClass1 { return ReferencesAccessStatic.f_UClass1; }
    public static getf_UClass2(): UClass2 { return ReferencesAccessStatic.f_UClass2; }
    public static getf_String(): String { return ReferencesAccessStatic.f_String; }
    public static getf_JSValue(): JSValue { return ReferencesAccessStatic.f_JSValue; }
    public static getf_Promise(): Promise<Object> { return ReferencesAccessStatic.f_Promise; }

    public static setf_UClass1(v: UClass1): void { ReferencesAccessStatic.f_UClass1 = v; }
    public static setf_UClass2(v: UClass2): void { ReferencesAccessStatic.f_UClass2 = v; }
    public static setf_String(v: String): void { ReferencesAccessStatic.f_String = v; }
    public static setf_JSValue(v: JSValue): void { ReferencesAccessStatic.f_JSValue = v; }
    public static setf_Promise(v: Promise<Object>): void { ReferencesAccessStatic.f_Promise = v; }

    public static f_UClass1: UClass1;
    public static f_UClass2: UClass2;
    public static f_String: String;
    public static f_JSValue: JSValue;
    public static f_Promise: Promise<Object>;
};

class FooBase {
    public f_base: String = "f_base";
    public static sf_base: String = "sf_base";

    public getAsObject(): Object { return this; }
    public static createAsObject(): Object { return new FooDerived(); }

    public override toString(): String {
        return "FooBase toString";
    }
    public fn_base(): String { return "fn_base"; }
};

class FooDerived extends FooBase {
    public f_derived: String = "f_derived";
    public static sf_derived: String = "sf_derived";

    public getAsFooBase(): FooBase { return this; }
    public static createAsFooBase(): FooBase { return new FooDerived(); }

    public override toString(): String {
        return "FooDerived toString";
    }
};

class FooError1 extends Error {
    public f_error1: String = "f_error1";
};

class FooError2 extends FooError1 {
    public f_error2: String = "f_error2";

    public override toString(): String {
        return "FooError2 toString";
    }
}

function IdentityObject(a: Object): Object {
    return a;
}
function IdentityError(a: Error): Error {
    return a;
}
