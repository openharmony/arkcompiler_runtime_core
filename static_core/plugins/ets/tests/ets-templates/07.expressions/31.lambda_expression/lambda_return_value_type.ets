/*---
desc: lambda return value type.
---*/

type func = (y: byte) => byte;

function main(): void {
  let bar: func = (x: byte): byte => { return (5 + x) as byte; }
  let value: byte = 3;

  assert bar(value) == 8;
}