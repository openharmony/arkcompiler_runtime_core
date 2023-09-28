/*---
desc: Private interface methods.
assert: A default method in an interface body may explicitly be declared private.
tags: [compile-only, negative]
---*/

interface Vehicle {
  private getHorsePower(rpm: int, torque: int): int {
    return (rpm * torque) / 5;
  }
}

class A implements Vehicle {
  public bar(): int {
    return this.getHorsePower(100, 2);
  }
}
