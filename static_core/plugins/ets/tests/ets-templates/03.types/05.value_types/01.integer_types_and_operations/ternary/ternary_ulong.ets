{% for v in values %}
/*---
desc: check ternary if-else operation with condition of unsigned long integer operand
---*/

function main(): void {
    const a: ulong = {{v.condition}}
    const b: ulong = {{v.ifTrue}}
    const c: ulong = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
