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
    BOOLEAN_VALUE,
    ENUM_VALUE,
    GenericInterface,
    STRING_VALUE,
    TFunctionReturnsAny,
    TFunctionReturnsBigInt,
    TFunctionReturnsBoolean,
    TFunctionReturnsEnum,
    TFunctionReturnsFunctionOfType,
    TFunctionReturnsNull,
    TFunctionReturnsNumber,
    TFunctionReturnsString,
    TFunctionReturnsUndefined,
    UNDEFINED,
    genericInterfaceImplementation
} from "js_code";


/* Note: nullish \ undefish types here are workarounds for note 17745*/
type UndefishHelperType = undefined | string | number;
type NullishHelperType = null | string | number

function cleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
}

function type_imported__returnAny(){
    const anyGetter: TFunctionReturnsAny = genericInterfaceImplementation.getAny as TFunctionReturnsAny;
    return typeof anyGetter() == 'object';
}

function type_imported__returnString(){
    const getterFn: TFunctionReturnsString = genericInterfaceImplementation.getString as TFunctionReturnsString;
    return typeof getterFn() == "object" && (getterFn() as string == 'Panda')
}

function type_imported__returnBoolean(){
    const getterFn: TFunctionReturnsBoolean = genericInterfaceImplementation.getBoolean;
    return typeof getterFn() == "object" && (getterFn() as boolean) == true && (getterFn() as boolean) != false;
}

function type_imported__returnInteger(){
    const getterFn: TFunctionReturnsNumber = genericInterfaceImplementation.getInt as TFunctionReturnsNumber;
    return typeof getterFn() == "object" && (getterFn() as int) > 0 && (getterFn() as int) / 2 == (getterFn() as int) * 0.5;
}

function type_imported__returnNegativeInteger(){
    const getterFn: TFunctionReturnsNumber = genericInterfaceImplementation.getNegativeInt;
    return typeof getterFn() == "object" && (getterFn() as int) < 0 && (getterFn() as int) * -1 > 0;
}


function type_imported__returnInfinity(){
    const getterFn: TFunctionReturnsNumber = genericInterfaceImplementation.getInfinity;
    const isValid: boolean = (typeof getterFn() == "object")
            && ((getterFn() as number) > 0)
            && ((getterFn() as number) - 1 == getterFn())
            && !Number.isFinite(getterFn() as number);
    cleanup();
    return isValid;
}

function type_imported__returnNegativeInfinity(){
    const getterFn: TFunctionReturnsNumber = genericInterfaceImplementation.getNegativeInfinity;
    const isValid: boolean = (typeof getterFn() == "object")
            && ((getterFn() as number) < 0)
            && ((getterFn() as number) + 1 == getterFn())
            && !Number.isFinite(getterFn() as number);
    cleanup();
    return isValid;
}


function type_imported__returnNaN(){
    const getterFn: TFunctionReturnsNumber = genericInterfaceImplementation.getNanAsNumber as TFunctionReturnsNumber;
    const isValid: boolean = `${(getterFn() as number)}` == 'NaN' && (Number.isNaN(getterFn()));
    cleanup();
    return isValid;
}

function type_imported__returnBigint(){
    const getterFn: TFunctionReturnsBigInt = genericInterfaceImplementation.getBigInt;
    return typeof getterFn() == "object"
}

function type_imported__returnEnum(){
    const getterFn: TFunctionReturnsEnum = genericInterfaceImplementation.getEnum;
    const returnValue = getterFn();
    const isAbleToGetExistingProperties = ((returnValue.OPTION_ONE as int == 0)
                                            && (returnValue.OPTION_TWO as number == 1)
                                            && (returnValue.OPTION_THREE as number == 2));
    const isReturningUndefForMissingProperties = returnValue.OPTION_FOUR as UndefishHelperType == undefined;
    cleanup();
    return typeof getterFn() == "object" && isAbleToGetExistingProperties && isReturningUndefForMissingProperties
}

function type_imported__returnUndefined(){
    const getterFn: TFunctionReturnsUndefined = genericInterfaceImplementation.getUndefined;
    const returnValue = getterFn() as UndefishHelperType;
    cleanup();
    return returnValue == undefined && returnValue != '';
}

function type_imported__returnNull(){
    const getterFn: TFunctionReturnsNull = genericInterfaceImplementation.getNull;
    const returnValue = getterFn() as NullishHelperType;
    return returnValue == null;
}


function type_imported__returnFunction(){
    const getterFn: TFunctionReturnsFunctionOfType = genericInterfaceImplementation.getFunctionReturningType;
    const returnStringValue = getterFn('panda');
    const returnNumericValue = getterFn(1);
    return returnStringValue() == 'pandaPANDA'
            && (returnNumericValue() as float)> 3
            && (returnNumericValue() as float) < 4;
}

