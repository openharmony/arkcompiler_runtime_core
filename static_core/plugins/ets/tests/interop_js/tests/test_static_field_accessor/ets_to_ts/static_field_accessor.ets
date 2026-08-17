/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export class ClassWithStaticFields {
    public static counter: int = 42;
    public static label: string = 'hello';
    public instanceValue: int = 100;
}

export class ClassWithMixedFields {
    public static totalCount: int = 0;
    public static prefix: string = 'item';
    public name: string = 'default';
    public value: int = 10;
}

export class ClassWithOnlyStaticFields {
    public static x: int = 1;
    public static y: int = 2;
    public static z: int = 3;
}

export class ClassWithReservedStaticName {
    public static name: string = 'ets_name_value';
    public static normalField: string = 'normal';
    public instanceValue: int = 42;
}

export class ClassWithReservedStaticLength {
    public static length: int = 999;
    public static description: string = 'test';
}

export class ClassWithReservedStaticMethodName {
    public static name(): string {
        return 'ets_method_name_value';
    }
    public static normalMethod(): string {
        return 'normal_method';
    }
}

export class ClassWithStaticGetterSetterReservedName {
    public static cName: string = 'hidden_by_getter';
    public static get name(): string {
        return ClassWithStaticGetterSetterReservedName.cName;
    }
    public static set name(v: string) {
        ClassWithStaticGetterSetterReservedName.cName = v;
    }
}

export class ClassWithStaticGetterSetterReservedLength {
    public static cLength: int = 100;
    public static get length(): int {
        return ClassWithStaticGetterSetterReservedLength.cLength;
    }
    public static set length(v: int) {
        ClassWithStaticGetterSetterReservedLength.cLength = v;
    }
}

// 'prototype' is caught at compile-time by ETS compiler (error 2699),
// matching TypeScript behavior. No runtime test needed.

export class ClassWithInstanceGetterSetterReservedName {
    public cName: string = 'instance_name_value';
    public get name(): string {
        return this.cName;
    }
    public set name(v: string) {
        this.cName = v;
    }
}

// Instance getter/setter with reserved name 'length': should work
export class ClassWithInstanceGetterSetterReservedLength {
    public cLength: int = 50;
    public get length(): int {
        return this.cLength;
    }
    public set length(v: int) {
        this.cLength = v;
    }
}

// Static getter/setter with non-reserved name 'data': should work
export class ClassWithStaticGetterSetterNonReserved {
    public static cData: string = 'exported_data';
    public static get data(): string {
        return ClassWithStaticGetterSetterNonReserved.cData;
    }
    public static set data(v: string) {
        ClassWithStaticGetterSetterNonReserved.cData = v;
    }
}

// Static getter-only with reserved name 'name': also rejected
export class ClassWithStaticGetterReservedName {
    public static cName: string = 'getter_only';
    public static get name(): string {
        return ClassWithStaticGetterReservedName.cName;
    }
}

// Static setter-only with reserved name 'name': also rejected
export class ClassWithStaticSetterReservedName {
    public static cName: string = 'setter_only';
    public static set name(v: string) {
        ClassWithStaticSetterReservedName.cName = v;
    }
}

export function createClassWithStaticFields(): ClassWithStaticFields {
    return new ClassWithStaticFields();
}

export function createClassWithMixedFields(): ClassWithMixedFields {
    return new ClassWithMixedFields();
}

export function createClassWithOnlyStaticFields(): ClassWithOnlyStaticFields {
    return new ClassWithOnlyStaticFields();
}
