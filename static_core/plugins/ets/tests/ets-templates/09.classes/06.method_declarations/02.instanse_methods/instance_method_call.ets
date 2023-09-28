/*---
desc: Instance method call. 
assert: A method that is not declared static is called an instance method or a non-static method.
---*/

class A {
  public foo(): int {
    return 5;
  }
}

function main(): void {
  let instance: A = new A();
  assert instance.bar() == 10;
}
