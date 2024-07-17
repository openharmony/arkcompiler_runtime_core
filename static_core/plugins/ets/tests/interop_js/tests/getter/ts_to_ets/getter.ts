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

export const ts_string = 'string';
export const ts_number = 1;

export class PublicGetterClass {
    _value = ts_string;

    get value(): string {
        return this._value;
    }
}

export function create_public_getter_class_from_ts(): PublicGetterClass {
    return new PublicGetterClass();
}

export const publicGetterInstanceClass = new PublicGetterClass();

export class ProtectedGetterOrigenClass {
    protected _value = ts_string;

    protected get value(): string {
        return this._value;
    }
}

export function create_protected_getter_origen_class_from_ts(): ProtectedGetterOrigenClass {
    return new ProtectedGetterOrigenClass();
}

export const protectedGetterOrigenInstanceClass = new ProtectedGetterOrigenClass();

export class ProtectedGetterInheritanceClass extends ProtectedGetterOrigenClass { }

export function create_protected_getter_inheritance_class_from_ts(): ProtectedGetterInheritanceClass {
    return new ProtectedGetterInheritanceClass();
}

export const protectedGetterInstanceInheritanceClass = new ProtectedGetterInheritanceClass();

export class PrivateGetterClass {
    private _value = ts_string;

    private get value(): string {
        return this._value;
    }
}

export function create_private_getter_class_from_ts(): PrivateGetterClass {
    return new PrivateGetterClass();
}

export const privateGetterInstanceClass = new PrivateGetterClass();

export type UnionType = number | string;

export class UnionTypeClass {
    private _value: UnionType;

    constructor(value: UnionType) {
        this._value = value;
    }

    public get value(): UnionType {
        return this._value;
    }
}

export function create_union_type_getter_class_from_ts(arg: UnionType): UnionTypeClass {
    return new UnionTypeClass(arg);
}

export const unionTypeGetterInstanceClassInt = new UnionTypeClass(ts_number);
export const unionTypeGetterInstanceClassString = new UnionTypeClass(ts_string);

export type LiteralValue = 1 | 'string';

export class LiteralClass {
    private _value: LiteralValue;

    constructor(value: LiteralValue) {
        this._value = value;
    }

    public get value(): LiteralValue {
        return this._value;
    }
}

export function create_literal_type_getter_class_from_ts(arg: LiteralValue): LiteralClass {
    return new LiteralClass(arg);
}

export const literalTypeGetterInstanceClassInt = new LiteralClass(ts_number);
export const literalTypeGetterInstanceClassString = new LiteralClass(ts_string);

export type TupleType = [number, string];

export class TupleTypeClass {
    private _value: TupleType;

    constructor(value: TupleType) {
        this._value = value;
    }

    public get value(): TupleType {
        return this._value;
    }
}

export function create_tuple_type_getter_class_from_ts(arg: TupleType): TupleTypeClass {
    return new TupleTypeClass(arg);
}

export const tupleTypeGetterInstanceClass = new TupleTypeClass([ts_number, ts_string]);

export class AnyTypeClass<T> {
    public _value: T;

    public get value(): T {
        return this._value;
    }
}

export function create_any_type_getter_class_from_ts(): AnyTypeClass<any> {
    return new AnyTypeClass();
}

export const anyTypeGetterInstanceClass = new AnyTypeClass();
export const anyTypeExplicitGetterInstanceClass = new AnyTypeClass<string>();

export class SubsetByRef {
    private _refClass: PublicGetterClass;

    constructor() {
        this._refClass = new PublicGetterClass();
    }

    public get value(): string {
        return this._refClass.value;
    }
}

export function create_subset_by_ref_getter_class_from_ts(): SubsetByRef {
    return new SubsetByRef();
}

export const subsetByRefInstanceClass = new SubsetByRef();

export class SubsetByValueClass {
    public _value: string;

    constructor(value: string) {
        this._value = value;
    }

    get value(): string {
        return this._value;
    }
}

const GClass = new PublicGetterClass();

export function create_subset_by_value_getter_class_from_ts(): SubsetByValueClass {
    return new SubsetByValueClass(GClass.value);
}

export const subsetByValueInstanceClass = new SubsetByValueClass(GClass.value);
