/*---
desc: Static interface methods
---*/

interface Vehicle {
  static getHorsePower(rpm: int, torque: int): int {
    return (rpm * torque) / 5252;
  }

  private static getHorsePower1(rpm: int, torque: int): int {
    return (rpm * torque) / 5252;
  }

  static private getHorsePower2(rpm: int, torque: int): int {
    return (rpm * torque) / 5252;
  }
}

function main(): void {}
