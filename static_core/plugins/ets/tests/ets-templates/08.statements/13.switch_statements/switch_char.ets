/*---
desc: Switch statement with char
---*/

function f(i: char): void {
  switch (i) {
    case c'0':
      assert i == c'0';
      break;
    case c'1':
      assert i == c'1';
      break;
    case c'2':
      assert i == c'2';
      break;
    default:
      assert false;
  }
}

function main(): void {
  f(c'0');
  f(c'1');
  f(c'2');
}
