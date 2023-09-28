/*---
desc: Save function call expression
---*/

Class A {
  check(): int {
    return null;
  }
}

function f(): A|null {
  return new A()
}

function main(): void {
  assert f()?.check() == null;
}
