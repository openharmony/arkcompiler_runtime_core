/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


const en_US: string = intlBestFitLocale("en-US")

function main(): int {
    const suite = new ArkTestsuite("Segmenter");
    suite.addTest("Basic Segmenter creation", testBasicSegmenterCreation);
    suite.addTest("Segmenter options", testSegmenterOptions);
    suite.addTest("Word segmentation", testWordSegmentation);
    suite.addTest("Grapheme segmentation", testGraphemeSegmentation);
    suite.addTest("Sentence segmentation", testSentenceSegmentation);
    suite.addTest("Locale handling", testLocaleHandling);
    suite.addTest("Segment containing", testSegmentContaining);
    return suite.run();
}

function testBasicSegmenterCreation(): void {
    // Test constructor with no arguments
    new Intl.Segmenter();

    // Test constructor with locale array
    const seg2 = new Intl.Segmenter([en_US]);
    const opts = seg2.resolvedOptions();
    assertEQ(opts.locale, en_US);

    // Test constructor with options
    const seg3 = new Intl.Segmenter([] as String[], { granularity: "word" } as Intl.SegmenterOptions);
    const opts3 = seg3.resolvedOptions();
    assertEQ(opts3.granularity, "word");
}

function testSegmenterOptions(): void {
    // Test valid granularity options
    new Intl.Segmenter([] as String[], { granularity: "grapheme" } as Intl.SegmenterOptions);
    new Intl.Segmenter([] as String[], { granularity: "word" } as Intl.SegmenterOptions);
    new Intl.Segmenter([] as String[], { granularity: "sentence" } as Intl.SegmenterOptions);
    // Test resolvedOptions
    const seg = new Intl.Segmenter([en_US], { granularity: "word" } as Intl.SegmenterOptions );
    const opts = seg.resolvedOptions();
    assertTrue(opts.locale instanceof String);
    assertTrue(opts.granularity instanceof String);
}

function testWordSegmentation(): void {
    const seg = new Intl.Segmenter(en_US, { granularity: "word" } as Intl.SegmenterOptions);

    const text = "Hello world!";
    const segments = Array.from(seg.segment(text) as Iterable<Intl.SegmentData>);

    let wordCount = 0;
    let spaceCount = 0;
    let punctCount = 0;

    for (const segment of segments) {
        if (segment.isWordLike == true) {
            wordCount++;
        } else if (segment.segment == " ") {
            spaceCount++;
        } else {
            punctCount++;
        }
    }

    assertEQ(wordCount, 2); // "Hello" and "world"
    assertEQ(spaceCount, 1); // Space between words
    assertEQ(punctCount, 1); // Exclamation mark
}

function testGraphemeSegmentation(): void {
    const seg = new Intl.Segmenter(en_US, { granularity: "grapheme" } as Intl.SegmenterOptions);

    const text = "123";
    const segments = Array.from(seg.segment(text) as Iterable<Intl.SegmentData>);

    let spaceCount = 0;
    let punctCount = 0;
    let wordCount = 0;

    for (const segment of segments) {
        if (segment.isWordLike == true) {
            wordCount++;
        } else if (segment.segment == " ") {
            spaceCount++;
        } else {
            punctCount++;
        }
    }

    assertEQ(segments.length, 3);
    assertEQ(wordCount, 0);
    assertEQ(spaceCount, 0);
    assertEQ(punctCount, 3);
}

function testSentenceSegmentation(): void {
    const seg = new Intl.Segmenter(en_US, { granularity: "sentence" } as Intl.SegmenterOptions);

    const text = "Hello, how are you? I am fine.";
    const segments = Array.from(seg.segment(text) as Iterable<Intl.SegmentData>);
    arktest.assertEQ(segments.length, 2);
}

function testLocaleHandling(): void {
    // Test supportedLocalesOf
    const sBa = Intl.Segmenter.supportedLocalesOf([en_US, "invalid"]);
    let supported = Array.from<String>(sBa);
    arktest.assertTrue(supported.includes(en_US));
    arktest.assertFalse(supported.includes("invalid"));
}

function testSegmentContaining(): void {
    const seg = new Intl.Segmenter(en_US);
    const text = "Hello";
    const segments = Array.from(seg.segment(text) as Iterable<Intl.SegmentData>);
    // Test valid indices
    const first = segments.at(0);
    assertEQ(first!.segment, "H");
    const last = segments.at(4);
    assertEQ(last!.segment!, "o");
}

function verifySupportedLocalesOf(locales: string[]): void {
    const localesArray = Array.from<String>(locales);
    const supported = Array.from<String>(Intl.Segmenter.supportedLocalesOf(locales));
    assertTrue(Array.isArray(supported));
    assertTrue(supported.every((locale: string) => localesArray.includes(locale)));
}
