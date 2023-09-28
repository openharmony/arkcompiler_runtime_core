
String API
================

Construction
------------

Signature:

* constructor()
* constructor(data: char[])
* constructor(otherStr: string)

.. code-block:: typescript

        let s1 = new string()
        let s2 = new string("42")
        let s3 = new string(s2)

Instance Methods
----------------

``length``

Length of a string.

Signature: **length(): int**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.length())
        // extended 2

``charAt``

Getter for char at some index.

Signature: **charAt(index: int): char**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.charAt())

``codePointAt``

Gets the codepoint at the specified index in this string.
Is similar to charAt(index), but if the character at the
index is the beginning of a surrogate pair, the int
representation for this codepoint is returned. Otherwise,
the character at the index is returned.

Signature: **codePointAt(index: int): int**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.codePointAt(1))

``concat``

Getter for char at some index.

Signature: **concat(to: string): string**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.concat("1"))
        // expected "421"

``endsWith``

Checks that the string ends with the specified suffix.

Signature: **endsWith(suffix: string): boolean**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.endsWith("2"))
        // expected true

``indexOf``

Finds the first occurrence of a character in the string.

Signature: **indexOf(ch: char): int**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.indexOf("2"))
        // expected 1

``lastIndexOf``

Finds the last occurrence of a character in the string.

Signature: **lastIndexOf(ch: char): int**

.. code-block:: typescript

        let s : string = new string("422")
        console.println(s.lastIndexOf("2"))
        // expected 2

``padRight``

Creates a new string of a specified length in which
the end of the string is padded with a specified
character.

Signature: **padRight(pad: char, count: int): string**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.padRight('2', 3))
        // expected 422

``padLeft``

Creates a new string of a specified length in which
the beginning of the string is padded with a
specified character.

Signature: **padLeft(pad: char, count: int): string**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.padLeft('2', 3))
        // expected 242

``repeat``

Repeats the string a certain number of times (count), i.e.

Signature: **repeat(count: int): string**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.repeat(2))
        // expected 4242

``startsWith``

Checks that the string starts with the specified prefix.

Signature: **startsWith(prefix: string): boolean**

.. code-block:: typescript

        let s : string = new string("42")
        console.println(s.startsWith("4"))
        // expected true

``substring``

Selects a substring of the string starting at a specified index
and ending at the end of this string.

Signature:

* substring(begin: int): string
* substring(begin: int, end: int): string

.. code-block:: typescript

        let s : string = new string("423")
        console.println(s.substring(1))
        // expected 23
        console.println(s.substring(1, 2))
        // expected 2

``substr``

Selects a substring of the string starting at a specified index
and ending at the end of this string.

Signature:

* substr(begin: int): string
* substr(begin: int, end: int): string

.. code-block:: typescript

        let s : string = new string("423")
        console.println(s.substr(1))
        // expected 23
        console.println(s.substr(1, 2))
        // expected 2

``raw``

The ``string.raw()`` static method is a tag function of template literals.

.. code-block:: typescript

        const filePath = string.raw("C:\Development\profile\aboutme.html")
        console.println(filePath)
        // expected C:\Development\profile\aboutme.html

``toLowerCase``

Creates a new string similar to the string but with
all characters in lower case.

Signature: **toLowerCase(): string**

.. code-block:: typescript

        let s : string = new string("AaA")
        console.println(s.toLowerCase())
        // expected aaa

``toUpperCase``

Creates a new string similar to the string but with
all characters in upper case.

Signature: **toUpperCase(): string**

.. code-block:: typescript

        let s : string = new string("AaA")
        console.println(s.toUpperCase())
        // expected AAA

``trim``

Trims all whitespaces from the beginning and end of this string.

Signature: **trim(): string**

.. code-block:: typescript

        let s : string = new string("  AaA ")
        console.println(s.trim())
        // expected "AaA"

``trimLeft``

Trims all whitespaces from the beginning of this string.

Signature: **trimLeft(): string**

.. code-block:: typescript

        let s : string = new string("  AaA ")
        console.println(s.trimLeft())
        // expected "AaA "

``trimRight``

Trims all whitespaces from the end of this string.

Signature: **trimRight(): string**

.. code-block:: typescript

        let s : string = new string("  AaA ")
        console.println(s.trimRight())
        // expected "  AaA"

``split``

The ``split()`` method takes a pattern and divides a string into an ordered
list of substrings by searching for the pattern, puts these substrings into
an array, and returns the array.

.. code-block:: typescript

        let s : string = new string("w1 s2 w3 w4")
        let words = str.split(' ')
        console.log(words[3])
        // Expected output: "w4"

``localeCompare``

The ``localeCompare()`` method returns a number indicating whether a
reference a string comes before, after or is the same as the given string
in the sort order.

.. code-block:: typescript

        let a = 'réservé'; // With accents, lowercase
        let b = 'RESERVE'; // No accents, uppercase
        console.log(a.localeCompare(b));
        // Expected output: 1
        console.log(a.localeCompare(b, 'en', { sensitivity: 'base' }));
        // Expected output: 0

``fromCharCode``

The ``string.fromCharCode()`` static method returns a string created from
the specified sequence of UTF-16 code units.

.. code-block:: typescript

        console.log(string.fromCharCode(189, 43, 190, 61));
        // Expected output: "½ + ¾="

``fromCodePoint``

The ``string.fromCodePoint()`` static method returns a string created by
using the specified sequence of code points.

.. code-block:: typescript

        console.log(string.fromCodePoint(9731, 9733, 9842, 0x2F804));
        // Expected output: "☃★♲你"

``at``

The ``at()`` method takes an integer value and returns a new string consisting
of the single UTF-16 code unit located at the specified offset. This method
allows for positive and negative integers. Negative integers count back from
the last string character.

.. code-block:: typescript

        const str = 'asdfg';
        console.log(str.at(3));
        // Expected output: "f"

``charCodeAt``

The ``charCodeAt()`` method returns an integer between 0 and 65535
representing the UTF-16 code unit at the given index.

.. code-block:: typescript

        const str = 'asdfg';
        console.log(str.charCodeAt(3));
        // Expected output: "f"

``includes``

The ``includes()`` method performs a case-sensitive search to determine whether
one string may be found within another string, returning true or false as
appropriate.

.. code-block:: typescript

        const worldString = "Hello, world";
        console.log(worldString.includes("world")); // true

``match``

The ``match()`` method retrieves the result of matching a string against a
regular expression.

.. code-block:: typescript

        const paragraph = 'The quick brown fox jumps over the lazy dog. It barked.';
        const regex = /[A-Z]/g;
        const found = paragraph.match(regex);
        console.log(found);
        // Expected output: Array ["T", "I"]

``matchAll``

The ``matchAll()`` method returns an iterator of all results matching a string
against a regular expression, including capturing groups.

.. code-block:: typescript

        const regexp = /t(e)(st(\d?))/g;
        const str = 'test1test2';

        const array = str.matchAll(regexp);

        console.log(array[0]);
        // Expected output: Array ["test1", "e", "st1", "1"]

``normalize``

The ``normalize()`` method returns the Unicode Normalization Form of the string.

.. code-block:: typescript

        const name1 = '\u0041\u006d\u00e9\u006c\u0069\u0065';
        console.log(name1.normalize('NFC'));
        // Amélie

``replace``

The ``replace()`` method returns a new string with one, some, or all matches
of a pattern replaced by a replacement.

.. code-block:: typescript

        const str = 'w1 w2';
        console.log(name1.replace('w2', w3));
        // expected "w1 w3"

``replaceAll``

The ``replaceAll()`` method returns a new string with all matches of a
pattern replaced by a replacement.

.. code-block:: typescript

        const str = 'w1 w2 w2';
        console.log(name1.replace('w2', w3));
        // expected "w1 w3 w3"

``search``

The ``search()`` method executes a search for a match between a regular
expression and the string object.

.. code-block:: typescript

        const paragraph = 'The quick brown fox jumps over the lazy dog. If the dog barked, was it really lazy?';
        // Any character that is not a word character or whitespace
        const regex = /[^\w\s]/g;
        console.log(paragraph.search(regex));
        // Expected output: 43

``slice``

The ``slice()`` method extracts a section of a string and returns it as a new
string without any modification to the original string.

.. code-block:: typescript

        const worldString = "Hello, world";
        console.log(worldString.slice(1, 3)); // expected "el"

``toLocaleLowerCase``

The ``toLocaleLowerCase()`` method returns the calling string value converted
to lower case according to any locale-specific case mappings.

.. code-block:: typescript

        const text = "Text";
        console.log(text.toLocaleLowerCase('en-US')); // text

``toLocaleUpperCase``

The ``toLocaleUpperCase()`` method returns the calling string value converted
to upper case according to any locale-specific case mappings.

.. code-block:: typescript

        const text = "Text";
        console.log(text.toLocaleUpperCase('en-US')); // TEXT
