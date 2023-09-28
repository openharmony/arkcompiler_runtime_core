{% for v in values %}
/*---
desc: check ternary if-else operation with condition of long integer operand
---*/

function main(): void {
    const a: long = {{v.condition}}
    const b: long = {{v.ifTrue}}
    const c: long = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
