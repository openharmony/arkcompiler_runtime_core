/*---
desc: Enum type methods (valueOf)
---*/

enum Color {Red, Green, Blue}

function main(): void throws {
  let c: Color = Color.Red;
  let colors = Color.values();
  let red = Color.valueOf("Red");
  assert red == colors[0];
  assert(c == red);
}
