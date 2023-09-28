/*---
desc:  {function: Test_escompat_Array }
---*/

function main(): int {
    let failures: int = 0;

    failures += check(testCreateFromString(), "Test Create Array object from String");
    failures += check(testCreateFromString1(), "Test Create Array object from String with mapping func");
    failures += check(testCreateFromString2(), "Test Create Array object from String with mapping func");
    failures += check(testFiltered(), "Test Create Array object from given with filter function");

    return check(failures, "All tests run");
}


function testCreateFromString(): int {
                        
    let src: String = "Hello World";
    let test: Array = Array.from(src);
    if(test.length() != src.length()) {
        console.println("Array size mismatch");
        return 1;
    }
    for(let i: int = 0; i < src.length(); i++) {
        let cc: char = src.charAt(i);
        let tmp: String = test.at(i) as String;
        let ac: char = tmp.charAt(0);
        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}

function testCreateFromString1(): int {
    let src: String = "HELLO";
    let test: Array = Array.from(src, (k: int, item: Object) => { return  (item as String).toLowerCase(); });
    if(test.length() != src.length()) {
        console.println("Array size mismatch");
        return 1;
    }
    let exp: String = "hello";

    for(let i: int = 0; i < exp.length(); i++) {
        let tmp: String = test.at(i) as String;
        let ac: char = tmp.charAt(0);
        let cc: char = exp.charAt(i);

        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}

function testCreateFromString2(): int {
    let src: String = "Hello";
    let test: Array = Array.from(src, (k: int, v: Object) => { return k; });
    if(test.length() != src.length()) {
        console.println("Array size mismatch");
        return 1;
    }
    for(let i: int = 0; i < src.length(); i++) {
        let av: Int = test.at(i) as Int;
        if(av == i) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}


function testFiltered(): int {
    let src: String = "HHeelllloo";
    let exp: String = "Hello";
    let test: Array = Array.from(src).filter((k: int, v: Object) => { return k%2 == 0 });

    if(test.length() != exp.length()) {
        console.println("Array size mismatch");
        return 1;
    }
    
    for(let i: int = 0; i < exp.length(); i++) {
        let tmp: String = test.at(i) as String;
        let ac: char = tmp.charAt(0);
        let cc: char = exp.charAt(i);

        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }

    return 0;
}


function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

