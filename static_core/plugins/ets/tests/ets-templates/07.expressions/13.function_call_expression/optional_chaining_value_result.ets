/*---
desc: Optional chaining declaration.
---*/

class A {
    public x: int = 5;
}

function getX(): Int {
    let a: A = new A();
    return a?.x;
}

function main(): void {
  assert(getX() ==  5);
}
