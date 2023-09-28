{% for qname in qualified_name %}

/*---
desc: Qualified name {{qname}}
params: {{qname}}
tags: [compile-only]
---*/

package test;

class Clazz {
  static staticField: int;
  static staticMethod(): void {}
  field: int;
  method(): void {}
}

let instance: Clazz = new Clazz();

function main(): void {
  {{qname}}
}

{% endfor %}
