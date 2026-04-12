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

{% yaml "fields_with_late_initialization_in_different_contexts_common.yaml" as common_test_data %}

{% for c in common_test_data.cases %}

/*---
desc: >-
    Field with late initialization f!: T = expr effectively means that
    the type of the field f is T | undefined. However, the field behaves
    like a field of type T with any form of access.
assert: >-
    Checking usage fields with late initialization in different contexts.
    As all other fields, a field with late initialization must be initialized
    before it is used for the first time.
test_suffix: "{{c.test_suffix}}"
---*/

class C {
  f!: {{c.check_type}}
  iterable_f!: {{c.check_iterable_type}}
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
    return this.f
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
  return v.f
}

function check_switch (v: C): string {
  let res: string;
  const scv0: {{c.check_type}} = {{c.switch_check_val_0}}
  const scv1: {{c.check_type}} = {{c.switch_check_val_1}}
  switch (v.f) {
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
  let expectedError = (e: Error): boolean => {
    return (e instanceof Error)
  }

  testSuite.addTest("check_declaration_context_for_var_decl1", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let v = c.f
    }, expectedError)
  });

  testSuite.addTest("check_declaration_context_for_var_decl2", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let v: {{c.check_type}} = c.f
    }, expectedError)
  });

  testSuite.addTest("check_declaration_context_for_const_var_decl1", () => {
    arktest.expectThrow(() => {
      let c = new C()
      const v = c.f
    }, expectedError)
  });

  testSuite.addTest("check_declaration_context_for_const_var_decl2", () => {
    arktest.expectThrow(() => {
      let c = new C()
      const v: {{c.check_type}} = c.f
    }, expectedError)
  });

  testSuite.addTest("check_assignment_context_for_var", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let v: {{c.check_type}};
      v = c.f
    }, expectedError)
  });

  testSuite.addTest("check_assignment_context_for_instance_field", () => {
    arktest.expectThrow(() => {
      let c = new C()
      c.fi = c.f
    }, expectedError)
  });

  testSuite.addTest("check_assignment_context_for_static_field", () => {
    arktest.expectThrow(() => {
      let c = new C()
      C.fs = c.f
    }, expectedError)
  });

  testSuite.addTest("check_call_context_for_func_argument", () => {
    arktest.expectThrow(() => {
      let c = new C()
      check_func_parameter(c.f)
    }, expectedError)
  });

  testSuite.addTest("check_call_context_for_lambda_argument", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let check_lambda_parameter = (v: {{c.check_type}}): {{c.check_type}} => { return v }
      check_lambda_parameter(c.f)
    }, expectedError)
  });

  testSuite.addTest("check_call_context_for_method_argument", () => {
    arktest.expectThrow(() => {
      let c = new C()
      c.check_method_parameter(c.f)
    }, expectedError)
  });

  testSuite.addTest("check_call_context_for_constructor_argument", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let d = new D(c.f)
    }, expectedError)
  });

  testSuite.addTest("check_return_context_for_func_return_value_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      check_func_return_type(c)
    }, expectedError)
  });

  testSuite.addTest("check_return_context_for_lambda_return_value_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let check_lambda_return_type = (v: C): {{c.check_type}} => { return v.f }
      check_lambda_return_type(c)
    }, expectedError)
  });

  testSuite.addTest("check_return_context_for_method_return_value_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      c.check_method_return_type()
    }, expectedError)
  });

  testSuite.addTest("check_composite_literal_context_for_array_literal_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let a: {{c.check_array_type}} = [c.f]
    }, expectedError)
  });

  testSuite.addTest("check_composite_literal_context_for_object_literal_of_class_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let e: E = {f: c.f}
    }, expectedError)
  });

  testSuite.addTest("check_composite_literal_context_for_object_literal_of_interface_type", () => {
    arktest.expectThrow(() => {
      let c = new C()
      let i: I = {f: c.f}
    }, expectedError)
  });

  testSuite.addTest("check_string_operator_contexts", () => {
    let c = new C()
    arktest.expectThrow(() => {
      "_" + c.f;
    }, expectedError)
    arktest.expectThrow(() => {
      c.f + "_";
    }, expectedError)
  });

{% if c.numeric_check_val %}
  testSuite.addTest("check_numeric_operator_contexts", () => {
    let c = new C()
    arktest.expectThrow(() => {
      c.f + {{c.add_val}};
    }, expectedError)
    arktest.expectThrow(() => {
      {{c.add_val}} + c.f;
    }, expectedError)
  });
{%- endif %}

  testSuite.addTest("check_switch_statement", () => {
    arktest.expectThrow(() => {
      let c = new C()
      check_switch(c)
    }, expectedError)
  });

{% if c.for_start_val %}
  testSuite.addTest("check_for_statement", () => {
    let c = new C()
    arktest.expectThrow(() => {
      for (; c.f < {{c.for_end_val}};) {
      }
    }, expectedError)
    arktest.expectThrow(() => {
      for (; true; c.f += {{c.for_add_val}}) {
      }
    }, expectedError)
  });
{%- endif %}

  testSuite.addTest("check_while_statement", () => {
    arktest.expectThrow(() => {
      let c = new C()
      while (c.f) {
      }
    }, expectedError)
  });

  testSuite.addTest("check_do_statement", () => {
    arktest.expectThrow(() => {
      let c = new C()
      do {
      } while (c.f)
    }, expectedError)
  });

  testSuite.addTest("check_for-of_statement1", () => {
    arktest.expectThrow(() => {
      let c = new C()
      for (let element of c.iterable_f) {
      }
    }, expectedError)
  });

  testSuite.addTest("check_for-of_statement2", () => {
    arktest.expectThrow(() => {
      let c = new C()
      for (const element of c.iterable_f) {
      }
    }, expectedError)
  });

{% if c.check_ternary_cond_expr1 %}
  testSuite.addTest("check_ternary_cond_expression1", () => {
    arktest.expectThrow(() => {
      let c = new C()
      c.check_ternary_cond_expression1()
    }, expectedError)
  });
{%- endif %}

{% if c.check_ternary_cond_expr2 %}
  testSuite.addTest("check_ternary_cond_expression2", () => {
    arktest.expectThrow(() => {
      let c = new C()
      c.check_ternary_cond_expression2()
    }, expectedError)
  });
{%- endif %}

  return testSuite.run();
}

{% endfor %}
