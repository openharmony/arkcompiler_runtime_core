/*---
desc: Save function call expression
---*/

class A {
  check(): int {
    return 1;
  }
}

function f(): A|null {
  return new A();
}

function main(): void {
  assert f()?.check() == 1;
}
