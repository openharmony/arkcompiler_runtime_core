/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/**
 * The maximum depth of printing nested objects
 * Useful for cyclic linked objects
 */
const MAX_CONSOLE_PRINT_DEPTH: int = 10

/**
 * A Console class that provides access to standard output and error streams.
 * Supports printing various data types, timing operations, and indentation management.
 * @export
 * @final
 */
export final class Console {

    /**
     * @section Internal fields
     */


    /**
     * Map to store named timers for performance measurements
     * @private
     */
    private timers: Map<string, long> = new Map<string, long>();

    /**
     * Current indentation level for formatted output
     * @private
     */
    private indentationLevel: int = 0;

    /**
     * Number of spaces used for each level of indentation
     * @private
     * @static
     * @readonly
     */
    private static readonly INDENT_SIZE: int = 2;

    /**
     * Represents the space creating the margin for indentation 
     * @private
     * @static
     * @readonly
     */
    private static readonly INDENT_MARGIN: string = " ".repeat(Console.INDENT_SIZE)

    /**
     * Map to store named counters for counting operations
     * @private
     */
    private counters: Map<string, int> = new Map<string, int>();

    /**
     * Internal constructor for Console class
     * @internal
     */
    internal constructor() {}


    /**
     * @section Native functions 
     */


    /**
     * Prints a nullable value to standard output
     * Converts the value to a string representation with depth limitation
     * @param {NullishType} o - The value to print
     * @returns {void}
     * @public
     */
    public print(o: NullishType): void {
        this.print(Value.of(o).toPrint(MAX_CONSOLE_PRINT_DEPTH))
    }

    /**
     * Prints a nullable value to standard error
     * Converts the value to a string representation with depth limitation
     * @param {NullishType} o - The value to print to stderr
     * @returns {void}
     * @public
     */
    public printError(o: NullishType): void {
        this.printError(Value.of(o).toPrint(MAX_CONSOLE_PRINT_DEPTH))
    }

    /**
     * Native implementations for printing primitive types to stdout
     * @param {string | boolean | byte | short | char | int | long} i - The value to print
     * @returns {void}
     * @public
     * @native
     */
    public native print(i: String): void;
    public native print(i: boolean): void;
    public native print(i: byte): void;
    public native print(i: short): void;
    public native print(i: char): void;
    public native print(i: int): void;
    public native print(i: long): void;

    /**
     * Native implementations for printing primitive types to stderr
     * @param {string | boolean | byte | short | char | int | long} i - The value to print to stderr
     * @returns {void}
     * @public
     * @native
     */
    public native printError(i: String): void;
    public native printError(i: boolean): void;
    public native printError(i: byte): void;
    public native printError(i: short): void;
    public native printError(i: char): void;
    public native printError(i: int): void;
    public native printError(i: long): void;


    /**
     * @section Float/Double handling 
     */


    /**
     * Prints a float value to stdout
     * Converts the float to a string before printing
     * @param {float} f - The float value to print
     * @returns {void}
     * @public
     */
    public print(f: float): void {
        this.print(Float.toString(f));
    }

    /**
     * Prints a double value to stdout
     * Converts the double to a string before printing
     * @param {double} d - The double value to print
     * @returns {void}
     * @public
     */
    public print(d: double): void {
        this.print(Double.toString(d));
    }

    /**
     * Prints a float value to stderr
     * Converts the float to a string before printing
     * @param {float} f - The float value to print to stderr
     * @returns {void}
     * @public
     */
    public printError(f: float): void {
        this.printError(Float.toString(f));
    }

    /**
     * Prints a double value to stderr
     * Converts the double to a string before printing
     * @param {double} d - The double value to print to stderr
     * @returns {void}
     * @public
     */
    public printError(d: double): void {
        this.printError(Double.toString(d));
    }


    /**
     * @section Println definitions (to stdout)
     */


    /**
     * Prints a newline to stdout
     * @returns {void}
     * @public
     * @native
     */
    public native println(): void;

    /**
     * Prints a newline to stderr
     * @returns {void}
     * @public
     * @native
     */
    public native printlnError(): void;

    /**
     * Prints an object followed by a newline to stdout
     * @param {Object | string | boolean | byte | short | char | int | long | float | double} i - The value to print
     * @returns {void}
     * @public
     */
    public println(i: Object): void {
        this.print(i);
        this.println();
    }
    public println(i: String): void {
        this.print(i);
        this.println();
    }
    public println(i: boolean): void {
        this.print(i);
        this.println();
    }
    public println(i: byte): void {
        this.print(i);
        this.println();
    }
    public println(i: short): void {
        this.print(i);
        this.println();
    }
    public println(i: char): void {
        this.print(i);
        this.println();
    }
    public println(i: int): void {
        this.print(i);
        this.println();
    }
    public println(i: long): void {
        this.print(i);
        this.println();
    }
    public println(i: float): void {
        this.print(i);
        this.println();
    }
    public println(i: double): void {
        this.print(i);
        this.println();
    }


    /**
     * @section Println definitions (to stderr)
     */


    /**
     * Prints an object followed by a newline to stderr
     * @param {Object | string | boolean | byte | short | char | int | long | float | double}  i - The value to print
     * @returns {void}
     * @public
     */
    public printlnError(i: Object): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: String): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: boolean): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: byte): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: short): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: char): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: int): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: long): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: float): void {
        this.printError(i);
        this.printlnError();
    }
    public printlnError(i: double): void {
        this.printError(i);
        this.printlnError();
    }


    /**
     * @section Console.log API
     */


    /**
     * Implementations for log primitive types to stdout
     * @param {string | boolean | byte | short | char | int | long} i - The value to print to stdout
     * @returns {void}
     * @public
     * @native
     */
    public log(i: String): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: boolean): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: byte): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: short): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: char): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: int): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: long): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: float): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }
    public log(i: double): void {
        this.printIndentation()
        this.print(i);
        this.println();
    }

    /**
     * @section Console.warn API
     */

    /**
     * Implementations for warn primitive types to stderr
     * @param {string | boolean | byte | short | char | int | long} i - The value to print to stderr
     * @returns {void}
     * @public
     * @native
     */
    public warn(i: String): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: boolean): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: byte): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: short): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: char): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: int): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: long): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: float): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }
    public warn(i: double): void {
        this.printIndentationError()
        this.printError(i);
        this.printlnError();
    }

    /**
     * @section Variadics
     */

    /**
     * Prints multiple values to stdout with space separation and a trailing newline
     * Values are printed with proper indentation
     * @param {...NullishType[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public log(...vals: NullishType[]): void {
        const len = vals.length
        const lastIndex = len - 1
        this.printIndentation()
        for (let i = 0; i < lastIndex; i++) {
            this.print(vals[i])
            this.print(" ")
        }
        if (len != 0) {
            this.print(vals[lastIndex])
        }
        this.println()
    }

    /**
     * Alias for log() method
     * Prints debug-level messages to stdout
     * @param {...NullishType[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public debug(...vals: NullishType[]): void {
        this.log(...vals)
    }

    /**
     * Alias for log() method
     * Prints info-level messages to stdout
     * @param {...NullishType[]} vals - Variable number of values to be logged
     * @returns {void}
     * @public
     */
    public info(...vals: NullishType[]): void {
        this.log(...vals)
    }

    /**
     * Prints multiple values to stderr with space separation and a trailing newline
     * Values are printed with proper indentation
     * @param {...NullishType[]} vals - Variable number of values to be logged as warnings
     * @returns {void}
     * @public
     */
    public warn(...vals: NullishType[]): void {
        const len = vals.length
        const lastIndex = len - 1
        this.printIndentationError()
        for (let i = 0; i < lastIndex; i++) {
            this.printError(vals[i])
            this.printError(" ")
        }
        if (len != 0) {
            this.printError(vals[lastIndex])
        }
        this.printlnError()
    }

    /**
     * Alias for warn() method
     * Prints error-level messages to stderr
     * @param {...NullishType[]} vals - Variable number of values to be logged as errors
     * @returns {void}
     * @public
     */
    public error(...vals: NullishType[]): void {
        this.warn(...vals)
    }

    /**
     * Conditionally prints an error message if the assertion condition is false
     * @param {boolean} condition - The condition to evaluate
     * @param {...NullishType[]} vals - Values to be logged if condition is false
     * @returns {void}
     * @public
     */
    public assert_(condition: boolean, ...vals: NullishType[]): void {
        if (!condition) {
            this.error(...vals)
        }
    }


    /**
     * @section Console.count* API
     */
    

    /**
    * Counts the number of times this method has been called with a specific label
    * Prints the current count to stdout
    * @param {string} [label='default'] - The label to identify this counter
    * @returns {void}
    */
    public count(label?: string): void {
        const key = label ?? 'default'
        const current = this.counters.get(key) ?? 0
        this.counters.set(key, current + 1)
        this.log(`${key}: ${current + 1}`)
    }

    /**
    * Resets the counter for a specific label
    * @param {string} [label='default'] - The label of the counter to reset
    * @returns {void}
    */
    public countReset(label?: string): void {
        const key = label ?? 'default'
        this.counters.delete(key)
    }


    /**
     * @section Console.dir* API
     */


    /**
    * Prints a formatted representation of an object to stdout
    * Filters out properties containing 'field#' in their keys
    * @param {NullishType} obj - The object to inspect
    * @returns {void}
    */
    public dir(obj: NullishType): void {
        if (obj == null) {
            this.log('null')
            return
        }

        
        // NOTE (templin.konstantin): Internal Fields Filtering
        // 
        //  In the current language implementation, objects have an internal representation
        //  where some fields are duplicated with a "field#" prefix. These fields are part
        //  of the internal implementation and should not be displayed when outputting objects.
        // 
        //  Example of internal representation:
        //  {
        //     "name": "test",         
        //     "field#n": "test",    // Internal field
        //     "age": 25,               
        //     "field#m": 25         // Internal field
        //  }
        const filterer = (key: String, value: NullishType): NullishType => {
            if (key.includes("field#")) {
                return undefined;
            }
            return value;
        }

        this.log(JSON.stringify(obj, filterer))
    }

    /**
    * Prints an XML representation of an object to stdout
    * Currently outputs the object as-is
    * @param {NullishType} obj - The object to display as XML
    * @returns {void}
    */
    public dirxml(obj: NullishType): void {
        console.log(obj);
    }


    /**
     * @section Console.group* API
     */


    /**
    * Starts a new logging group with optional label
    * Increases indentation level for subsequent log messages
    * @param {string} [label] - Optional label for the group
    * @returns {void}
    */
    public group(label?: string): void {
        if (label) {
            this.printIndentation()
            this.log(label)
        }
        this.indentationLevel++
    }

    /**
    * Ends the current logging group
    * Decreases indentation level for subsequent log messages
    * @returns {void}
    */
    public groupEnd(): void {
        if (this.indentationLevel > 0) {
            this.indentationLevel--
        }
    }

    /**
    * Alias for group() method
    * Creates a collapsed group in environments that support it
    * @param {string} [label] - Optional label for the group
    * @returns {void}
    * @see group
    */
    public groupCollapsed(label?: string): void {
        this.group(label);
    }


    /**
     * @section Helpers for print with indentations
     */


    /**
    * Prints the current indentation to stdout
    * Used internally for formatting grouped log messages
    * @private
    * @returns {void}
    */
    private printIndentation(): void {
        this.print(Console.INDENT_MARGIN.repeat(this.indentationLevel));
    }

    /**
    * Prints the current indentation to stderr
    * Used internally for formatting grouped error messages
    * @private
    * @returns {void}
    */
    private printIndentationError(): void {
        this.printError(Console.INDENT_MARGIN.repeat(this.indentationLevel));
    }


    /**
    * @section Console methods for tabular data display.
    */

    /**
    * Displays an array of objects in tabular format
    * Converts the data to a DataFrame and renders it
    * @param {NullishType[]} data - Array of objects to display as a table
    * @returns {void}
    */
    public table(data: NullishType[]): void {
        const df = DataFrame.fromObjects(data);
        df.display();
    }

    /**
    * @section Console methods for tabular data display.
    */


    /**
    * Starts a timer with an optional label
    * Used to track execution time between time() and timeEnd() calls
    * @param {string} [label='default'] - Label to identify the timer
    * @returns {void}
    * @see timeEnd
    * @see timeLog
    */
    public time(label?: string): void {
        const key = label ?? 'default'
        if (this.timers.has(key))
        {
            this.warn("⚠️ Warning: Label 'default' already exists for console.time()")
            return
        }
        const start = Date.now() as long
        this.timers.set(key, start)
    }

    /**
    * Logs the current duration of a running timer without stopping it
    * Prints a warning if the specified timer doesn't exist
    * @param {string} [label='default'] - Label of the timer to check
    * @returns {void}
    * @see time
    */
    public timeLog(label?: string): void {
        const key = label ?? 'default'
        const startTime = this.timers.get(key)
        if (startTime === undefined) {
            this.warn(`Timer '${key}' does not exist`)
            return
        }
        const end = Date.now() as long
        const duration = end - startTime
        this.log("Start: ", startTime, " end:", end, " diff: ", duration)
        this.log(`${key}: ${duration}ms`)
    }

    /**
    * Stops a timer and logs its final duration
    * Removes the timer and prints a warning if it doesn't exist
    * @param {string} [label='default'] - Label of the timer to stop
    * @returns {void}
    * @see time
    */
    public timeEnd(label?: string): void {
        this.timeLog(label)
        this.timers.delete(label ?? 'default')
    }

    /**
    * Prints the current stack trace with an optional label
    * Skips the first stack frame (the trace call itself)
    * @param {NullishType[]} ...data - args to prints
    * @returns {void}
    */
    public trace(...data: NullishType[]): void {
        const stackT = StackTrace.provisionStackTrace();
        this.log(...data)
        for (let i = 1; i < stackT.length; ++i)
        {
            this.log(stackT[i]);
        }
    }


}

export const console = new Console();


/**
 * @section Dataframe helpers class for console.table API
 */

/** Column name type alias */
type ColumnName = string;

/** Column value type alias */
type ColumnValue = string;

/** Array of column values type alias */
type ColumnValues = Array<ColumnValue>;

/** Column width type alias */
type ColumnWidth = int;

/**
 * DataFrame class for handling and displaying tabular data
 * Provides functionality to create, manipulate, and render data in a table format
 */
class DataFrame {
    /** Map storing column names and their corresponding values */
    private tableModel: Map<ColumnName, ColumnValues>

    /** Map storing column names and their display widths */
    private columnWidths: Map<ColumnName, ColumnWidth>

    /** Number of rows in the DataFrame */
    private rowsCount: int

    /** Header text for the index column */
    private static readonly headerIndexRow: string = '| (index) |'

    /** Width of the index column based on header */
    private readonly _indexColumnWidth: int = DataFrame.headerIndexRow.length as int;

    /**
     * Creates a new DataFrame with specified columns
     * @param {Array<ColumnName>} columns - Array of column names
     * @ensures All columns are initialized with empty value arrays
     * @ensures Column widths are initialized to the length of column names
     */
    constructor(columns: Array<ColumnName>) {
        this.tableModel = new Map<ColumnName, ColumnValues>();
        this.columnWidths = new Map<ColumnName, ColumnWidth>();
        for (let column of columns) {
            this.tableModel.set(column, new Array<string>());
            this.columnWidths.set(column, column.length as int);
        }
        this.rowsCount = 0;
    }

    /**
     * Inserts a new row of data into the DataFrame
     * @param {Map<ColumnName, ColumnValue>} row - Map of column names to values
     * @ensures Column count remains unchanged
     * @ensures Column widths are updated if new values are longer
     */
    public insertRow(row: Map<ColumnName, ColumnValue>) {
        for (let key of this.tableModel.keys()) {
            const value = row.get(key) ?? "-";
            this.tableModel.get(key)!.push(value);
            const currentWidth = this.columnWidths.get(key)!;
            const valueLength = value.length as int;
            this.columnWidths.set(key, 
                                currentWidth < valueLength
                                ? valueLength
                                : currentWidth);
        }
        this.rowsCount++;
    }

    /**
     * Builds a border row for the table
     * @param {string} char - String to use for the border line
     * @param {string} commonConnector - Character for column separators
     * @param {string} connectorLeft - Character for left border
     * @param {string} connectorRight - Character for right border
     * @returns {string} Formatted border row
     * @private
     */
    private buildBorderRow(
        ch: string,
        commonConnector: string,
        connectorLeft: string,
        connectorRight: string,
    ): string {
        const connLength: int = 
            this._indexColumnWidth - 
            (connectorLeft.length as int) -
            (connectorRight.length as int);
        const conn: string = ch.repeat(connLength);
        let border: string = `${connectorLeft}${conn}`;
        for (const col of this.tableModel.keys()) {
            const colWidth = this.columnWidths.get(col)!;
            border += ch.repeat(colWidth + 2) + commonConnector;
        }
        return `${border}${connectorRight}`;
    }

    /**
     * Builds the header row containing column names
     * @returns {string} Formatted header row
     * @private
     */
    private buildHeaderRow(): string {
        let headerRow: string = DataFrame.headerIndexRow;
        for (const col of this.tableModel.keys()) {
            const colWidth = this.columnWidths.get(col)!;
            headerRow += ` ${col.padEnd(colWidth, ' ')} │`;
        }
        return headerRow;
    }

    /**
     * Builds a data row for the specified index
     * @param {number} rowIndex - Index of the row to build
     * @returns {string} Formatted data row
     * @private
     */
    private buildDataRow(rowIndex: number): string {
        const paddedIndex = `${rowIndex}`.padEnd(this._indexColumnWidth - 4);
        let row: string = `│ ${paddedIndex} │`;
        for (const col of this.tableModel.keys()) {
            const colWidth = this.columnWidths.get(col)!;
            const colContent = this.tableModel.get(col)![rowIndex];
            row += ` ${colContent.padEnd(colWidth)} │`;
        }
        return row;
    }

    /**
     * Renders the DataFrame as an array of strings
     * @returns {Array<String>} Array of formatted table rows
     */
    public render(): Array<String> {
        if (this.tableModel.size == 0) {
            return new Array<string>();
        }
        
        // row1
        const topBorder = this.buildBorderRow('─', '─', '┌', '┐');
        // row2
        const separator = this.buildBorderRow('─', '─', '├', '┤');
        // row3
        const bottomBorder = this.buildBorderRow('─', '─', '└', '┘');
        // header has 2 rows and footer - 1

        const output = new Array<string>(4 + this.rowsCount);

        // |output| = 0
        output[0] = topBorder
        // |output| = 1
        output[1] = this.buildHeaderRow()
        // |output| = 2
        output[2] = separator
        // |output| = 3
        
        for (let i = 0; i < this.rowsCount; i++) {
            output[3 + i] = this.buildDataRow(i);
        }
        // |output| = 3 + rowsCount
        output[3 + this.rowsCount] = bottomBorder;
        // |output| = 4 + rowsCount
        return output;
    }

    /**
     * Displays the DataFrame to the console
     * @returns {void}
     */
    public display(): void {
        const rows = this.render();
        for (const row of rows) {
            console.log(row);
        }
    }

    /**
    * Converts an object into a map of string key-value pairs for table representation.
    * Handles various types including primitives, collections, arrays, classes, and functions.
    * 
    * @param obj - The object to be converted into table info
    * @returns A Map containing string representations of object properties
    */
    private static getObjectInfoForTable(obj: NullishType): Map<string, string> {
        const mapping = new Map<string, string>();
        const objType = Type.of(obj);
        const objValue = Value.of(obj);

        // Handle primitive types
        if (obj === null) {
            mapping.set("Values", "null");
        } else if (obj === undefined) {
            mapping.set("Values", "undefined");
        } else if (obj instanceof String) {
            mapping.set("Values", `'${obj}'`);
        } else if (objType.isPrimitive()) {
            mapping.set("Values", `${obj}`);
        }

        // Handle complex types
        else if (obj instanceof Map) {
            const entries = obj.entries();
            for (let entry of entries) {
                mapping.set(`${entry[0]}`, `${entry[1]}`);
            }
        } else if (obj instanceof Set) {
            const values = obj.values();
            let i = 0;
            for (let value of values) {
                mapping.set(`{i}?`, `${value}`);
                i++;
            }
        } else if (objType instanceof ArrayType) {
            const arrayValue = objValue as ArrayValue;
            for (let i = 0; i < arrayValue.getLength(); i++) {
                mapping.set(`${i}`, arrayValue.getElement(i).toPrint(10));
            }
        } else if (objType instanceof ClassType) {
            const arrayValue = objValue as ClassValue;
            for (let i = 0; i < arrayValue.getFieldsNum(); i++) {
                try {
                    let fieldName = objType.getField(i).getName() as string;
                    // NOTE (templin.konstantin): replace internal-representation
                    //  fields like gensym%% with more human-readable field name
                    //  used in interfacesq
                    if (fieldName.startsWith("gensym%%")) {
                        fieldName = `Field${i}`;
                    }
                    if (obj instanceof Tuple){
                        fieldName = `${i}`;
                    }
                    mapping.set(
                        fieldName,
                        arrayValue.getField(i).toPrint(10)
                    );
                } catch (e) {
                    continue;
                }
            }
        } else if (objType instanceof FunctionType) {
            mapping.set("Values", `${objType}`);
        } else {
            mapping.set("Values", JSON.stringify(obj))
        }

        return mapping;
    }

    /**
     * Creates a DataFrame from an array of objects
     * @param {NullishType[]} data - Array of objects to convert
     * @returns {DataFrame} New DataFrame instance
     * @static
     */
    public static fromObjects(data: NullishType[]): DataFrame {
        if (data.length == 0) {
            return new DataFrame(new Array<ColumnName>());
        }
        const columns = new Set<string>();
        const preparedInfo = new Array<Map<string, string>>(data.length);
        for (let i = 0; i < data.length; ++i){
            preparedInfo[i] = DataFrame.getObjectInfoForTable(data[i]);
            for (let key of preparedInfo[i].keys())
            {
                columns.add(key);
            }
        }
        const df = new DataFrame(Array.from(columns));
        for (let i = 0; i < data.length; ++i) {
            df.insertRow(preparedInfo[i]);
        }
        return df;
    }

}
