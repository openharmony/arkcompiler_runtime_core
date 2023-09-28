/*---
desc: Method call by his name.
assert: The identifier in a classMethodDeclaration is the name of the method whitch may be used to refer to the method.
---*/

class A {
  public run(): int {
    return 5;
  }

  public stop(): int {
    return 0;
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.run() == 5;
}
