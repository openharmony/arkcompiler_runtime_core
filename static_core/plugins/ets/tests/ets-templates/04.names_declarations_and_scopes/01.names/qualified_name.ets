{% for qname in qualified_name %}
/*---
desc: Qualified name {{qname}}
params: {{qname}}
---*/

package test;

class Clazz {
    static staticField: int = 1;
    static staticMethod(): int { return 2; }

    field: int = 3;
    method(): int { return 4; }
}

let instance: Clazz = new Clazz();

function main(): void {
	  assert {{qname}}
}
{% endfor %}
