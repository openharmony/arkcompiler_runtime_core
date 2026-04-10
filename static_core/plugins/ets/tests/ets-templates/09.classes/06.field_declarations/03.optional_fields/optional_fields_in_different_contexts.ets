/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for c in cases %}

/*---
desc: >-
    Checking usage optional fields in different contexts
test_suffix: "{{c.test_suffix}}"
---*/

class C {
  f?: {{c.check_type}} = {{c.decl_val}}
  iterable_f?: {{c.check_iterable_type}} = {{c.iterable_set_val}}
  fi: {{c.check_type}} = {{c.decl_val}}
  static fs: {{c.check_type}} = {{c.decl_val}}

{%- if c.check_ternary_cond_expr1 %}
  check_ternary_cond_expression1(): {{c.check_type}} {
    let res: {{c.check_type}} = {{c.check_ternary_cond_expr1}} ?
      {{c.check_ternary_cond_expr1_ret_pos}} : {{c.check_ternary_cond_expr1_ret_neg}};
    return res
  }
{%- endif %}

{%- if c.check_ternary_cond_expr2 %}
  check_ternary_cond_expression2(): {{c.check_type}} {
    let res: {{c.check_type}} = {{c.check_ternary_cond_expr2}} ?
      {{c.check_ternary_cond_expr2_ret_pos}} : {{c.check_ternary_cond_expr2_ret_neg}};
    return res
  }
{%- endif %}

  check_method_parameter(v: {{c.check_type}}): {{c.check_type}} {
    return v
  }

  check_method_return_type(): {{c.check_type}} {
    return this.f!
  }
}

class D {
  f: {{c.check_type}}

  constructor(v: {{c.check_type}}) {
    this.f = v
  }
}

class E {
  f: {{c.check_type}} = {{c.decl_val}}
}

interface I {
  f: {{c.check_type}}
}

function check_func_parameter(v: {{c.check_type}}): {{c.check_type}} {
  return v
}

function check_func_return_type(v: C): {{c.check_type}} {
  return v.f!
}

function check_switch (v: C): string {
  let res: string;
  const scv0: {{c.check_type}} = {{c.switch_check_val_0}}
  const scv1: {{c.check_type}} = {{c.switch_check_val_1}}
  switch (v.f!) {
    case scv0:
      res = "0"
      break
    case scv1:
      res = "1"
      break
    default:
      res = "other"
  }
  return res
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("checks_in_different_contexts");

  testSuite.addTest("check_declaration_context_for_var_decl1", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let v = c.f!
    let fl = (v: {{c.check_type}}) : {{c.check_type}} => { return v }
    arktest.assertTrue(v instanceof {{c.check_type}})
    arktest.assertEQ(v, {{c.check_val}})
    arktest.assertEQ(fl(v), {{c.check_val}})
  });

  testSuite.addTest("check_declaration_context_for_var_decl2", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let v: {{c.check_type}} = c.f!
    let fl = (v: {{c.check_type}}) : {{c.check_type}} => { return v }
    arktest.assertTrue(v instanceof {{c.check_type}})
    arktest.assertEQ(v, {{c.check_val}})
    arktest.assertEQ(fl(v), {{c.check_val}})
  });

  testSuite.addTest("check_declaration_context_for_const_var_decl1", () => {
    let c = new C()
    c.f = {{c.check_val}}
    const v = c.f!
    let fl = (v: {{c.check_type}}) : {{c.check_type}} => { return v }
    arktest.assertTrue(v instanceof {{c.check_type}})
    arktest.assertEQ(v, {{c.check_val}})
    arktest.assertEQ(fl(v), {{c.check_val}})
  });

  testSuite.addTest("check_declaration_context_for_const_var_decl2", () => {
    let c = new C()
    c.f = {{c.check_val}}
    const v: {{c.check_type}} = c.f!
    let fl = (v: {{c.check_type}}) : {{c.check_type}} => { return v }
    arktest.assertTrue(v instanceof {{c.check_type}})
    arktest.assertEQ(v, {{c.check_val}})
    arktest.assertEQ(fl(v), {{c.check_val}})
  });

  testSuite.addTest("check_assignment_context_for_var", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let v: {{c.check_type}};
    v = c.f!
    arktest.assertTrue(v instanceof {{c.check_type}})
    arktest.assertEQ(v, {{c.check_val}})
  });

  testSuite.addTest("check_assignment_context_for_instance_field", () => {
    let c = new C()
    c.f = {{c.check_val}}
    c.fi = c.f!
    arktest.assertTrue(c.fi instanceof {{c.check_type}})
    arktest.assertEQ(c.fi, {{c.check_val}})
  });

  testSuite.addTest("check_assignment_context_for_static_field", () => {
    let c = new C()
    c.f = {{c.check_val}}
    C.fs = c.f!
    arktest.assertTrue(C.fs instanceof {{c.check_type}})
    arktest.assertEQ(C.fs, {{c.check_val}})
    C.fs = {{c.decl_val}}
  });

  testSuite.addTest("check_call_context_for_func_argument", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ(check_func_parameter(c.f!), {{c.check_val}})
  });

  testSuite.addTest("check_call_context_for_lambda_argument", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let check_lambda_parameter = (v: {{c.check_type}}): {{c.check_type}} => { return v }
    arktest.assertEQ(check_lambda_parameter(c.f!), {{c.check_val}})
  });

  testSuite.addTest("check_call_context_for_method_argument", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ(c.check_method_parameter(c.f!), {{c.check_val}})
  });

  testSuite.addTest("check_call_context_for_constructor_argument", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let d = new D(c.f!)
    arktest.assertEQ(d.f, {{c.check_val}})
  });

  testSuite.addTest("check_return_context_for_func_return_value_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ(check_func_return_type(c), {{c.check_val}})
  });

  testSuite.addTest("check_return_context_for_lambda_return_value_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let check_lambda_return_type = (v: C): {{c.check_type}} => { return v.f! }
    arktest.assertEQ(check_lambda_return_type(c), {{c.check_val}})
  });

  testSuite.addTest("check_return_context_for_method_return_value_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ(c.check_method_return_type(), {{c.check_val}})
  });

  testSuite.addTest("check_composite_literal_context_for_array_literal_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let a: {{c.check_array_type}} = [c.f!, {{c.decl_val}}]
    arktest.assertEQ(a.length, 2)
    arktest.assertEQ(a[0], {{c.check_val}})
    arktest.assertEQ(a[1], {{c.decl_val}})
  });

  testSuite.addTest("check_composite_literal_context_for_object_literal_of_class_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let e: E = {f: c.f!}
    arktest.assertEQ(e.f, {{c.check_val}})
  });

  testSuite.addTest("check_composite_literal_context_for_object_literal_of_interface_type", () => {
    let c = new C()
    c.f = {{c.check_val}}
    let i: I = {f: c.f!}
    arktest.assertEQ(i.f, {{c.check_val}})
  });

  testSuite.addTest("check_string_operator_contexts", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ("_" + c.f, "_{{c.string_check_val}}")
    arktest.assertEQ(c.f + "_", "{{c.string_check_val}}_")
  });

{% if c.numeric_check_val %}
  testSuite.addTest("check_numeric_operator_contexts", () => {
    let c = new C()
    c.f = {{c.check_val}}
    arktest.assertEQ(c.f, {{c.numeric_check_val}})
    arktest.assertEQ(c.f! + {{c.add_val}}, {{c.numeric_check_val}} + {{c.add_val}})
    arktest.assertEQ({{c.add_val}} + c.f!, {{c.numeric_check_val}} + {{c.add_val}})
  });
{%- endif %}

  testSuite.addTest("check_switch_statement", () => {
    let c = new C()
    c.f = {{c.switch_check_val_0}}
    arktest.assertEQ(check_switch(c), "0")
    c.f = {{c.switch_check_val_1}}
    arktest.assertEQ(check_switch(c), "1")
    c.f = {{c.switch_check_val_other}}
    arktest.assertEQ(check_switch(c), "other")
  });

{% if c.for_start_val %}
  testSuite.addTest("check_for_statement", () => {
    let c = new C()
    for (c.f = {{c.for_start_val}}; c.f! < {{c.for_end_val}}; c.f = (c.f! + {{c.for_add_val}}){{c.type_conversion_expr}}) {
    }
    arktest.assertEQ(c.f, {{c.for_end_val}})
  });
{%- endif %}

  testSuite.addTest("check_while_statement", () => {
    let c = new C()
    c.f = {{c.while_true_val}}
    let iter_id: int = 0
    while (c.f) {
      c.f = {{c.while_false_val}}
      iter_id++
    }
    arktest.assertEQ(iter_id, 1)
    arktest.assertEQ(c.f, {{c.while_false_val}})
  });

  testSuite.addTest("check_do_statement", () => {
    let c = new C()
    c.f = {{c.while_true_val}}
    let iter_id: int = 0
    do {
      if (iter_id == 1) {
        c.f = {{c.while_false_val}}
      }
      iter_id++
    } while (c.f)

    arktest.assertEQ(iter_id, 2)
    arktest.assertEQ(c.f, {{c.while_false_val}})
  });

  testSuite.addTest("check_for-of_statement1", () => {
    let c = new C()
    let iter_id: int = 0
    let checked_0: boolean = false
    let checked_1: boolean = false
    let checked_2: boolean = false
    for (let element of c.iterable_f!) {
      switch (iter_id) {
        case 0:
          arktest.assertEQ(element, {{c.iterable_check_val_0}})
          checked_0 = true
          break
        case 1:
          arktest.assertEQ(element, {{c.iterable_check_val_1}})
          checked_1 = true
          break
        case 2:
          arktest.assertEQ(element, {{c.iterable_check_val_2}})
          checked_2 = true
          break
      }
      iter_id++
    }
    arktest.assertTrue(checked_0)
    arktest.assertTrue(checked_1)
    arktest.assertTrue(checked_2)
  });

  testSuite.addTest("check_for-of_statement2", () => {
    let c = new C()
    c.iterable_f = {{c.iterable_set_val}}
    let iter_id: int = 0
    let checked_0: boolean = false
    let checked_1: boolean = false
    let checked_2: boolean = false
    for (const element of c.iterable_f!) {
      switch (iter_id) {
        case 0:
          arktest.assertEQ(element, {{c.iterable_check_val_0}})
          checked_0 = true
          break
        case 1:
          arktest.assertEQ(element, {{c.iterable_check_val_1}})
          checked_1 = true
          break
        case 2:
          arktest.assertEQ(element, {{c.iterable_check_val_2}})
          checked_2 = true
          break
      }
      iter_id++
    }
    arktest.assertTrue(checked_0)
    arktest.assertTrue(checked_1)
    arktest.assertTrue(checked_2)
  });

{% if c.check_ternary_cond_expr1 %}
  testSuite.addTest("check_ternary_cond_expression1", () => {
    let c = new C()
    c.f = {{c.check_ternary_cond_expr1_set_pos_val}}
    arktest.assertTrue(c.f instanceof {{c.check_type}})
    arktest.assertEQ(c.check_ternary_cond_expression1(), {{c.check_ternary_cond_expr1_ret_pos}})
    c.f = {{c.check_ternary_cond_expr1_set_neg_val}}
    arktest.assertTrue(c.f instanceof {{c.check_type}})
    arktest.assertEQ(c.check_ternary_cond_expression1(), {{c.check_ternary_cond_expr1_ret_neg}})
  });
{%- endif %}

{% if c.check_ternary_cond_expr2 %}
  testSuite.addTest("check_ternary_cond_expression2", () => {
    let c = new C()
    c.f = {{c.check_ternary_cond_expr2_set_pos_val}}
    arktest.assertTrue(c.f instanceof {{c.check_type}})
    arktest.assertEQ(c.check_ternary_cond_expression2(), {{c.check_ternary_cond_expr2_ret_pos}})
    c.f = {{c.check_ternary_cond_expr2_set_neg_val}}
    arktest.assertTrue(c.f instanceof {{c.check_type}})
    arktest.assertEQ(c.check_ternary_cond_expression2(), {{c.check_ternary_cond_expr2_ret_neg}})
  });
{%- endif %}

  return testSuite.run();
}

{% endfor %}
