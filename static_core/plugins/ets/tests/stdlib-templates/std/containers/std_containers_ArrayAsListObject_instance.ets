/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for item in std_containers_ArrayAsListObject_instance %}

/*---
desc:  {function: With{{.item.itemType}} }
---*/

function main(): int {
    let failures: int = 0;
    failures += check((): int => { return createDefault()}, "Create default empty list");
    
    failures += check((): int => { return createWithGivenZeroSize()}, "Create list with given zero size");
    failures += check((): int => { return createWithGivenSize()}, "Create list with given size");
    failures += check((): int => { return createWithGivenSizeAndInitialValue()}, "Create list with given size and given init value");

    failures += check((): int => { return pushFrontToEmptyList()}, "Create empty list and push item in front");
    failures += check((): int => { return pushBackToEmptyList()}, "Create empty list and push item in back");
    
    failures += check((): int => { return pushFrontToNonEmptyList()}, "Create non empty list and push item in front");
    failures += check((): int => { return pushBackToNonEmptyList()}, "Create non empty list and push item in back");

    failures += check((): int => { return pushAndPopFrontFromNonEmptyList()}, "Push and Pop from/to start of non empty list");
    failures += check((): int => { return pushAndPopBackFromNonEmptyList()}, "Push and Pop from/to end of non empty list");

    failures += check((): int => { return emptyToArray()}, "Test empty toArray method");
    failures += check((): int => { return toArray()}, "Test toArray method");

    failures += check((): int => { return updateCapacityNoChange()}, "Update list capacity with no change");
    failures += check((): int => { return updateCapacityWithIncrease()}, "Update list capacity with increase");
    failures += check((): int => { return updateCapacityWithDecrease()}, "Update list capacity with decrease");

    failures += check((): int => { return createWithNegativeInitialCapacity()}, "Negative array size passed. Expected to throw NegativeArraySizeError error");
    failures += check((): int => { return accessOutOfBoundsValue()}, "Access value out of bounds. Expected to throw ArrayIndexOutOfBoundsError")
    
    return check(failures, "All tests run");
}


function createDefault(): int {
    let list = new ArrayAsListObject();
    return list.size();
}

function createWithNegativeInitialCapacity(): int {
    try {
        new ArrayAsListObject(-1);
    } catch(nse: NegativeArraySizeError) {
        return 0;
    }
    return 1;
}

function accessOutOfBoundsValue(): int {
    let list = new ArrayAsListObject(10);
    try {
        for(let i: int = 0; i< 11; i++) {
            list.at(i);
        }
    } catch(aiobe: ArrayIndexOutOfBoundsError) {
        return 0;
    }
    return 1;
}


function createWithGivenZeroSize(): int {
    let list = new ArrayAsListObject(0);
    let result = list.size() == 0 ? 0 : 1; 
    return result;
}

function createWithGivenSize(): int {
    let list = new ArrayAsListObject(10);
    let result = list.size() == 10 ? 0 : 1;
    return result;    
}

function createWithGivenSizeAndInitialValue(): int {
    let value = {{.item.itemValue}};
    let list = new ArrayAsListObject(10, value);

    for(let i: int = 0; i< 10; i++) {
        let given = list.at(i) as {{.item.itemType}};
        if(given != value) return 1;
    }
    let result = list.size() == 10 ? 0 : 1;
    return result; 
}

function pushFrontToEmptyList(): int {
    let value = {{.item.itemValue}};
    let list = new ArrayAsListObject();
    list.pushFront(value);

    let expected = list.at(0) as {{.item.itemType}};
    let result = (expected == value && list.size() == 1) ? 0 : 1;
    return result;
}

function pushBackToEmptyList(): int {
    let value = {{.item.itemValue}};
    let list = new ArrayAsListObject();
    list.pushBack(value);

    let expected = list.at(0) as {{.item.itemType}};
    let result = (expected == value && list.size() == 1) ? 0 : 1;
    return result;
}


function pushFrontToNonEmptyList(): int {
    let init = {{.item.initValue}};
    let value = {{.item.itemValue}};
    let list = new ArrayAsListObject(9, init);
    list.pushFront(value);

    let expected = list.at(0) as {{.item.itemType}};
    let result = (expected == value && list.size() == 10) ? 0 : 1;
    return result;    
}

function pushBackToNonEmptyList(): int {
    let init = {{.item.initValue}};
    let value = {{.item.itemValue}};
    let list = new ArrayAsListObject(9, init);
    list.pushBack(value);

    let expected = list.at(9) as {{.item.itemType}};
    let result = (expected == value && list.size() == 10) ? 0 : 1;
    return result;
}

function pushAndPopFrontFromNonEmptyList(): int {
    let init = {{.item.initValue}};
    let list = new ArrayAsListObject(10, init);
    let value = {{.item.itemValue}};
    
    list.pushFront(value);
    let expected = list.popFront() as {{.item.itemType}};
    let head = list.at(0) as {{.item.itemType}};

    let result = (expected == value && head == init && list.size() == 10) ? 0 : 1;
    return result;

}

function pushAndPopBackFromNonEmptyList(): int {
    let init = {{.item.initValue}};
    let list = new ArrayAsListObject(10, init);
    let value = {{.item.itemValue}};
    
    list.pushBack(value);
    let expected = list.popBack() as {{.item.itemType}};
    let tail = list.at(9) as {{.item.itemType}};

    let result = (expected == value && tail == init && list.size() == 10) ? 0 : 1;
    return result;

}

function emptyToArray(): int {
    let list = new ArrayAsListObject();
    let target: Object[] = list.toArray();
    if(target.length == 0) {
        return 0;
    }
    console.println("Output array length mismatch");
    return 1;
}

function toArray(): int {
    let value: {{.item.itemType}} = {{.item.itemValue}};
    let list = new ArrayAsListObject(10);

    for(let j: int = 0; j< 10; j++) {
        list.set(j, value);    
    }

    let target: Object[] = list.toArray();
    if(target.length != 10) {
        console.println("Output array length mismatch");
        return 1;
    }

    for(let i: int = 0; i< target.length; i++) {
        if(target[i] as {{.item.itemType}} == value as {{.item.itemType}}) continue;
        console.println("Unexpected value in array");
        return 1;
    }
    return 0;
}

function updateCapacityNoChange(): int {
    let list = new ArrayAsListObject(100);
    list.reserve(100);
    let result = list.size() == 100 ? 0: 1;
    return result;
}

function updateCapacityWithIncrease(): int {
    let list = new ArrayAsListObject(100);
    list.reserve(200);
    let result = list.size() == 100 ? 0: 1;
    return result;
}

function updateCapacityWithDecrease(): int {
    let list = new ArrayAsListObject(100);
    list.reserve(50);

    let result = list.size() == 100 ? 0: 1;
    return result;
}



function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Exception catched: " + e);
        return 1;
    }

    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

{%- endfor %}
