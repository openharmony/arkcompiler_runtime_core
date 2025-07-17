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

import { BusinessError } from "@ohos.base";

/**
* The uri module provides utilities for URI resolution and parsing.
*
* @namespace uri
*/
export namespace uri {
  /**
  * URI Represents a Uniform Resource Identifier (URI) reference.
  *
  * @class URI
  */
  export class URI {
    uriEntry: UriEntry;

    /**
    * URI constructor, which is used to instantiate a URI object.
    * uri: Constructs a URI by parsing a given string.
    *
    * @param { string } uri - uri uri
    * @throws { BusinessError } 10200002 - Invalid uri string.
    */
    constructor(uri: string) {
      this.uriEntry = new UriEntry(uri);
    }

    /**
    * Creates an opaque Uri from the given components.
    *
    * @param { string } scheme -  of the URI.
    * @param { string } ssp -scheme-specific-part, everything between the scheme separator (':') and the fragment
    * separator ('#'), which will get encoded.
    * @param { string } fragment - fragment, everything after the '#', null if undefined, will get encoded.
    * @returns { URI } Return Uri consisting of a given scheme, SSP, and fragment.
    */
    static createFromParts(scheme: string, ssp: string, fragment: string): URI {
      let uriStr: string = scheme;
      uriStr += ':' + encodeURIComponent(ssp);
      if (fragment != "") {
        uriStr += '#' + encodeURIComponent(fragment);
      }
      return new URI(uriStr);
    }

    /**
    * Returns the serialized URI as a string.
    *
    * @returns { string } Returns the serialized URI as a string.
    */
    toString(): string {
      return toAsciiString(this.uriEntry.toString());
    }

    /**
    * Check whether this URI is equivalent to other URI objects.
    *
    * @param { URI } other - other other URI object to be compared
    * @returns { boolean } boolean Tests whether this URI is equivalent to other URI objects.
    */
    equalsTo(other: URI): boolean {
      return this.uriEntry.equalsTo(other.uriEntry);
    }

    /**
    * Indicates whether this URI is an absolute URI.
    *
    * @returns { boolean } boolean Indicates whether the URI is an absolute URI (whether the scheme component is defined).
    */
    checkIsAbsolute(): boolean {
      return this.uriEntry.isAbsolute();
    }

    /**
    * Determine whether URI is Relative.
    *
    * @returns { boolean } Return true as Relative, otherwise return false.
    */
    checkRelative(): boolean {
      return this.uriEntry.isRelative();
    }

    /**
    * Determine whether URI is Opaque.
    *
    * @returns { boolean } Return true as Opaque, otherwise return false.
    */
    checkOpaque(): boolean {
      return this.uriEntry.isOpaque();
    }

    /**
    * Determine whether URI is hierarchical.
    *
    * @returns { boolean } Return true as Hierarchical, otherwise return false.
    */
    checkHierarchical(): boolean {
      return this.uriEntry.isHierarchical();
    }

    /**
    * Encodes the key and value and then appends the result to the query string.
    *
    * @param { string } [key] - The key it will be encoded with.
    * @param { string } [value] - The value it will be encoded with.
    * @returns { URI } Return URI object.
    */
    addQueryValue(key: string, value: string): URI {
      let uriStr = this.uriEntry.addQueryValue(encodeURIComponent(key), encodeURIComponent(value));
      return new URI(uriStr);
    }

    /**
    * Creates a new Uri by appending an already-encoded path segment to a base Uri.
    *
    * @param { string } encodedPathSegment - Encoded path segment to be added.
    * @returns { URI } After adding, return the URI object.
    */
    addEncodedSegment(encodedPathSegment: string): URI {
      let uriStr = this.uriEntry.addSegment(encodedPathSegment);
      return new URI(uriStr);
    }

    /**
    * Encodes the given path segment and appends it to the path.
    *
    * @param { string } [pathSegment] - path segment to be added.
    * @returns { URI } After adding, return the URI object.
    */
    addSegment(pathSegment: string): URI {
      let uriStr = this.uriEntry.addSegment(encodeURIComponent(pathSegment));
      return new URI(uriStr);
    }

    /**
    * Searches the query string for the first value with the given key.
    *
    * @param { string } key - Given the first value of the key.
    * @returns { string | null } Return decoded value.
    */
    getQueryValue(key: string): string | null {
      return this.uriEntry.getQueryValue(key);
    }

    /**
    * Returns a set of the unique names of all query parameters.
    *
    * @returns { Array<string> } Return a set of decoded names.
    */
    getQueryNames(): Array<string> {
      return this.uriEntry.getQueryNames();
    }

    /**
    * Searches the query string for parameter values with the given key.
    *
    * @param { string } key - The key it will be encoded with.
    * @returns { Array<string> } Return a set of decoded values.
    */
    getQueryValues(key: string): Array<string> {
      return this.uriEntry.getQueryValues(key);
    }

    /**
    * Searches the query string for the first value with the given key and interprets it as a boolean value.
    *
    * @param { string } key - Indicates the key value to be queried.
    * @param { boolean } defaultValue - The default value returned when the key has no query parameters.
    * @returns { boolean } Query with key value returns true, otherwise returns false.
    */
    getBooleanQueryValue(key: string, defaultValue: boolean): boolean {
      let flag = this.uriEntry.getQueryValue(key);
      if (flag == null) {
        return defaultValue;
      }
      flag = flag.toLocaleLowerCase();
      return 'false' != flag && '0' != flag;
    }

    /**
    * Gets the decoded last path segment.
    *
    * @returns { string } Returns the last decoded segment, or null if the path is empty.
    */
    getLastSegment(): string {
      let segments = this.uriEntry.getSegments();
      if (segments.length == 0) {
        return "";
      }
      return segments[segments.length - 1];
    }

    /**
    * Gets the decoded path segments.
    *
    * @returns { Array<string> } Return decoded path segments, each without a leading or trailing "/".
    */
    getSegment(): Array<string> {
      return this.uriEntry.getSegments();
    }

    /**
    * Clears the previously set query.
    *
    * @returns { URI } After clearing, return the URI object.
    */
    clearQuery(): URI {
      let uriStr: string = this.uriEntry.clearQuery();
      return new URI(uriStr);
    }

    /**
    * Normalize the path of this URI, It is not safe to call the normalize interface with URI.
    *
    * @returns { URI } URI Used to normalize the path of this URI and return a URI object whose path has been normalized.
    */
    normalize(): URI {
      let uriStr: string = this.uriEntry.normalize();
      return new URI(uriStr);
    }

    /**
    * Gets the protocol part of the URI.
    *
    * @returns { string | null }
    */
    get scheme(): string | null {
      if (this.uriEntry.getScheme() == '') {
        return null;
      }
      return this.uriEntry.getScheme();
    }

    /**
    * Gets the authority part of the URI.
    *
    * @returns { string | null }
    */
    get authority(): string | null {
      let s: string = this.uriEntry.getAuthority();
      return s == "" ? null : this.dealDecodeInput(s);
    }

    /**
    * Gets the decoding scheme-specific part of the URI.
    *
    * @returns { string }
    */
    get ssp(): string {
      return this.dealDecodeInput(this.uriEntry.getSsp())
    }

    private dealDecodeInput(input: string): string {
      let index1 = input.indexOf('[');
      if (index1 == -1) {
        return decodeURIComponent(input);
      }
      let index2 = input.indexOf(']', index1);
      let split1 = input.substring(0, index1);
      let split2 = input.substring(index1 + 1, index2);
      let split3 = input.substring(index2 + 1);
      return decodeURIComponent(split1) + '[' + split2 + ']' + decodeURIComponent(split3);
    }

    /**
    * Gets Obtains the user information part of the URI.
    *
    * @returns { string | null }
    */
    get userInfo(): string | null {
      let s: string = this.uriEntry.getUserinfo();
      return s == '' ? null : decodeURIComponent(s);
    }

    /**
    * Gets the hostname portion of the URI without a port.
    *
    * @returns { string | null }
    */
    get host(): string | null {
      let s: string = this.uriEntry.getHost();
      return s == '' ? null : s;
    }

    /**
    *  Gets the port portion of the URI.
    *
    * @returns { string }
    */
    get port(): string {
      return Number.toString(this.uriEntry.getPort());
    }

    /**
    * Gets the path portion of the URI.
    *
    * @returns { string | null }
    */
    get path(): string | null {
      let s: string = this.uriEntry.getPath();
      return s == '' ? null : decodeURIComponent(s);
    }

    /**
    * Gets the query portion of the URI
    *
    * @returns { string | null }
    */
    get query(): string | null {
      let s: string = this.uriEntry.getQuery();
      return s == '' ? null : decodeURIComponent(s);
    }

    /**
    * Gets the fragment portion of the URI
    *
    * @returns { string | null }
    */
    get fragment(): string | null {
      let s: string = this.uriEntry.getFragment();
      return s == '' ? null : decodeURIComponent(s);
    }

    /**
    * Gets Obtains the encoded user information part of the URI.
    *
    * @returns { string | null }
    */
    get encodedUserInfo(): string | null {
      let s: string = this.uriEntry.getUserinfo();
      return s == '' ? null : s;
    }

    /**
    * Gets the encoded path portion of the URI .
    *
    * @returns { string | null }
    */
    get encodedPath(): string | null {
      let s: string = this.uriEntry.getPath();
      return s == '' ? null : s;
    }

    /**
    * Gets the encoded query component from this URI.
    *
    * @returns { string | null }
    */
    get encodedQuery(): string | null {
      let s: string = this.uriEntry.getQuery();
      return s == '' ? null : s;
    }

    /**
    * Gets the encoded fragment part of this URI, everything after the '#'.
    *
    * @returns { string | null }
    */
    get encodedFragment(): string | null {
      let s: string = this.uriEntry.getFragment();
      return s == '' ? null : s;
    }

    /**
    * Gets the encoded authority part of this URI.
    *
    * @returns { string | null }
    */
    get encodedAuthority(): string | null {
      let s: string = this.uriEntry.getAuthority();
      return s == '' ? null : s;
    }

    /**
    * Gets the scheme-specific part of this URI, i.e. everything between the scheme separator ':' and
    * the fragment separator '#'.
    *
    * @returns { string }
    */
    get encodedSSP(): string {
      return this.uriEntry.getSsp();
    }

    /**
     * Sets the protocol part of the URI.
     *
     * @param { string | null } input
     */
    set scheme(input: string | null) {
      if (input === null || input.length === 0) {
        return;
      }
      this.uriEntry.setScheme(input);
    }

    /**
     * Sets the path portion of the URI.
     * @param { string | null } input
     */
    set path(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setPath(encodeURI(input));
    }

    /**
     * Sets the decoding scheme-specific part of the URI.
     * @param { string | null } input
     * NOTE(zhangziye):#25267 Unable to use different getter/setter signature 
     */
    set ssp(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setSsp(encodeURI(input))
    }

    /**
     * Sets the decoding permission component part of this URI.
     * @param { string | null } input
     */
    set authority(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setAuthority(encodeURI(input))
    }

    /**
     * Sets Obtains the user information part of the URI.
     * @param { string | null } input
     */
    set userInfo(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setUserInfo(encodeURIComponent(input));
    }

    /**
     * Sets the query portion of the URI
     * @param { string | null } input
     */
    set query(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setQuery(encodeURIComponent(input));
    }

    /**
     * Sets the fragment portion of the URI
     * @param { string | null } input
     */
    set fragment(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setFragment(encodeURIComponent(input));
    }

    /**
     * Sets Obtains the encoded user information part of the URI.
     * @param { string | null } input
     */
    set encodedUserInfo(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setUserInfo(input);
    }

    /**
     * Sets the encoded path portion of the URI.
     * @param { string | null } input
     */
    set encodedPath(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setPath(input);
    }

    /**
     * Sets the encoded query component from this URI.
     * @param { string | null } input
     */
    set encodedQuery(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setQuery(input);
    }

    /**
     * Sets the encoded fragment component from this URI.
     * @param { string | null } input
     */
    set encodedFragment(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setFragment(input);
    }

    /**
     * Sets the encoded authority component from this URI.
     * @param { string | null } input
     */
    set encodedAuthority(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setAuthority(input);
    }
 
    /**
     * NOTE(zhangziye):#25267 Unable to use different getter/setter signature 
     */
    set encodedSSP(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setSsp(input);
    }
  }

  function toAsciiString(uriStr: string): string {
    return encodeURI(uriStr)
      .replaceAll("%5B", '[')
      .replaceAll("%5D", ']')
      .replaceAll("%25", '%');
  }

  /**
  * Handling specific URI logic
  */
  class UriEntry {
    private errStr: string = "";
    private data: string = "";
    private inputUri: string = "";

    private port: number = -1;
    private scheme: string = "";
    private userInfo: string = "";
    private host: string = "";
    private query: string = "";
    private fragment: string = "";
    private path: string = "";
    private authority: string = "";
    private schemeSpecificPart: string = "";

    private REG_ALPHA = new RegExp("[a-zA-Z]", 'g');
    private REG_SCHEME = new RegExp("[\\da-zA-Z+\\-.| _~!$&=,;'(){}*]", 'g');
    private REG_URLC = new RegExp("[\\da-zA-Z+\\-.| _~!$&=,;'(){}*/?:@\\[\\]%\"]", 'g');
    private REG_PATH = new RegExp("[\\da-zA-Z+\\-.| _~!$&=,;'(){}*/:@%]", 'g');
    private REG_USER_INFO = new RegExp("[\\da-zA-Z+\\-.| _~!$&=,;'(){}*:%]", 'g');
    private REG_PORT = new RegExp("[\\da-zA-Z.:@\\-;&=+$,_!~*'()]", 'g');
    private REG_NONASC = new RegExp("[^\\x00-\\x7F]", 'g');

    constructor(input: string) {
      if (input == "") {
        this.errStr = "Uri is empty.";
        this.checkErrAndThrow();
      }
      this.inputUri = input;
      this.analysisUri();
    }
    getQueryValue(key: string): string | null {
      let queryList = this.getQueryList();
      for (let i = 0; i < queryList.length; i += 2) {
        if (decodeURIComponent(queryList[i]) == key) {
          return decodeURIComponent(queryList[i + 1].replaceAll("+", ' '));
        }
      }
      return null;
    }

    getQueryValues(key: string): Array<string> {
      let queryList = this.getQueryList();
      let values = new Array<string>();
      for (let i = 0; i < queryList.length; i += 2) {
        if (decodeURIComponent(queryList[i]) == key) {
          values.push(decodeURIComponent(queryList[i + 1]));
        }
      }
      return values;
    }
    getQueryNames(): Array<string> {
      let queryList = this.getQueryList();
      let names = new Set<string>();
      for (let i = 0; i < queryList.length; i += 2) {
        names.add(decodeURIComponent(queryList[i]));
      }
      return Array.from<string>(names);
    }
    private assignSchemeSpecificPart() {
      this.schemeSpecificPart += this.data;
      if (this.query != "") {
        this.schemeSpecificPart += "?";
        this.schemeSpecificPart += this.query;
      }
    }

    private analysisUri() {
      this.data = this.inputUri;
      let pos = this.data.indexOf('#');// Fragment
      if (pos != -1) {
        this.analysisFragment(pos);
        this.checkErrAndThrow();
      }
      pos = this.data.indexOf('?');// Query
      if (pos != -1) {
        this.analysisQuery(pos);
        this.checkErrAndThrow();
      }
      pos = this.data.indexOf(':'); // Scheme
      if (pos != -1) {
        this.analysisScheme(pos);
        this.checkErrAndThrow();
      } else {
        this.specialPath();
        this.checkErrAndThrow();
        this.assignSchemeSpecificPart();
        return;
      }
      pos = this.data.indexOf("//"); // userInfo path host port ipv4 or ipv6
      if (pos == 0) {
        this.assignSchemeSpecificPart();
        this.data = this.data.substring(2); // 2:Intercept the string from the second subscript
        this.analysisHostAndPath();
        this.checkErrAndThrow();
      } else if (this.data.length != 0 && this.data[0] == c'/') {
        this.path = this.data;
        this.assignSchemeSpecificPart();
        this.data = "";
      } else {
        this.assignSchemeSpecificPart();
        this.query = "";
        this.data = "";
      }
    }

    private checkCharacter(str: string, regex: RegExp, ignoreNonAscll: boolean) {
      if (ignoreNonAscll) {
        str = str.replaceAll(this.REG_NONASC, '');
      }
      str = str.replaceAll(regex, "");
      return str == '';
    }

    private getQueryList(): Array<string> {
      let queryList = new Array<string>();
      if (this.query == "" || this.query == null) {
        return queryList;
      }
      for (let str of this.query.split('&')) {
        let pare = str.split('=');
        let key = pare.length > 0 ? pare[0] : '';
        let value = pare.length > 1 ? pare[1] : '';
        queryList.push(key);
        queryList.push(value);
      }
      return queryList;
    }

    private specialPath() {
      if (!this.checkCharacter(this.data, this.REG_PATH, true)) {
        this.errStr = "SpecialPath does not conform to the rule.";
        return;
      }
      this.path = this.data;
      this.data = "";
    }

    private analysisFragment(pos: number) {
      if (pos == 0) {
        this.errStr = "#It can't be the first.";
        return;
      }
      let fragment = this.data.substring(pos + 1);
      if (!this.checkCharacter(fragment, this.REG_URLC, true)) {
        this.errStr = "Fragment does not conform to the rule.";
        return;
      }
      this.fragment = fragment;
      this.data = this.data.substring(0, pos);
    }

    private analysisQuery(pos: number) {
      let query = this.data.substring(pos + 1);
      if (!this.checkCharacter(query, this.REG_URLC, true)) {
        this.errStr = "Query does not conform to the rule.";
        return;
      }
      this.query = query;
      this.data = this.data.substring(0, pos);
    }

    private analysisScheme(pos: number) {
      let slashPos = this.data.indexOf('/');
      if (slashPos != -1 && slashPos < pos) {
        this.specialPath();
        this.schemeSpecificPart += (this.path);
        this.schemeSpecificPart += ("?");
        this.schemeSpecificPart += (this.query);
        this.data = "";
      } else {
        if (!this.REG_ALPHA.test(this.data.substring(0, 1))) {
          this.errStr = "Scheme the first character must be a letter.";
          return;
        }
        let scheme = this.data.substring(0, pos);
        if (!this.checkCharacter(scheme, this.REG_SCHEME, false)) {
          this.errStr = "Scheme does not conform to the rule.";
          return;
        }
        this.scheme = scheme;
        this.data = this.data.substring(pos + 1);
      }
    }

    private analysisHost(isLawfulPort: boolean) {
      // find ipv4 or ipv6 or host
      if (this.data.length > 0 && this.data[0] == c'[') {
        if (this.data[this.data.length - 1] == c']') {
          // IPV6
          if (!isLawfulPort) {
            this.errStr = "Port does not conform to the rule.";
            return;
          }
          this.analysisIPV6();
        } else {
          this.errStr = "IPv6 is missing a closing bracket.";
          return;
        }
      } else {
        if (this.data.indexOf('[') != -1 || this.data.indexOf(']') != -1) {
          this.errStr = "Host does not conform to the rule.";
          return;
        }
        // ipv4
        if (!isLawfulPort || !this.analysisIPV4()) {
          this.port = -1;
          this.host = "";
          this.userInfo = "";
        }
      }
    }

    private analysisHostAndPath() {
      if (this.data == "") {
        return;
      }
      // find path
      let pos = this.data.indexOf('/');
      if (pos != -1) {
        this.analysisPath(pos);
        this.checkErrAndThrow();
      }
      this.authority = this.data;
      // find UserInfo
      pos = this.data.indexOf('@');
      if (pos != -1) {
        this.analysisUserInfo(pos);
        this.checkErrAndThrow();
      }
      let isLawfulPort = true;
      // find port
      pos = this.data.lastIndexOf(':');
      if (pos != -1) {
        let pos1 = this.data.lastIndexOf(']');
        if (pos1 == -1 || pos > pos1) {
          isLawfulPort = this.analysisPort(pos);
        }
        this.checkErrAndThrow();
      }
      this.analysisHost(isLawfulPort);
    }

    private analysisPath(pos: number) {
      let path = this.data.substring(pos);
      if (!this.checkCharacter(path, this.REG_PATH, true)) {
        this.errStr = "Path does not conform to the rule.";
        return;
      }
      this.path = path;
      this.data = this.data.substring(0, pos);
    }

    private analysisUserInfo(pos: number) {
      let userInfo = this.data.substring(0, pos);
      if (!this.checkCharacter(userInfo, this.REG_USER_INFO, true)) {
        this.errStr = "UserInfo does not conform to the rule.";
        return;
      }
      this.userInfo = userInfo;
      this.data = this.data.substring(pos + 1);
    }

    private analysisPort(pos: number) {
      let portStr = this.data.substring(pos + 1);
      if (!this.checkCharacter(portStr, this.REG_PORT, true)) {
        this.errStr = "Port does not conform to the rule.";
        return false;
      }

      if (portStr == '') {
        return false;
      }

      let port = new Number(portStr);
      if (isNaN(port)) {
        this.data = this.data.substring(0, pos);
        return false;
      }
      if (port < 0 || port > Int.MAX_VALUE) {
        return false;
      }
      this.port = port;
      this.data = this.data.substring(0, pos);
      return true;
    }

    private analysisIPV4() {
      let ipv4 = new RegExp("^((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)$");
      let hostname = new RegExp("^(([a-zA-Z0-9]([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9])?\\.)+([a-zA-Z]"
        + "([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s])?))$|^([a-zA-Z0-9]([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9])?)$");
      let isIpv4 = ipv4.test(this.data);
      let isHosName = hostname.test(this.data);
      if (!isIpv4 && !isHosName) {
        return false;
      } else {
        this.host = this.data;
        this.data = "";
        return true;
      }
    }

    private analysisIPV6() {
      let str = this.data.substr(1, this.data.length - 2); // 2:Intercept the string from the second subscript
      let ipv6 = new RegExp("^(::|(:((:[0-9A-Fa-f]{1,4}){1,7}))|(([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|"
        + "(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|:))|(([0-9A-Fa-f]{1,4}:){2}"
        + "(((:[0-9A-Fa-f]{1,4}){1,5})|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})"
        + "|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|:))|(([0-9A-Fa-f]{1,4}:){5}"
        + "(((:[0-9A-Fa-f]{1,4}){1,2})|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|:))|"
        + "(((:(:[0-9A-Fa-f]{1,4}){0,5}:)|(([0-9A-Fa-f]{1,4}:){1}(:[0-9A-Fa-f]{1,4}){0,4}:)"
        + "|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){0,3}:)|(([0-9A-Fa-f]{1,4}:){3}"
        + "(:[0-9A-Fa-f]{1,4}){0,2}:)|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4})?:)|"
        + "(([0-9A-Fa-f]{1,4}:){5}:)|(([0-9A-Fa-f]{1,4}:){6}))((25[0-5]|2[0-4]\\d|1\\d{2}|"
        + "[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)))(%[a-zA-Z0-9._]+)?$");
      if (!ipv6.test(str)) {
        this.errStr = "Ipv6 does not conform to the rule.";
        return;
      }
      this.host = this.data;
      this.data = "";
    }

    equalsTo(other: UriEntry): boolean {
      return (this.port == other.port) && (this.scheme == other.scheme)
        && (this.userInfo == other.userInfo) && (this.host == other.host)
        && (this.query == other.query) && (this.fragment == other.fragment)
        && (this.path == other.path) && (this.authority == other.authority)
        && (this.schemeSpecificPart == other.schemeSpecificPart);
    }

    toString(): string {
      return this.inputUri;
    }

    isAbsolute(): boolean {
      return this.scheme != "";
    }

    isRelative(): boolean {
      return this.scheme == "";
    }

    isOpaque(): boolean {
      return !this.isHierarchical();
    }

    isHierarchical(): boolean {
      let index = this.inputUri.indexOf(':');
      if (index == -1) {
        return true;
      }
      if (this.inputUri.length == index + 1) {
        return false;
      }
      return this.inputUri[index + 1] == c'/';
    }

    addQueryValue(key: string, value: string) {
      return this.buildUriString("query", key + "=" + value);
    }

    addSegment(pathSegment: string) {
      return this.buildUriString("segment", pathSegment);
    }

    private buildUriString(str: string, param: string) {
      let result = "";
      if (this.scheme != "") {
        result += this.scheme + ":";
      }
      if (this.authority != "") {
        result += "//" + this.authority;
      }
      if (this.path != "") {
        result += this.path;
      }
      if (str == "segment") {
        let lastChar = result[result.length - 1];
        if (lastChar == c'/') {
          result += param;
        } else {
          result += "/" + param;
        }
      }
      if (str != "clearquery") {
        if (this.query == "") {
          if (str == "query") {
            result += "?" + param;
          }
        } else {
          result += "?" + this.query;
          if (str == "query") {
            result += "&" + param;
          }
        }
      }
      if (this.fragment != "") {
        result += "#" + this.fragment;
      }
      return result;
    }

    getSegments(): Array<string> {
      let segmentArray = new Array<string>();
      for (let segment of this.path.split('/')) {
        if (segment.trim() != '') {
          segmentArray.push(decodeURIComponent(segment.trim()))
        }
      }
      return segmentArray;
    }

    private checkErrAndThrow() {
      if (this.errStr != "") {
        const SyntaxErrorCodeId: number = 10200002;
        throw new BusinessError(SyntaxErrorCodeId, new Error('BusinessError', `Syntax Error. Invalid Uri string: The ${this.errStr}`, undefined));
      }
    }

    normalize(): string {
      let temp = new Array<string>;
      let pathLen = this.path.length;
      if (pathLen == 0) {
        return this.inputUri;
      }
      let pos = 0;
      let left = 0;
      while ((pos = this.path.indexOf('/', left)) != -1) {
        temp.push(this.path.substring(left, pos));
        left = pos + 1;
      }
      if (left != pathLen) {
        temp.push(this.path.substring(left));
      }
      const STR_DOTDOT = '..';
      let tempLen = temp.length;
      let normalizeTemp = new Array<string>;
      for (let i = 0; i < tempLen; ++i) {
        if (temp[i] != "" && temp[i] != '.' && temp[i] != STR_DOTDOT) {
          normalizeTemp.push(temp[i]);
        }
        if (temp[i] == STR_DOTDOT) {
          if (normalizeTemp.length != 0 && normalizeTemp[normalizeTemp.length - 1] != STR_DOTDOT) {
            normalizeTemp.pop();
          } else {
            normalizeTemp.push(temp[i]);
          }
        }
      }
      let normalizePath = "";
      tempLen = normalizeTemp.length;
      if (tempLen == 0) {
        normalizePath = "/";
      } else {
        for (let i = 0; i < tempLen; ++i) {
          normalizePath += "/" + normalizeTemp[i];
        }
      }
      return this.split(normalizePath);
    }

    private split(path: string): string {
      let normalizeUri = "";
      if (this.scheme != "") {
        normalizeUri += this.scheme + ":";
      }
      if (this.path == "") {
        normalizeUri += this.schemeSpecificPart;
      } else {
        if (this.host != "") {
          normalizeUri += "//";
          if (this.userInfo != "") {
            normalizeUri += this.userInfo + "@";
          }
          normalizeUri += this.host;
          if (this.port != -1) {
            normalizeUri += ":" + Number.toString(this.port);
          }
        } else if (this.authority != "") {
          normalizeUri += "//" + this.authority;
        }
        normalizeUri += path;
      }
      if (this.query != "") {
        normalizeUri += "?" + this.query;
      }
      if (this.fragment != "") {
        normalizeUri += '#' + this.fragment;
      }
      return normalizeUri;
    }

    getScheme() {
      return this.scheme;
    }

    getAuthority() {
      return this.authority;
    }

    getSsp() {
      return this.schemeSpecificPart;
    }

    getUserinfo() {
      return this.userInfo;
    }

    getHost() {
      return this.host;
    }

    getPort() {
      return this.port;
    }

    getPath() {
      return this.path;
    }

    getQuery() {
      return this.query;
    }

    getFragment() {
      return this.fragment;
    }

    clearQuery() {
      return this.buildUriString("clearquery", "");
    }

    setScheme(scheme: string): void {
      if (!this.checkCharacter(scheme.substring(0, 1), this.REG_ALPHA, true)) {
        this.errStr = "Scheme the first character must be a letter.";
        return;
      }
      let temp: string = "";
      const pos: number = scheme.indexOf(':');
      if (pos !== -1) {
        temp = scheme.substring(0, pos);
      } else {
        temp = scheme;
      }

      if (!this.checkCharacter(temp, this.REG_SCHEME, false)) {
        this.errStr = "Scheme does not conform to the rule.";
        return;
      }
      this.scheme = temp;
      this.inputUri = this.updateToString();
    }

    setAuthority(authorityStr: string): void {
      // Reset values
      this.port = -1;
      this.host = "";
      this.userInfo = "";
      this.authority = authorityStr;
      this.data = authorityStr;

      // Find UserInfo
      const atPos = this.data.indexOf('@');
      if (atPos !== -1) {
        const userStr = this.data.substring(0, atPos);
        this.data = this.data.substring(atPos + 1);

        if (!this.checkCharacter(userStr, this.REG_USER_INFO, true)) {
          this.errStr = "userInfo does not conform to the rule";
          return;
        }
        this.userInfo = userStr;
      }

      let isLawfulPort = true;
      // Find port
      const colonPos = this.data.lastIndexOf(':');
      if (colonPos !== -1) {
        const bracketPos = this.data.lastIndexOf(']');
        if (bracketPos === -1 || colonPos > bracketPos) {
          isLawfulPort = this.analysisPort(colonPos);
        }
        if (this.errStr) {
          return;
        }
      }
      this.analysisHost(isLawfulPort);
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setSsp(sspStr: string): void {
      // Reset all relevant fields
      this.authority = "";
      this.port = -1;
      this.host = "";
      this.userInfo = "";
      this.query = "";
      this.path = "";
      this.schemeSpecificPart = "";
      this.data = sspStr;

      // Handle query part
      const queryPos = this.data.indexOf('?');
      if (queryPos !== -1) {
        this.analysisQuery(queryPos);
      }

      // Handle path and host parts
      const doubleSlashPos = this.data.indexOf('//');
      if (doubleSlashPos !== -1 && doubleSlashPos === 0) {
        this.assignSchemeSpecificPart();
        this.data = this.data.substring(2); // Skip the '//'
        this.analysisHostAndPath();
      } else if (this.data[0] === c'/') {
        this.path = this.data;
        this.assignSchemeSpecificPart();
        this.data = "";
      } else {
        this.assignSchemeSpecificPart();
        this.path = "";
        this.query = "";
        this.data = "";
      }
      this.inputUri = this.updateToString();
    }

    setUserInfo(userInfo: string): void {
      if (!this.host || !this.checkCharacter(userInfo, this.REG_USER_INFO, true)) {
        this.errStr = "userInfo does not conform to the rule";
        return;
      }
      this.userInfo = userInfo;
      this.updateAuthority();
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setPath(pathStr: string): void {
      if (!this.checkCharacter(pathStr, this.REG_PATH, true)) {
        this.errStr = "pathStr does not conform to the rule";
        return;
      }
      this.path = pathStr;
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setQuery(queryStr: string): void {
      if (!this.checkCharacter(queryStr, this.REG_URLC, true)) {
        this.errStr = "QueryStr does not conform to the rule";
        return;
      }
      this.query = queryStr;
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setFragment(fragmentStr: string): void {
      if (!this.checkCharacter(fragmentStr, this.REG_URLC, true)) {
        this.errStr = "Fragment does not conform to the rule";
        return;
      }
      this.fragment = fragmentStr;
      this.inputUri = this.updateToString();
    }

    private updateAuthority(): void {
      let temp = this.userInfo;
      if (this.userInfo) {
        temp += "@";
      }
      temp += this.host;
      if (this.port !== -1) {
        temp += `:${this.port}`;
      }
      this.authority = temp;
    }

    private updateSsp(): void {
      let temp = "";
      if (this.authority) {
        temp += `//${this.authority}`;
      }
      if (this.path) {
        temp += this.path.startsWith('/')
          ? this.path
          : `/${this.path}`;
      }
      if (this.query) {
        temp += `?${this.query}`;
      }
      this.schemeSpecificPart = temp;
    }

    private updateToString(): string {
      let uriStr = "";
      let addQuery = false;
      if (this.scheme) {
        uriStr += this.scheme + ":";
      }
      if (!this.path) {
        uriStr += this.schemeSpecificPart;
        addQuery = true;
      } else {
        if (this.host) {
          uriStr += "//";
          if (this.userInfo) {
            uriStr += this.userInfo + "@";
          }
          uriStr += this.host;
          if (this.port !== -1) {
            uriStr += ":" + this.port;
          }
        } else if (this.authority) {
          uriStr += "//" + this.authority;
        }
        if (this.path) {
          uriStr += this.path.startsWith('/')
            ? this.path
            : "/" + this.path;
        }
      }
      if (this.query && !addQuery) {
        uriStr += "?" + this.query;
      }

      if (this.fragment) {
        uriStr += "#" + this.fragment;
      }
      return uriStr;
    }
  }


}
