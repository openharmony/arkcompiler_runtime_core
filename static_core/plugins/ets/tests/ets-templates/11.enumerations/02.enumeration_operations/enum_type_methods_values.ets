/*---
desc: Enum type methods (values)
---*/

enum Color {Red, Green, Blue}

function main(): void {
  let colors = Color.values();
  assert colors.length == 3;
}
