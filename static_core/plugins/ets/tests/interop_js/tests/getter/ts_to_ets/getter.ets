/**
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

import {
    tsString,
    tsNumber,
    PublicGetterClass,
    createPublicGetterClassFromTs,
    publicGetterInstanceClass,
    UnionTypeClass,
    createUnionTypeGetterClassFromTs,
    unionTypeGetterInstanceClassInt,
    unionTypeGetterInstanceClassString,
    LiteralClass,
    createLiteralTypeGetterClassFromTs,
    literalTypeGetterInstanceClassInt,
    literalTypeGetterInstanceClassString,
    TupleType,
    TupleTypeClass,
    createTupleTypeGetterClassFromTs,
    tupleTypeGetterInstanceClass,
    AnyTypeClass,
    createAnyTypeGetterClassFromTs,
    anyTypeGetterInstanceClass,
    anyTypeExplicitGetterInstanceClass,
    SubsetByRef,
    createSubsetByRefGetterClassFromTs,
    subsetByRefInstanceClass,
    SubsetByValueClass,
    createSubsetByValueGetterClassFromTs,
    subsetByValueInstanceClass,
} from 'dynamic_import';


const tupleArr: TupleType = [tsNumber, tsString] as FixedArray<JSValue>;
const etsNumber = 1;
const etsString = 'string';
const etsArr: FixedArray<int> = [etsNumber];
const etsBool = true;
const etsObj: Object = {};
const etsUnion: int | string = etsNumber;

function checkGetterPublicClass(): boolean {
    const GClass = new PublicGetterClass();

    return GClass.value as string == tsString as string;
}

function checkCreatePublicGetterClassFromTs(): boolean {
    const GClass = createPublicGetterClassFromTs();

    return tsString as string == GClass.value as string;
}

function checkPublicGetterInstanceClassFromTs(): boolean {
    return tsString as string == publicGetterInstanceClass.value as string;
}

function checkUnionTypeGetterClassInt(): boolean {
    const GClass = new UnionTypeClass(tsNumber);

    return GClass.value as int == tsNumber as int;
}

function checkCreateUnionTypeGetterClassFromTsInt(): boolean {
    const GClass = createUnionTypeGetterClassFromTs(tsNumber);

    return GClass.value as int == tsNumber as int;
}

function checkInstanceUnionTypeGetterClassFromTsInt(): boolean {
    return tsNumber as int == unionTypeGetterInstanceClassInt.value as int;
}

function checkUnionTypeGetterClassString(): boolean {
    const GClass = new UnionTypeClass(tsString);

    return GClass.value as string == tsString as string;
}

function checkCreateUnionTypeGetterClassFromTsString(): boolean {
    const GClass = createUnionTypeGetterClassFromTs(tsString);

    return GClass.value as string == tsString as string;
}

function checkInstanceUnionTypeGetterClassFromTsString(): boolean {
    return tsString as string == unionTypeGetterInstanceClassString.value as string;
}

function checkLiteralTypeGetterClassInt(): boolean {
    const GClass = new LiteralClass(tsNumber);

    return GClass.value as int == tsNumber as int;
}

function checkLiteralTypeGetterClassString(): boolean {
    const GClass = new LiteralClass(tsString);

    return GClass.value as string == tsString as string;
}

function checkCreateLiteralTypeGetterClassFromTsInt(): boolean {
    const GClass = createLiteralTypeGetterClassFromTs(tsNumber);

    return GClass.value as int == tsNumber as int;
}

function checkCreateLiteralTypeGetterClassFromTsString(): boolean {
    const GClass = createLiteralTypeGetterClassFromTs(tsString);

    return GClass.value as string == tsString as string;
}

function checkInstanceLiteralTypeGetterClassFromTsInt(): boolean {
    return tsNumber as int == literalTypeGetterInstanceClassInt.value as int;
}

function checkInstanceLiteralTypeGetterClassFromTsString(): boolean {
    return tsString as string == literalTypeGetterInstanceClassString.value as string;
}

function checkTupleTypeGetterClass(): boolean {
    const GClass = new TupleTypeClass(tupleArr);

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkCreateTupleTypeGetterClassFromTs(): boolean {
    const GClass = createTupleTypeGetterClassFromTs(tupleArr);

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkInstanceTupleTypeGetterClassFromTs(): boolean {
    const GClass = tupleTypeGetterInstanceClass;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkAnyTypeGetterClassInt(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsNumber;

    return GClass.value as int == etsNumber as int
}

function checkAnyTypeGetterClassString(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsString;

    return GClass.value as string == etsString as string;
}

function checkAnyTypeGetterClassBool(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsBool;

    return GClass.value as boolean == etsBool as boolean;
}

function checkAnyTypeGetterClassArr(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsArr;

    return GClass.value as Array<int> instanceof Array;
}

function checkAnyTypeGetterClassObj(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsObj;

    return typeof GClass.value as Object == 'object';
}

function checkAnyTypeGetterClassTuple(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = tupleArr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkAnyTypeGetterClassUnion(): boolean {
    const GClass = new AnyTypeClass();

    GClass._value = etsUnion;

    return GClass.value as int == etsUnion as int;
}

function checkCreateAnyTypeGetterClassFromTsInt(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsNumber;

    return GClass.value as int == etsNumber as int
}

function checkCreateAnyTypeGetterClassFromTsString(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsString;

    return GClass.value as string == etsString as string;
}

function checkCreateAnyTypeGetterClassFromTsBool(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsBool;

    return GClass.value as boolean == etsBool as boolean;
}

function checkCreateAnyTypeGetterClassFromTsArr(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsArr;

    return GClass.value as Array<int> instanceof Array;
}

function checkCreateAnyTypeGetterClassFromTsObj(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsObj;

    return typeof GClass.value as Object == 'object';
}

function checkCreateAnyTypeGetterClassFromTsTuple(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = tupleArr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkCreateAnyTypeGetterClassFromTsUnion(): boolean {
    const GClass = createAnyTypeGetterClassFromTs();

    GClass._value = etsUnion;

    return GClass.value as int == etsUnion as int;
}

function checkInstanceAnyTypeGetterClassFromTsInt(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsNumber;

    return GClass.value as int == etsNumber as int
}

function checkInstanceAnyTypeGetterClassFromTsString(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsString;

    return GClass.value as string == etsString as string;
}

function checkInstanceAnyTypeGetterClassFromTsBool(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsBool;

    return GClass.value as boolean == etsBool as boolean;
}

function checkInstanceAnyTypeGetterClassFromTsArr(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsArr;

    return GClass.value as Array<int> instanceof Array;
}

function checkInstanceAnyTypeGetterClassFromTsObj(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsObj;

    return typeof GClass.value as Object == 'object';
}

function checkInstanceAnyTypeGetterClassFromTsTuple(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = tupleArr;

    return GClass.value as Array<TupleType> instanceof Array
        && GClass.value[0] as int == tupleArr[0] as int
        && GClass.value[1] as string == tupleArr[1] as string;
}

function checkInstanceAnyTypeGetterClassFromTsUnion(): boolean {
    const GClass = anyTypeGetterInstanceClass;

    GClass._value = etsUnion;

    return GClass.value as int == etsUnion as int;
}

function checkInstanceAnyExplicitTypeGetterClassFromTsExplicit(): boolean {
    const GClass = anyTypeExplicitGetterInstanceClass;

    GClass._value = etsString;

    return GClass.value as string == etsString as string;
}

function checkGetterSubsetByRefClass(): boolean {
    const GClass = new SubsetByRef();

    return GClass.value as string == tsString as string;
}

function checkCreateSubsetByRefGetterClassFromTs(): boolean {
    const GClass = createSubsetByRefGetterClassFromTs();

    return tsString as string == GClass.value as string;
}

function checkInstanceSubsetByRefGetterClassFromTs(): boolean {
    const GClass = subsetByRefInstanceClass;

    return GClass.value as string == etsString as string;
}

function checkGetterSubsetByValueClass(): boolean {
    const GClass = new SubsetByValueClass(new PublicGetterClass().value);

    return GClass.value as string == tsString as string;
}

function checkCreateSubsetByValueGetterClassFromTs(): boolean {
    const GClass = createSubsetByValueGetterClassFromTs();

    return tsString as string == GClass.value as string;
}

function checkInstanceSubsetByValueGetterClassFromTs(): boolean {
    const GClass = subsetByValueInstanceClass;

    return GClass.value as string == etsString as string;
}