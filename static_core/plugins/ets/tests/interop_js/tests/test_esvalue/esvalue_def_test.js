'use strict';
/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
export let n = 1;
export let b = false;
export let s = 'hello';
export let bi = 9007199254740991n;
export let jsUndefined = undefined;
export let jsNull = null;
export let jsFunc = function () { return 6; };
export let jsFuncWithParam = function (a, b) { return a + b; };
export let A = {
    'property1': 1,
    'property2': 2
};

export let B = {
    'property1': 'aaaaa',
    'property2': 'bbbbb'
};
export let jsArray = ['foo', 1, true];
export let jsArray1 = ['foo', 1, true];
export let objWithFunc = {
    'func': function () {
        return 'hello';
    }
};
export let objWithFuncParam = {
    'hello': function (a, b) {
        return a + b;
    }
};

export let jsIterable = ['a', 'b', 'c', 'd'];
export let testItetatorObject = {'a': 1, 'b': 2, 'c' : 3};

export class User {
	ID = 123;
}

export let doubledObj = {
    3.2: 'aaa',
    4.5: 'bbb',
};

export let propObj = {
    'property1': 'aaaaa',
    'property2': 'bbbbb',
    123: 456,
};

export function throwNull() {
    throw null;
}

export function throwUndefined() {
    throw undefined;
}

export let objWithProto = {
    ownProp: 'own',
};

Object.defineProperty(objWithProto, 'nonEnumerable', {
    value: 'hidden',
    enumerable: false
});

export let complexObject = {
    nested: {
        level2: {
            value: 'deep'
        }
    },
    array: [1, 2, 3],
    func: function() {
        return 'method';
    }
};

export let objectWithMethods = {
    method1: function() {
        return 'method1';
    },
    method2: function(a, b) {
        return a + b;
    },
    value: 42
};

export let objWithNulls = {
    nullProp: null,
    undefProp: undefined,
    normalProp: 'value'
};

export class TestClass {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }

    sum() {
        return this.x + this.y;
    }
}

export let customIterable = {
    [Symbol.iterator]: function() {
        let step = 0;
        return {
            next: function() {
                if (step < 3) {
                    step++;
                    return { value: step * 10, done: false };
                }
                return { value: undefined, done: true };
            }
        };
    }
};

export let objWithProtoProps = { own: 'own' };
Object.setPrototypeOf(
  objWithProtoProps,
  { inherited: 'inherited' }
);

export class Animal {
    name = 'animal';
    age = 0;
    
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }

    speak() {
        return this.name + ' makes a sound';
    }

    getInfo() {
        return this.name + ' is ' + this.age + ' years old';
    }
}

export class Dog extends Animal {
    breed;

    constructor(name, age, breed) {
        super(name, age);
        this.breed = breed;
    }

    speak() {
        return this.name + ' barks: Woof!';
    }

    fetch() {
        return this.name + ' fetches the ball';
    }

    getInfo() {
        return super.getInfo() + ' and is a ' + this.breed;
    }
}

export class Cat extends Animal {
    color;

    constructor(name, age, color) {
        super(name, age);
        this.color = color;
    }

    speak() {
        return this.name + ' meows: Meow!';
    }

    getFullInfo() {
        return 'Cat - ' + super.getInfo();
    }
}

export let myDog = new Dog('Buddy', 3, 'Golden Retriever');
export let myCat = new Cat('Whiskers', 2, 'white');

export let sym = Symbol('description');
export let objWithSym = {
    [Symbol('hidden')]: 'secret1',
    [sym]: 'secret2',
    normalProp: 'visible'
};

export let reflectTarget = {
    x: 10,
    y: 20,
    sum: function() { return this.x + this.y; }
};

export function reflectFunc(a, b, c) {
    return a + b + c;
}
export let reflectReceiver = { x: 5, y: 10 };

export let frozenObj = Object.freeze({ x: 1, y: 2 });
export let sealedObj = Object.seal({ a: 10, b: 20 });
export let extensibleObj = { prop: 'value' };

export let funcToBind = function(a, b) {
    return this.x + a + b;
};
export let bindContext = { x: 100 };
export let funcBound = funcToBind.bind(bindContext, 10);
export function funcToApply(a, b, c) {
    return this.base + a + b + c;
}
export let applyContext = { base: 1000 };

export let nestedObj = {
    level1: {
        level2: {
            level3: { deep: 'value' }
        }
    },
    arr: [1, 2, { nested: 'array item' }]
};

export let jsonSpecialValues = {
    str: 'text',
    num: 123,
    bool: true,
    nullVal: null,
    date: new Date('2025-01-01')
};

export let circularObj = { name: 'root', value: 42 };
circularObj.self = circularObj;

export let testMap = new Map([
    ['key1', 'value1'],
    ['key2', 'value2'],
    [123, 'number key']
]);

export let testSet = new Set([1, 2, 3, 2, 1]);

export let testWeakMap = new WeakMap();
let weakMapKey1 = { id: 1 };
let weakMapKey2 = { id: 2 };
testWeakMap.set(weakMapKey1, 'weak value 1');
testWeakMap.set(weakMapKey2, 'weak value 2');
export let weakMapKeys = [weakMapKey1, weakMapKey2];

export let testWeakSet = new WeakSet();
let weakSetItem1 = { id: 1 };
let weakSetItem2 = { id: 2 };
testWeakSet.add(weakSetItem1);
testWeakSet.add(weakSetItem2);
export let weakSetItems = [weakSetItem1, weakSetItem2];

export let sparseArray = [1, , , 4];
export let arrayWithHoles = new Array(5);
arrayWithHoles[0] = 'first';
arrayWithHoles[4] = 'last';
export let arrayLikeObj = {
    0: 'a',
    1: 'b',
    2: 'c',
    length: 3
};
export let nonElementProperty = [1, 2, 3];
nonElementProperty['custom'] = 'not an index';
nonElementProperty[-1] = 'negative index';

export let testDate = new Date('2025-03-15T10:30:00Z');
export let invalidDate = new Date('invalid');
export let epochDate = new Date(0);
export let futureDate = new Date('2100-01-01');