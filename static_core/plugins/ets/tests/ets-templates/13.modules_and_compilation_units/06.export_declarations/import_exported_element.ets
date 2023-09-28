/*---
desc: A declared name is considered private if not exported; a private declared name can be used only inside the package it is declared in.
---*/

package ets.tests;

import * as Time from "std/time"

function main() {
  let value = new Time.Chrono();
}
