/*---
desc: >-
  Expressions may contain assignments, increment operators, decrement operators,
  method calls and function calls; as a result, an evaluation of an expression
  can produce side effects.
---*/

let sg: String = "a";
let ig: int = 42;

function foo(p: int, s: String): int {
    sg += s;
    ig = -1;
    return p - s.length();
}

class A {
  meth(p: int, s: String): int {
    sg += s;
    ig = 1;
    return p - s.length();
  }
}

function main(): int {
    sg = "a"; // reset due to jit mode warmup
    ig = 42;

    // assignment side effect
    let i: int = (sg += "b").length() + (ig = 43);
    if (i != 45 || sg != "ab" || ig != 43) return 1;

    // increment side effect
    let k: int = ig++ + ++ig;
    if (k != 88 || ig != 45) return 1;

    // decrement side effect
    let j: int = ig-- + --ig;
    if (j != 88 || ig != 43) return 1;

    // function call side effect
    let m: int = foo(j, sg);
    if (m != 86 || ig != -1 || sg != "abab") return 1;

    // method call side effect
    let n: int = new A().meth(m, sg);
    if (n != 82 || ig != 1 || sg != "abababab") return 1;

    return 0;
}
