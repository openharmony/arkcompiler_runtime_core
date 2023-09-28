{% for v in values %}
/*---
desc: check multiplication of two bytes
---*/

function main(): void {
    const a: byte = {{v.left}}
    const b: byte = {{v.right}}
    assert (a * b) == {{v.result}}
}

{% endfor %}
