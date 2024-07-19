/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

export const ts_int = 1;

export class MainClass {
    _value: number;

    constructor(value: number) {
        this._value = value;
    }
}

export const IIFEClass = (
    function () {
        return class {
            _value: number;

            constructor(value: number) {
                this._value = value;
            }
        }
    }
)();

export const AnonymousClass = class {
    _value: number;

    constructor(value: number) {
        this._value = value;
    }
}

interface ClassConstructor {
    new(value: number): any;
}

export class ParentClass {
    _other_class: any;

    constructor(other_class: ClassConstructor) {
        this._other_class = new other_class(ts_int);
    }
}

export function create_class_with_arg_from_ts(arg: ClassConstructor): ParentClass {
    return new ParentClass(arg);
}

export function create_main_class_from_ts(): ParentClass {
    return new ParentClass(MainClass);
}

export const mainClassInstance = new ParentClass(MainClass);

export function create_anonymous_class_from_ts(): ParentClass {
    return new ParentClass(AnonymousClass);
}

export const anonymousClassInstance = new ParentClass(AnonymousClass);

export function create_IIFE_class_from_ts(): ParentClass {
    return new ParentClass(IIFEClass);
}

export const iifeClassInstance = new ParentClass(IIFEClass);

export class ChildClass extends ParentClass {
    constructor(other_class: ClassConstructor) {
        super(other_class);
    }
}

export function create_child_class_with_arg_from_ts(arg: ClassConstructor): ChildClass {
    return new ChildClass(arg);
}

export function create_child_class_with_main_from_ts(): ChildClass {
    return new ChildClass(MainClass);
}

export const childClassWithMainInstance = new ChildClass(MainClass);

export function create_child_class_with_anonymous_from_ts(): ChildClass {
    return new ChildClass(MainClass);
}

export const childClassWithAnonymousInstance = new ChildClass(MainClass);

export function create_child_class_with_IIFE_from_ts(): ChildClass {
    return new ChildClass(IIFEClass);
}

export const childClassWithIIFEInstance = new ChildClass(IIFEClass);

export const AnonymousClassCreateClass = class {
    _other_class: MainClass;

    constructor(other_class: ClassConstructor) {
        this._other_class = new other_class(ts_int);
    }
}

export function create_anonymous_class_create_class_with_arg_from_ts(arg: ClassConstructor) {
    return new AnonymousClassCreateClass(arg);
}

export function create_anonymous_class_create_class_from_ts() {
    return new AnonymousClassCreateClass(MainClass);
}

export function create_anonymous_class_create_IIFE_class_from_ts() {
    return new AnonymousClassCreateClass(IIFEClass);
}

export const anonymousClassCreateMainInstance = new AnonymousClassCreateClass(MainClass);

export const anonymousClassCreateIIFEInstance = new AnonymousClassCreateClass(IIFEClass);

export const IIFECreateClassMain = (
    function (ctor: new (_value: number) => MainClass, value: number) {
        return new ctor(value)
    }
)(MainClass, ts_int);


export const IIFECreateClassAnonymous = (
    function (ctor: new (_value: number) => InstanceType<typeof AnonymousClass>, value: number) {
        return new ctor(value)
    }
)(AnonymousClass, ts_int);

export const IIFECreateClass = (
    function (ctor: new (_value: number) => InstanceType<typeof IIFEClass>, value: number) {
        return new ctor(value)
    }
)(IIFEClass, ts_int);

export class MethodClass {
    init(anyClass: ClassConstructor, value: any) {
        return new anyClass(value)
    }
}

export function create_class_function(arg: ClassConstructor, val: any) {
    return new arg(val);
}

export function create_class_arrow_function(arg: ClassConstructor, val: any) {
    return new arg(val);
}

export function check_instance<T, U>(mainClass: new (...args: any[]) => T, instance: U): boolean {
    if (typeof mainClass !== 'function' || typeof instance !== 'object' || instance === null) {
        throw new TypeError('must be a class');
    }

    return instance instanceof mainClass;
}