/*---
desc: Static interface methods should have a body
tags: [negative, compile-only]
---*/

interface Vehicle {
  static getHorsePower(rpm: int, torque: int): int
}

function main(): void {}
