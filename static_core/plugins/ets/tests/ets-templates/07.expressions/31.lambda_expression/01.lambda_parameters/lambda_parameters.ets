/*---
desc: lambda declaration with some parameters.
---*/

type func = (x: float, y: int) => float;

function main(): void {
  let bar: func = (x: float, y: int): float => { return x + y; }
}