/*---
desc: >-
    A continue statement with label identifier transfers control out of the enclosing
    loop statement which has the same label identifier. Such statement must be in the
    body of the surrounding function or method otherwise compile-time error is raised
    by the compiler.
tags: [compile-only, negative]
---*/

function foo() {
    loop1: for (let v: int = 0; v < 50; v +=10) {
        if (v % 20 == 0) {
            continue loop1;
        }
        bar(v);
    }
}

function bar(v: int) {
    loop2: for (let v: int = 0; v < 10; v++) {
        if (v % 2 == 0) {
            continue loop2;
        }
        if (v % 5 == 0) {
            continue loop1;  // to outside of surrounding function
        }
    }
}

function main(): int {
    foo()
    return 0;
}
