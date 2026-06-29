/*
* Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export interface CSSRuleLike {
    mergeRule(rule: CSSRuleLike): void;
}

export class CSSRule implements CSSRuleLike {
    mergeRule(rule: CSSRuleLike): void {
    }
}

export class CSSLogicRule extends CSSRule {
    mergeRule(rule: CSSRule): void {
    }
}

export function createCSSLogicRule(): CSSLogicRule {
    return new CSSLogicRule();
}

export class CountOverloadRule {
    mergeRule(rule: int): string {
        return 'one';
    }

    mergeRule(rule1: int, rule2: int): string {
        return 'two';
    }

    mergeRule(rule1: int, rule2: int, rule3: int): string {
        return 'three';
    }
}

export function createCountOverloadRule(): CountOverloadRule {
    return new CountOverloadRule();
}

export class TypeOverloadRule {
    mergeRule(rule: byte): string {
        return 'byte';
    }

    mergeRule(rule: short): string {
        return 'short';
    }
    mergeRule(rule: int): string {
        return 'int';
    }
    mergeRule(rule: long): string {
        return 'long';
    }

    mergeRule(rule: float): string {
        return 'float';
    }
    mergeRule(rule1: string): string {
        return 'string';
    }

    mergeRule(rule: double): string {
        return 'double';
    }

    mergeRule(rule: boolean): string {
        return 'boolean';
    }

    mergeRule(rule: bigint): string {
        return 'bigint';
    }

    mergeRule(rule: Array<int>): string {
        return 'array';
    }

    mergeRule(rule: Map<string, int>): string {
        return 'map';
    }

    mergeRule(rule: Set<int>): string {
        return 'set';
    }

    static mergeRuleStatic(rule: string): string {
        return 'string';
    }

    static mergeRuleStatic(rule: byte): string {
        return 'byte';
    }

    static mergeRuleStatic(rule: short): string {
        return 'short';
    }

    static mergeRuleStatic(rule: int): string {
        return 'int';
    }

    static mergeRuleStatic(rule: long): string {
        return 'long';
    }

    static mergeRuleStatic(rule: float): string {
        return 'float';
    }

    static mergeRuleStatic(rule: double): string {
        return 'double';
    }

    static mergeRuleStatic(rule: boolean): string {
        return 'boolean';
    }

    static mergeRuleStatic(rule: bigint): string {
        return 'bigint';
    }

    static mergeRuleStatic(rule: Array<int>): string {
        return 'array';
    }

    static mergeRuleStatic(rule: Map<string, int>): string {
        return 'map';
    }

    static mergeRuleStatic(rule: Set<int>): string {
        return 'set';
    }
}

export function createTypeOverloadRule(): TypeOverloadRule {
    return new TypeOverloadRule();
}


export class ComplexOverloadRule {
    mergeRule(...rule: int[]): string {
        return 'rest int';
    }

    mergeRule(rule: string | int): string {
        return 'string';
    }

    mergeRule(rule1: string, rule2: byte): string {
        return 'string byte';
    }
    mergeRule(rule1: string, rule2: short): string {
        return 'string short';
    }
    mergeRule(rule1: string, rule2: int): string {
        return 'string int';
    }
    mergeRule(rule1: string, rule2: long): string {
        return 'string long';
    }
    mergeRule(rule1: string, rule2: float): string {
        return 'string float';
    }
    mergeRule(rule1: string, rule2: double): string {
        return 'string double';
    }
    mergeRule(rule1: string, rule2: boolean): string {
        return 'string boolean';
    }
    mergeRule(rule1: string, rule2: bigint): string {
        return 'string bigint';
    }
    mergeRule(rule1: string, rule2: string): string {
        return 'two string';
    }
}

export function createComplexOverloadRule(): ComplexOverloadRule {
    return new ComplexOverloadRule();
}

export class GenericOverloadRule {
    mergeRule<T>(rule: T): string {
        return 'generic';
    }

    mergeRule(rule: int): string {
        return 'int';
    }

    mergeRule(rule: string): string {
        return 'string';
    }
}

export function createGenericOverloadRule(): GenericOverloadRule {
    return new GenericOverloadRule();
}

export class Animal {}
export class Dog extends Animal {}
export class Cat extends Animal {}

export class ObjectOverloadHandler {
    handlePet(obj: Dog): string {
        return 'dog';
    }
    handlePet(obj: Cat): string {
        return 'cat';
    }

    handleSpecific(obj: Dog): string {
        return 'dog';
    }
    handleSpecific(obj: Any): string {
        return 'object';
    }

    handleAnimal(obj: Dog): string {
        return 'dog';
    }
    handleAnimal(obj: Animal): string {
        return 'animal';
    }
}

export function createObjectOverloadHandler(): ObjectOverloadHandler {
    return new ObjectOverloadHandler();
}

export function createDog(): Dog {
    return new Dog();
}

export function createCat(): Cat {
    return new Cat();
}

export function createAnimal(): Animal {
    return new Animal();
}

export class ObjectOverloadPrimitiveHandler {
    handleIntOrAny(val: int): string {
        return 'int';
    }
    handleIntOrAny(val: Any): string {
        return 'any';
    }

    handleStringOrAny(val: string): string {
        return 'string';
    }
    handleStringOrAny(val: Any): string {
        return 'any';
    }

    handleAnyOrInt(val: Any): string {
        return 'any';
    }
    handleAnyOrInt(val: int): string {
        return 'int';
    }

    handleDogOrAny(val: Dog): string {
        return 'dog';
    }
    handleDogOrAny(val: Any): string {
        return 'object';
    }
}

export function createObjectOverloadPrimitiveHandler(): ObjectOverloadPrimitiveHandler {
    return new ObjectOverloadPrimitiveHandler();
}

export class CheckArgMatchObjectHandler {
    handleStringOrAny(val: string): string {
        return 'string';
    }
    handleStringOrAny(val: Any): string {
        return 'any';
    }

    handleBigIntOrAny(val: bigint): string {
        return 'bigint';
    }
    handleBigIntOrAny(val: Any): string {
        return 'any';
    }

    handleFnOrAny(val: int): string {
        return 'int';
    }
    handleFnOrAny(val: Any): string {
        return 'function';
    }

    handleBoolOrAny(val: boolean): string {
        return 'boolean';
    }
    handleBoolOrAny(val: Any): string {
        return 'any';
    }

    handleDoubleOrAny(val: double): string {
        return 'double';
    }
    handleDoubleOrAny(val: Any): string {
        return 'any';
    }

    handleTwoArgs(val1: string, val2: int): string {
        return 'string int';
    }
    handleTwoArgs(val1: Any, val2: Any): string {
        return 'any any';
    }

    handleDogOrAnimalOrAny(val: Dog): string {
        return 'dog';
    }
    handleDogOrAnimalOrAny(val: Animal): string {
        return 'animal';
    }
    handleDogOrAnimalOrAny(val: Any): string {
        return 'any';
    }

    handleThreeSpecific(val1: string, val2: int, val3: Dog): string {
        return 'string int dog';
    }
    handleThreeSpecific(val1: Any, val2: Any, val3: Any): string {
        return 'any any any';
    }

    handleThreeInherit(val1: Dog, val2: Dog, val3: Dog): string {
        return 'dog dog dog';
    }
    handleThreeInherit(val1: Animal, val2: Animal, val3: Any): string {
        return 'animal animal any';
    }
    handleThreeInherit(val1: Any, val2: Any, val3: Any): string {
        return 'any any any';
    }
}

export function createCheckArgMatchObjectHandler(): CheckArgMatchObjectHandler {
    return new CheckArgMatchObjectHandler();
}