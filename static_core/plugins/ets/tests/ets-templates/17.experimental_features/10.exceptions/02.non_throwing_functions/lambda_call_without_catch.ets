/*---
desc: Throwing function call if a call of a throwing function is not enclosed in a try statement;.
assert: Exception propagation occurs in two cases. If a call of a throwing function is not enclosed in a try statement; If the enclosed try statement does not contain a clause that catches the raised exception.
tags: [compile-only, negative]
---*/

class TestException extends Exception {}
type func = (x: int) => int;

function main(): void {
  let bar: func = (x: int): int => { throw new TestException(); return 5; };

  let i: int = bar(3);
}
