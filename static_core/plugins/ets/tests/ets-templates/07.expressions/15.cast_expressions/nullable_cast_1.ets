/*---
desc: nullable Cast T | null -> T. null value
---*/

function main(): void {
  let a: Int | null = null;
  let res: int = 0;

  try {
    let b: Int = a as Int;
    res = 0;
  } catch (e: ClassCastException) {
    res = 1;
  }

  assert res == 0;
}
