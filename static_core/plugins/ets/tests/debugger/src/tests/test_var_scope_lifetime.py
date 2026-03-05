#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from arkdb.walker import BreakpointWalkerType


def _ets_code_var_scope_lifetime() -> str:
    return """\
class A {
  public field: int = 1;
}

class C {
  a: A = new A();
  private getA(f: int): Object | undefined {
    return this.a.field == f ? this.a : undefined;
  }

  foo(): void {
    let d1: int = 1;
    console.log(d1);                 // #BP{before_decl}
    {
      let myA = this.getA(d1);
      console.log(d1);               // #BP{inside_block}
      if (myA) {
        console.log((myA as A).field);
      }
    }
    console.log(d1);                 
    let b = new A();                 // #BP{after_block}
    console.log(b.field);            // #BP{after_b}
  }
}

function main(): void {
  let c = new C();
  c.foo();
}\
"""


async def test_debugger_local_var_scope_lifetime(
    breakpoint_walker: BreakpointWalkerType,
):
    code = _ets_code_var_scope_lifetime()
    async with breakpoint_walker(code) as walker:
        paused = await anext(walker)
        assert paused.hit_breakpoint_labels().pop() == "before_decl"
        vars_before_decl = await paused.frame().scope().mirror_variables()
        assert "myA" not in vars_before_decl

        paused = await anext(walker)
        assert paused.hit_breakpoint_labels().pop() == "inside_block"
        vars_inside_block = await paused.frame().scope().mirror_variables()
        assert "myA" in vars_inside_block

        paused = await anext(walker)
        assert paused.hit_breakpoint_labels().pop() == "after_block"
        vars_after_block = await paused.frame().scope().mirror_variables()
        assert "b" not in vars_after_block
        assert "myA" not in vars_after_block

        paused = await anext(walker)
        assert paused.hit_breakpoint_labels().pop() == "after_b"
        vars_after_b = await paused.frame().scope().mirror_variables()
        assert "myA" not in vars_after_b
        assert "b" in vars_after_b
