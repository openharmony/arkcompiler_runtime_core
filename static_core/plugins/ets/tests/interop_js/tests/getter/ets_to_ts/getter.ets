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

package getter.test;

const ts_string = 'string';
const ts_number = 1;

class PublicGetterClass {
    _value = ts_string;

    get value(): string {
        return this._value;
    }
}

function create_public_getter_class_from_ets(): PublicGetterClass {
    return new PublicGetterClass();
}

class ProtectedGetterOrigenClass {
    protected _value = ts_string;

    public get value(): string {
        return this._value;
    }
}

function create_protected_getter_origen_class_from_ets(): ProtectedGetterOrigenClass {
    return new ProtectedGetterOrigenClass();
}

class ProtectedGetterInheritanceClass extends ProtectedGetterOrigenClass { }

function create_protected_getter_inheritance_class_from_ets(): ProtectedGetterInheritanceClass {
    return new ProtectedGetterInheritanceClass();
}

class PrivateGetterClass {
    private _value = ts_string;

    public get value(): string {
        return this._value;
    }
}

function create_private_getter_class_from_ets(): PrivateGetterClass {
    return new PrivateGetterClass();
}

class PrivateGetterInheritanceClass extends PrivateGetterClass { }

function create_private_getter_inheritance_class_from_ets(): PrivateGetterInheritanceClass {
    return new PrivateGetterInheritanceClass();
}

type Union = int | string;

class UnionTypeClass {
    public _value: Union;

    constructor(value: Union) {
        this._value = value;
    }

    public get value(): Union {
        return this._value;
    }
}

function create_union_type_getter_class_from_ets(arg: Union): UnionTypeClass {
    return new UnionTypeClass(arg);
}

// type LiteralValue = 1 | 'string';

// class LiteralClass {
//     private _value: LiteralValue;

//     constructor(value: LiteralValue) {
//         this._value = value;
//     }

//     public get value(): LiteralValue {
//         return this._value;
//     }
// }

// function create_literal_type_getter_class_from_ets(arg: LiteralValue): LiteralClass {
//     return new LiteralClass(arg);
// }

type Tuple = [number, string];

class TupleTypeClass {
    private _value: Tuple;

    constructor(value: Tuple) {
        this._value = value;
    }

    public get value(): Tuple {
        return this._value;
    }
}

function create_tuple_type_getter_class_from_ets(arg: Tuple): TupleTypeClass {
    return new TupleTypeClass(arg);
}

class AnyTypeClass<T> {
    public _value: T;

    constructor(value: T) {
        this._value = value;
    }

    public get value(): T {
        return this._value;
    }
}

function create_any_type_getter_class_from_ets_string(arg: string): AnyTypeClass<string> {
    return new AnyTypeClass<string>(arg);
}

function create_any_type_getter_class_from_ets_int(arg: int): AnyTypeClass<int> {
    return new AnyTypeClass<int>(arg);
}

function create_any_type_getter_class_from_ets_bool(arg: boolean): AnyTypeClass<boolean> {
    return new AnyTypeClass<boolean>(arg);
}

function create_any_type_getter_class_from_ets_arr(arg: int[]): AnyTypeClass<int[]> {
    return new AnyTypeClass<int[]>(arg);
}

function create_any_type_getter_class_from_ets_obj(arg: object): AnyTypeClass<object> {
    return new AnyTypeClass<object>(arg);
}

function create_any_type_getter_class_from_ets_tuple(arg: Tuple): AnyTypeClass<Tuple> {
    return new AnyTypeClass<Tuple>(arg);
}

function create_any_type_getter_class_from_ets_union(arg: Union): AnyTypeClass<Union> {
    return new AnyTypeClass<Union>(arg);
}

class SubsetByRef {
    private _refClass: PublicGetterClass;

    constructor() {
        this._refClass = new PublicGetterClass();
    }

    public get value(): string {
        return this._refClass.value;
    }
}

function create_subset_by_ref_getter_class_from_ets(): SubsetByRef {
    return new SubsetByRef();
}

class SubsetByValueClass {
    public _value: string;

    constructor(value: string) {
        this._value = value;
    }

    get value(): string {
        return this._value;
    }
}

function create_subset_by_value_getter_class_from_ets(): SubsetByValueClass {
    const GClass = new PublicGetterClass();

    return new SubsetByValueClass(GClass.value);
}
