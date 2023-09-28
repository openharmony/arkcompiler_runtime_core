{% for v in values %}
/*---
desc: check division for long integer operands
---*/

function main(): void {
    const a: long = {{v.left}}
    const b: long = {{v.right}}
    assert (a / b) == {{v.result}}
}

{% endfor %}
