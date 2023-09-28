/*---
desc: Static field declaration.
assert: If a field is declared static, there exists exactly one incarnation of the field, no matter how many instances of the class may eventually be created. A static field, sometimes called a class variable, is incarnated when the class is initialized.
---*/

class A {
  public static x: int = 0;

  public getX() : int {
    return A.x;
  }

  public addX() : void {
    A.x = A.x + 1;
  }
}

function main(): void {
  A.x = 0;  // reset due to jit mode warmup

  let i1: A = new A();
  let i2: A = new A();

  assert A.x == 0;

  i2.addX();

  assert i1.getX() == 1;

  A.x = A.x + 5;

  assert i2.getX() == 6;
}
