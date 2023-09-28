/*---
desc: A declared name is considered private if not exported; a private declared name can be used only inside the package it is declared in.
tags: [compile-only, negative]
---*/

package ets.tests;

import * as Time from "std/time"

function main() {
  let value = Time.getFirstDayInMonth(1, 1);
}
