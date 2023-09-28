/*---
desc: Interface field declaration
---*/

interface BaseColors {
  RED:   int = 1;
  GREEN: int = 2;
  BLUE:  int = 3;
}

interface ExtendedColors extends BaseColors {
  YELLOW: int = 4;
}

function main(): void {}