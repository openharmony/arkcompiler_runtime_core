/*---
desc: Private interface methods.
assert: A default method in an interface body may explicitly be declared private.
---*/
interface Vehicle {
  private getHorsePower3(rpm: int, torque: int): int {
      return (rpm * torque) / 5252;
  }

  getPwrIndex(torque: int): int {
      return this.getHorsePower3(5252, torque);
  }
}

class Car implements Vehicle {}

function main(): void {
    assert new Car().getPwrIndex(1) == 1
}
