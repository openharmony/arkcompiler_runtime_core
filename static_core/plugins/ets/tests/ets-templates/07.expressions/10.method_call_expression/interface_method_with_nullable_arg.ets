/*---
desc: Call instance method of a interface with ?? argument.
assert: A compile-time error occurs if typeArguments is present and any of the type arguments are wildcards (§??).
tags: [negative]
---*/

interface A {
  bar(x: int): int {
    return x * 2;
  }
}

class B implements A {}

function main(): void {
  let x: int? = 3;
  let instance: B = new B();

  assert instance.bar(x) == 6;
}