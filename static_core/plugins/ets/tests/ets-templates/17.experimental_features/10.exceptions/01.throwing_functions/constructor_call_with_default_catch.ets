/*---
desc: Throwing function call if a call of a throwing function is not enclosed in a try statement;.
assert: Exception propagation occurs in two cases. If a call of a throwing function is not enclosed in a try statement; If the enclosed try statement does not contain a clause that catches the raised exception.
---*/

class A {
  public constructor() throws {
    throw new Exception();
  }
}

function main(): void {
  let i: int = 1;

  try {
    let a: A = new A();
  }
  catch (e) {
    i = 5;
  }

  assert i == 5;
}