{% for v in values %}
/*---
desc: check unsigned right shift operation for long integer
---*/

function main(): void {
    const a: long = {{v.left}}
    const b: int = {{v.right}}
    assert (a >>> b) == {{v.result}}
}

{% endfor %}
