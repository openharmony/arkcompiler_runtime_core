function main(): int {
    let failures: int = 0;
    failures += test(createEmptySet(),"Create Empty Set");
    failures += test(createSetAndAdd(), "Create Set and Add single item");

    failures += test(createSetAddAndClear(), "Create Set add and clear it");
    failures += test(createSetAddAndRemove(), "Create Set add and remove some key");

    failures += test(createSetAndGetValues(), "Get All Values from empty set");
    failures += test(createSetAddAndGetValues(), "Get All Values from non empty set");

    failures += test(createSetAddRemoveAndGetValues(), "Get Values after some value removed");
    failures += test(createSetAddClearAndGetValues(), "Get Values after all values removed");

    failures += test(createSetAddAndCheckHasValue(),"Check set has given value");
    return test(failures, "All tests run");

}

function createSetAddAndCheckHasValue(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'b');
    set.put(c'c');
    let result = set.hasValue(c'c') ? 0 : 1;
    return result;
}

function createEmptySet(): int {
    let set: Set<char> = new Set<char>;
    return set.length();
}

function createSetAndGetValues(): int {
    let set: Set<char> = new Set<char>;
    let values: Char[] = set.values();

    let result = values.length == 0 ? 0 : 1;
    return result;
}

function createSetAddAndGetValues(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'c');
    set.put(c'b');
    set.put(c'c');
    let values: Char[] = set.values();

    let result = values.length == 3 ? 0 : 1;
    return result;
}

function createSetAddRemoveAndGetValues(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'b');
    set.put(c'c');
    set.remove(c'c');
    let values: Char[] = set.values();
    let result = values.length == 2 ? 0 : 1;
    return result;
}

function createSetAddClearAndGetValues(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'b');
    set.put(c'c');
    set.put(c'c');
    set.clear();
    let values: Char[] = set.values();
    let result = values.length == 0 ? 0 : 1;
    return result;
}

function createSetAddAndRemove(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'b');
    set.put(c'c');
    set.remove(c'c');
    let result = set.length() == 2 ? 0 : 1;
    return result;
}

function createSetAddAndClear(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    set.put(c'b');
    set.put(c'c');
    set.clear();
    let result = set.length() == 0 ? 0 : 1;
    return result;
}

function createSetAndAdd(): int {
    let set: Set<char> = new Set<char>;
    set.put(c'a');
    let result = set.length() == 1 ? 0 : 1;
    return result;
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println('PASSED: ' + message);
      return 0;
    }
    console.println('FAILED: ' + message);
    return 1;
}
