/*---
desc: Switch statement with enum
---*/

enum Color { Red, Yellow, Green }

function f(i: Color): void {
  switch (i) {
    case Color.Red:    assert i == Color.Red;    break;
    case Color.Yellow: assert i == Color.Yellow; break;
    case Color.Green:  assert i == Color.Green;  break;
  }
}

function main(): void {
  f(Color.Red);
  f(Color.Yellow);
  f(Color.Green);
}
