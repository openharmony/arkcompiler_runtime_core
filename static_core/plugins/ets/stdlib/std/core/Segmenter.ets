/*
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


package std.core;


export namespace Intl {

type SegmenterGranularity = 'grapheme' | 'word' | 'sentence';


// used in Intl native methods
class Cluster {
    /** The text content of the cluster */
    cluster: string;

    /** Indicates whether the cluster is word-like */
    isWordLike: boolean;

    /** Starts from index */
    index: int;

    /**
     * Default constructor initializing with empty values
     */
    public constructor() {
        this.cluster = "";
        this.isWordLike = false;
        this.index = 0;
    }
}

/**
 * Interface defining the structure of a text segment
 */
export interface SegmentData {
    /** The index of the segment in the original text */
    index: number;

    /** The original input text */
    input: string;

    /** Indicates if the segment is word-like (optional) */
    isWordLike: boolean | undefined;

    /** The text content of the segment */
    segment: string;
}

/**
 * Implementation of SegmentData with private fields and pub getters/setters
 */
class SegmentDataImpl implements SegmentData {
    // Private backing fields
    private _index: number = 0;
    private _input: string = "";
    private _isWordLike?: boolean;
    private _segment: string = "";

    /**
     * Constructor with optional parameters
     * @param index Segment index
     * @param input Original input text
     * @param segment Segment text
     * @param isWordLike Word-like status
     */
    constructor(index?: number, input?: string, segment?: string, isWordLike?: boolean) {
        this._index = index ?? 0;
        this._input = input ?? "";
        this._segment = segment ?? "";
        this._isWordLike = isWordLike;
    }

    // Getters and setters for each property
    public set index(i: number) { this._index = i; }
    public get index(): number { return this._index; }

    public set input(inp: string) { this._input = inp; }
    public get input(): string { return this._input; }

    public set isWordLike(iwl: boolean | undefined) { this._isWordLike = iwl; }
    public get isWordLike(): boolean | undefined { return this._isWordLike; }

    public set segment(seg: string) { this._segment = seg; }
    public get segment(): string { return this._segment; }
}


/**
 * Converts an Cluster to a SegmentDataImpl
 * @param clusterData The cluster to convert
 * @param granularity The segmentation granularity
 * @param index The segment index
 * @param input The original input text
 * @returns A SegmentDataImpl instance
 */
function TryToSegmentData(
    clusterData: Cluster,
    granularity: SegmenterGranularity,
    input: string
): SegmentDataImpl
{
    return new SegmentDataImpl(
        clusterData.index,
        input,
        clusterData.cluster,
        granularity == "word"
                    ? clusterData.isWordLike
                    : undefined
    )
}

/**
 * Options for text segmentation
 */
export interface SegmenterOptions {
    /** Locale matching strategy */
    localeMatcher?: 'best fit' | 'lookup';
    /** Type of segmentation to perform */
    granularity?: SegmenterGranularity;
}

interface PickLocaleMatchSegmenterOptions {
    localeMatcher?: 'best fit' | 'lookup'
}

function pickLocaleMatchSegmenterOptions(
    options: SegmenterOptions
): PickLocaleMatchSegmenterOptions {
    return {
        localeMatcher: options.localeMatcher
    } as PickLocaleMatchSegmenterOptions
}

/**
 * Fully resolved segmentation options
 */
export interface ResolvedSegmenterOptions {
    locale: string;
    granularity: SegmenterGranularity;
}

/**
 * Segments iterator class
 */
export class Segments implements IterableIterator<SegmentData> {
    /** Base parent segments array */
    private parent: SegmentDataImpl[]
    private idx: int = 0

    constructor(parent: SegmentDataImpl[]) {
        this.parent = parent
    }

    public override $_iterator(): IterableIterator<SegmentData>
    {
        return this
    }

    override next(): IteratorResult<SegmentData>
    {
        if (this.idx < 0 || this.idx >= this.parent.length.toInt()) {
            return new IteratorResult<SegmentData>()
        }
        return new IteratorResult<SegmentData>(false, this.parent[this.idx++])
    }
}


/**
 * A specialized class for text segmentation across different linguistic granularities
 */
export class Segmenter {
    /**
     * Native static methods for cluster generation across different segmentation types
     */

    private native static graphemeClusters(str: string, locale: string): Cluster[]
    private native static wordClusters(str: string, locale: string): Cluster[]
    private native static sentenceClusters(str: string, locale: string): Cluster[]

    private _locale: string;
    private _granularity: SegmenterGranularity;

    /**
     * Creates a new Segmenter instance with specified locales and options
     *
     * @param locales Optional locale or array of locales for segmentation
     * @param options Optional segmentation configuration
     *
     * @remarks
     * - Supports single or multiple locales
     * - Automatically resolves the best-fit locale
     * - Defaults to grapheme granularity if not specified
     */
    public constructor(
        locales?: BCP47LanguageTag | BCP47LanguageTag[],
        options?: SegmenterOptions
    ) {
        const picked = {localeMatcher: options?.localeMatcher}
            as PickLocaleMatchSegmenterOptions;
        const supported = Segmenter.supportedLocalesOf(locales ?? "", picked);
        this._locale = supported.length > 0 ? supported[0] : Locale.defaultTag();
        this._granularity = options?.granularity ?? 'grapheme';
    }

    /**
     * Segments a document into clusters based on the configured granularity
     *
     * @param doc The input text to segment
     * @returns A Segments iterator containing segmented data
     */
    public segment(doc: string): Segments {
        console.log("Segmentize", doc);
        console.log("Granularity", this._granularity);
        console.log("Locale", this._locale);

        let result = this.segmentWith(doc, this._granularity)
        let segments = new SegmentDataImpl[result.length];

        for (let i = 0; i < result.length; ++i) {
            segments[i] = TryToSegmentData(result[i], this._granularity, doc);
        }

        return new Segments(segments);
    }

    /**
     * Internal method to perform segmentation based on granularity
     *
     * @param doc The input text to segment
     * @param granularity The type of segmentation to perform
     * @returns An array of Cluster representing the segments
     *
     * @privateRemarks
     * Defaults to grapheme segmentation if an unknown granularity is provided
     */
    private segmentWith(doc: string, granularity: 'grapheme' | 'word' | 'sentence'): Cluster[] {
        switch (granularity) {
        case "word":
            return Array.from<Cluster>(Segmenter.wordClusters(doc, this._locale))
        case "sentence":
            return Array.from<Cluster>(Segmenter.sentenceClusters(doc, this._locale))
        case "grapheme":
        default:
            return Array.from<Cluster>(Segmenter.graphemeClusters(doc, this._locale))
        }
    }

    /**
     * Retrieves the resolved options for the current Segmenter instance
     *
     * @returns The fully resolved segmentation options
     */
    public resolvedOptions(): ResolvedSegmenterOptions {
        return {
            locale: this._locale,
            granularity: this._granularity
        }
    }

    /**
     * Returns an array of supported locales for text segmentation
     *
     * @param locales An array of BCP 47 language tags to check
     * @param options Optional configuration for locale matching
     * @returns An array of supported locales from the input
     *
     * @remarks
     * - Validates which of the provided locales are supported by the Segmenter
     * - Uses the specified locale matching strategy
     * - Helps developers determine locale compatibility before creating a Segmenter instance
     */
    public static supportedLocalesOf(
        locales: BCP47LanguageTag | BCP47LanguageTag[],
        options?: PickLocaleMatchSegmenterOptions,
    ): BCP47LanguageTag[]
    {
        const matchingStrategy = options?.localeMatcher ?? 'best fit';
        try {
            return [
                matchingStrategy == 'lookup'
                ? intlLookUpLocale(locales)
                : intlBestFitLocale(locales)
            ]
        }
        catch(e) {
            if (e instanceof Error) throw e
            throw new RangeError(`Incorrect locales ${locales}`)
        }
    }

}

}
