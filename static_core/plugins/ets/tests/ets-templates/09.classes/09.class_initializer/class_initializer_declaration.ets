/*---
desc: Class initializer declaration.
assert: A class initializer declared in a class is executed when the class is initialized. Together with any field initializers for class variables, class initializers may be used to initialize the class variables of the class.
---*/

class A {
  public static x: int;

  static {
    A.x = 5;
  }
}

function main(): void {
  assert A.x == 5;
}