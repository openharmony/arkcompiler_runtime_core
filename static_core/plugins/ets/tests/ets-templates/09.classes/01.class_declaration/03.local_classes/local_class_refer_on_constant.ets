{% for t in variable_types %}

/*---
desc: Declaring local class in method body with refer on constant {{t}} variable.
assert: A local class can only be referred to with a simple name (not a qualified or canonical name), i.e., a local class cannot be considered if a canonical name is required.
params: {{t}}
tags: [compile-only]
---*/

class A {
  public static const x: {{t}};

  public static bar(): void {
    inner class B {
      public bar(): void {
        let y: {{t}} = x;
      }
    }
  }
}

{% endfor %}
