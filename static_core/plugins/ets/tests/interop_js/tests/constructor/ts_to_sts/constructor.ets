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
    tsInt,
    NamedClass,
    createNamedClassFromTs,
    namedClassInstance,
    //
    AnonymousClass,
    createAnonymousClassFromTs,
    anonymousClassInstance,
    //
    FunctionConstructor,
    createFunctionConstructorFromTs,
    functionConstructorInstance,
    //
    IIFEClass,
    createIIFEClassFromTs,
    IIFEClassInstance,
    IIFEConstructor,
    createIIFEConstructorFromTs,
    IIFEConstructorInstance,
    //
    MethodCreateConstructor,
    createMethodConstructorClass,
    methodConstructorInstance,
    methodCreateAnonymousClass,
    methodCreateClassInstance,
    //
    SimpleObject,
    simpleArrowFunction,
    //
    AbstractClass,
    createAbstractClassFromTs,
    abstractClassInstance,
    //
    ChildClass,
    createChildClassFromTs,
    childClassInstance,
} from 'main_js';

function checkNamedClass(): boolean {
    const TSClass = new NamedClass(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkCreateNamedClassFromTs(): boolean {
    return createNamedClassFromTs()._value as int == tsInt as int;
}

function checkNamedClassInstance(): boolean {
    return namedClassInstance._value as int == tsInt as int;
}

function checkAnonymousClass(): boolean {
    const TSClass = new AnonymousClass(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkCreateAnonymousClassFromTs(): boolean {
    return createAnonymousClassFromTs()._value as int == tsInt as int;
}

function checkAnonymousClassInstance(): boolean {
    return anonymousClassInstance._value as int == tsInt as int;
}

function checkFunctionConstructor(): boolean {
    const TSClass = new FunctionConstructor(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkCreateFunctionConstructorFromTs(): boolean {
    const TSClass = createFunctionConstructorFromTs();

    return TSClass._value as int == tsInt as int;
}

function checkFunctionConstructorInstance(): boolean {
    return functionConstructorInstance._value as int == tsInt as int;
}

function checkIIFEClass(): boolean {
    const TSClass = new IIFEClass(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkIIFEClassError(): boolean {
    try {
        const TSClass = IIFEClass(tsInt);

        return false;
    } catch (e) {
        console.log(e);

        return true;
    }
}

function checkCreateIIFEClassFromTs(): boolean {
    const TSClass = createIIFEClassFromTs()

    return TSClass._value as int == tsInt as int;
}

function checkIIFEClassInstance(): boolean {
    return IIFEClassInstance._value as int == tsInt as int;
}

function checkIIFEConstructor(): boolean {
    const TSClass = new IIFEConstructor(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkIIFEConstructorUndefined(): boolean {
    const TSClass = IIFEConstructor(tsInt);

    return TSClass as NullishType == undefined;
}

function checkCreateIIFEConstructorFromTs(): boolean {
    const TSClass = createIIFEConstructorFromTs()

    return TSClass._value as int == tsInt as int;
}

function checkIIFEConstructorInstance(): boolean {
    return IIFEConstructorInstance._value as int == tsInt as int;
}

//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkMethodCreateConstructor(): boolean {
//     const ParentClass = new MethodCreateConstructor();
//     const anonymousClass = ParentClass.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkCreateMethodConstructorClass(): boolean {
//     const ParentClass = createMethodConstructorClass();
//     const anonymousClass = ParentClass.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkMethodConstructorInstance(): boolean {
//     const anonymousClass = methodConstructorInstance.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

function checkMethodCreateAnonymousClass(): boolean {
    const TSClass = new methodCreateAnonymousClass(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkMethodCreateClassInstance(): boolean {
    return methodCreateClassInstance._value as int == tsInt as int;
}

function checkSimpleObject(): boolean {
    try {
        new SimpleObject();

        return false;
    } catch (e) {
        return true;
    }
}

function checkSimpleArrowFunction(): boolean {
    try {
        const test = new simpleArrowFunction(tsInt);

        return false;

    } catch (e) {
        return true;
    }
}

function checkAbstractClass(): boolean {
    const TSClass = new AbstractClass(tsInt);

    return TSClass._value as int == tsInt as int;
}

function checkCreateAbstractClassFromTs(): boolean {
    const TSClass = createAbstractClassFromTs();

    return TSClass._value as int == tsInt as int;

}

function checkAbstractInstance(): boolean {
    const TSClass = abstractClassInstance;

    return TSClass._value as int == tsInt as int;

}

function checkChildClass(): boolean {
    const TSClass = new ChildClass(tsInt);

    return TSClass._value as int == tsInt as int;

}

function checkCreateChildClassFromTs(): boolean {
    const TSClass = createChildClassFromTs();

    return TSClass._value as int == tsInt as int;

}

function checkChildInstance(): boolean {
    const TSClass = childClassInstance;

    return TSClass._value as int == tsInt as int;
}
