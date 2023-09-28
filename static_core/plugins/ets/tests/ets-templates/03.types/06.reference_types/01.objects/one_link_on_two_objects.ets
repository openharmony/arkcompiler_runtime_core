/*---
desc: If two variables contain references to the same object, the state of the object can be modified using one variable' s reference to the object, and then the altered state can be observed through the reference in the other variable.
---*/

class A {
  x: int;
}

function main(): void {
    let a: A = new A();

    let b = a;
    let c = a;

    a.x = 10;
    assert b.x == 10;
}