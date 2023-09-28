/*---
desc: Declaring subinterface
assert: If an extends clause is provided, then the interface being declared extends each of the other named interfaces and therefore inherits the member types, instance methods, and constants of each of the other named interfaces.
tags: [compile-only]
---*/

interface A {
  foo(): int {
    return 5;
  }
}

interface B {
  bar(): int {
    return 5;
  }
}

interface C extends A, B {}

class D implements C {
    public sum(): void {
        let r: int = foo + bar();
    }
}