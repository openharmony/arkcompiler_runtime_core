Date API
================

Construction
------------

Signature: 

* constructor() 
* constructor(year: long)
* constructor(year: long, month: long)
* constructor(year: long, month: long, day: long)
* constructor(year: long, month: long, day: long, hours: long)
* constructor(year: long, month: long, day: long, hours: long, minutes: long)
* constructor(year: long, month: long, day: long, hours: long, minutes: long, seconds: long)
* constructor(year: long, month: long, day: long, hours: long, minutes: long, seconds: long, ms: long)
* constructor(year: double)
* constructor(year: double, month: double)
* constructor(year: double, month: double, day: double)
* constructor(year: double, month: double, day: double, hours: double)
* constructor(year: double, month: double, day: double, hours: double, minutes: double)
* constructor(year: double, month: double, day: double, hours: double, minutes: double, seconds: double)
* constructor(year: double, month: double, day: double, hours: double, minutes: double, seconds: double, ms: double)

.. code-block:: typescript

        let d1 = new Date();
        let d2 = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
        

Static Methods
--------------

``now``

The ``now()`` static method returns the number of milliseconds elapsed since
the epoch, which is defined as the midnight at the beginning of January 1,
1970, UTC.

Signature: **now(): long**

.. code-block:: typescript

        console.println(Date.now());

``parse``

The ``Date.parse()`` static method parses a string representation of a date,
and returns the number of milliseconds since January 1, 1970, 00:00:00 UTC. 

.. code-block:: typescript

        const date = Date.parse('01 Jan 1970 00:00:00 GMT');
        console.println(Date.now()); // 0

Instance Methods
----------------

``getDate``

Returns the day of the month for the specified date according to the local time.

Signature: **getDate(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getDate());

``getFullYear``

Returns the year of the specified date according to the local time.

Signature: **getFullYear(): int**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getFullYear());

``getHours``

Returns the hour for the specified date according to the local time.

Signature: **getHours(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getHours());

``getMilliseconds``

Returns the milliseconds in the specified date according to the local time.

Signature: **getMilliseconds(): short**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getMilliseconds());

``getMinutes``

Returns the minutes in the specified date according to the local time. 

Signature: **getMinutes(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getMinutes());

``getMonth``

Returns the month in the specified date according to the local time, as a
zero-based value (where zero indicates the first month of the year).

Signature: **getMonth(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getMonth());

``getSeconds``

Returns the seconds in the specified date according to the local time. 

Signature: **getSeconds(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getSeconds());

``getTime``

Returns the number of milliseconds since the epoch, which is defined as the
midnight at the beginning of January 1, 1970, UTC.

Signature: **getTime(): long**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getTime());

``getDay``

Returns the day in the specified date according to the local time.

Signature: **getDay(): short**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getDay());

``getYear``

Returns the day in the specified date according to the local time.

Signature: **getYear(): short**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getYear());

``getUTCDate``

Returns date according to the local time.

Signature: **getUTCDate(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCDate());

``getUTCDay``

Returns the day of the month for the specified date according to the local time.

Signature: **getUTCDay(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCDay());

``getUTCFullYear``

Returns the year of the specified date according to the local time.

Signature: **getUTCFullYear(): int**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCFullYear());

``getUTCHours``

Returns the hour for the specified date according to the local time.

Signature: **getUTCHours(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCHours());

``getUTCMilliseconds``

Returns the milliseconds in the specified date according to the local time.

Signature: **getUTCMilliseconds(): short**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCMilliseconds());

``getUTCMinutes``

Returns the minutes in the specified date according to the local time. 

Signature: **getUTCMinutes(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCMinutes());

``getUTCMonth``

Returns the month in the specified date according to the local time, as a
zero-based value (where zero indicates the first month of the year).

Signature: **getUTCMonth(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCMonth());

``getUTCSeconds``

Returns the seconds in the specified date according to the local time. 

Signature: **getUTCSeconds(): byte**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCSeconds());

``getUTCTime``

Returns the number of milliseconds since the epoch, which is defined as the
midnight at the beginning of January 1, 1970, UTC.

Signature: **getUTCTime(): long**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.getUTCTime());

``toDateString``

Returns the date portion of a Date object interpreted in the local timezone
in English.

Signature: **toDateString(): string**

.. code-block:: typescript

        let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
        console.println(d.toDateString()); // Sat Oct 27 1979

``toISOString``

Returns a string in simplified extended ISO format (ISO 8601), which is always
24 or 27 characters long (YYYY-MM-DDTHH:mm:ss.sssZ or
±YYYYYY-MM-DDTHH:mm:ss.sssZ, respectively). The timezone is always zero UTC
offset, as denoted by the suffix Z.

Signature: **toISOString(): string**

.. code-block:: typescript

        let today = new Date();
        console.println(today.toISOString()); // Returns 2023-02-05T14:48:00.000Z

``toTimeString``

Returns the date portion of a Date object interpreted in the local timezone in
English.

Signature: **toTimeString(): string**

.. code-block:: typescript

        let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
        console.println(d.toTimeString()); // 13:12:57 GMT

``toString``

Returns a string representing the specified Date object interpreted in the
local timezone.

Signature: **toString(): string**

.. code-block:: typescript

        let d = new Date(1979.0, 9.0, 27.0, 13.0, 12.8, 57.0, 444.1);
        console.println(d.toString()); // Sat Oct 27 1979 13:12:57 GMT

``valueOf``

Returns a string representing the specified Date object interpreted in the
local timezone.

Signature: **valueOf(): long**

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.valueOf());

``setDate``

Sets day part of time.

.. code-block:: typescript

        const date = new Date();
        date.setDate(24)
        console.println(date.getDate(5));

``setDay``

Sets day part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setDay(5));

``setFullYear``

Sets full year part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setFullYear(5));

``setHours``

Sets hours part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setHours(5));

``setMilliseconds``

Sets milliseconds part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setMilliseconds(5));

``setMinutes``

Sets minutes part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setMinutes(5));

``setMonth``

Sets month part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setMonth(5));

``setSeconds``

Sets seconds part of time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setSeconds(5));

``setTime``

Sets seconds part of time.

.. code-block:: typescript

        let date : Date = new Date();
        date.setTime(date.now() + setTime)
        console.println(date.getTime()); // now() + 5 minutes

``setYear``

The ``setYear()`` method sets the year for a specified date according to
local time.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setYear(5));


``setUTCDay``

Sets day part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCDay(5));

``setUTCFullYear``

Sets full year part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCFullYear(5));

``setUTCHours``

Sets hours part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCHours(5));

``setUTCMilliseconds``

Sets milliseconds part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCMilliseconds(5));

``setUTCMinutes``

Sets minutes part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCMinutes(5));

``setUTCMonth``

Sets month part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCMonth(5));

``setUTCSeconds``

Sets seconds part of time UTC.

.. code-block:: typescript

        let date : Date = new Date();
        console.println(date.setUTCSeconds(5));

``toUTCString``

The ``toUTCString()`` method converts a date to a string, interpreting it in
the UTC time zone. toGMTString() is an alias of this method.

.. code-block:: typescript

        const utcDate1 = new Date(Date.UTC(96, 1, 2, 3, 4, 5));
        console.log(utcDate1.toUTCString());
        // Expected output: "Fri, 02 Feb 1996 03:04:05 GMT"

``UTC``

The ``Date.UTC()`` static method accepts parameters similar to the Date
constructor but treats them as UTC. It returns the number of milliseconds since
January 1, 1970, 00:00:00 UTC.

.. code-block:: typescript

        const utcDate1 = new Date(Date.UTC(96, 1, 2, 3, 4, 5));
        console.log(utcDate1.toUTCString());
        // Expected output: "Fri, 02 Feb 1996 03:04:05 GMT"

``getTimezoneOffset``

The ``getTimezoneOffset()`` method returns the difference in minutes between
a date as evaluated in the UTC time zone, and the same date as evaluated in
the local time zone.

.. code-block:: typescript

        const date1 = new Date('August 19, 1975 23:15:30 GMT+05:00');
        console.log(date1.getTimezoneOffset());
        // Expected output: your local timezone offset in minutes

``toLocaleTimeString``

The ``toLocaleTimeString()`` method returns a string with a language-sensitive
representation of the time portion of the date.

.. code-block:: typescript

        const event = new Date('August 19, 1975 23:15:30 GMT+00:00');
        console.log(event.toLocaleTimeString('en-US'));
        // Expected output: "1:15:30 AM"

``toLocaleString``

The ``toLocaleString()`` method returns a string with a language-sensitive
representation of this date. 

.. code-block:: typescript

        const event = new Date(Date.UTC(2012, 11, 20, 3, 0, 0));
        console.log(event.toLocaleString('en-GB', { timeZone: 'UTC' }));
        // Expected output: "20/12/2012, 03:00:00"

``setTimeZoneOffset``

Sets the time zone of the system.

.. code-block:: typescript

        const date = new Date();
        date.setTimeZoneOffset(24);

``toJSON``

The ``toJSON()`` method returns a string representation of the Date object.

.. code-block:: typescript

        const date = new Date('August 19, 1975 23:15:30 UTC');
        const jsonDate = date.toJSON();
        console.log(jsonDate);
        // Expected output: "1975-08-19T23:15:30.000Z"

