{% for mod in modifiers %}

/*---
desc: Class constructor declaration
assert: A constructor is used in the creation of an object that is an instance of a class. A constructor declaration starts with the constructor keyword and does not has a name. In all other respects, a constructor declaration looks just like a method declaration that has no result.
params: {{mod}}
tags: [compile-only]
---*/

class A {
  public x: int;

  {{mod}} constructor() {
    this.x = 5;
  }
}

{% endfor %}
