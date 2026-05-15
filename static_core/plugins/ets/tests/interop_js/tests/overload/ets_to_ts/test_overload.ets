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