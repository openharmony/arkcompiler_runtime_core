/*---
desc: Default interface method declaration.
assert: A default method of an interface does have a body represented by a block of code, which provides a default implementation for any class that implements the interface without overriding the method.
---*/

interface Vehicle {
  getHorsePower(rpm: int, torque: int): int {
    return (rpm * torque) / 5;
  }
}

class A implements Vehicle {
  // default interface method overridden
  public getHorsePower(rpm: int, torque: int): int {
    return rpm * torque;
  }
}

function main(): void {
  let a: A = new A();

  assert a.getHorsePower(5, 5) == 25;
}
