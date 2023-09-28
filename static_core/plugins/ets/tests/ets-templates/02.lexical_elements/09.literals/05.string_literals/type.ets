/*---
desc: String literals are of type string, which is a predefined reference type.
---*/

function main() {
    let s: string[] = [ "", '', "\x00", '\u0000', "\u{CAFE}", "\uBABE", '生日快乐!', "С Днём рождения!" ]

    for (let i = 0; i < s.length; i++) {
        assert s[i] instanceof string : s[i] + " is not a string"
    }
}
