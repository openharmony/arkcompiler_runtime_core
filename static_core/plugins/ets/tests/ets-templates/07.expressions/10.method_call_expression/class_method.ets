/*---
desc: Call instance method of a class.
assert: A method call expression calls a static or instance method of a class or an interface.
---*/

class A {
  public foo(x: int): int { 
    return x * 2;
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo(3) == 6;
}
