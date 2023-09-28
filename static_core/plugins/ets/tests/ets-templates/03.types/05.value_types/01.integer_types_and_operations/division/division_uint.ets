{% for v in values %}
/*---
desc: check division for unsigned integer operands
---*/

function main(): void {
    const a: uint = {{v.left}}
    const b: uint = {{v.right}}
    assert (a / b) == {{v.result}}
}

{% endfor %}
