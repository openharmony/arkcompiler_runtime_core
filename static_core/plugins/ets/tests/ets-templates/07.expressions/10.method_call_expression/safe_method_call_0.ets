/*---
desc: Safe method call instance method of a class
assert: A method call with ‘?.’ in objectReference is called a safe method call because it handles null safely.
---*/

class A {
  public foo(x: int): int { 
    return x * 2;
  }
}

function main(): void {
  let instance: A|null = new A();

  assert instance?.foo(3) == 6;
}
