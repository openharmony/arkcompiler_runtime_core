/*---
desc: import multiple simple names with as
---*/

import {sin as Sine, PI as PNumber} from "std/math";

function main(): void {
    let x = Sine(PNumber);
}
