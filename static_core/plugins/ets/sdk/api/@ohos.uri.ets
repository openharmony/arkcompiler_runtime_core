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

const SyntaxErrorCodeId: number = 10200002;
const TypeErrorCodeId: number = 401;

function createBusinessError(code: number, message:string){
    let err = new BusinessError();
    err.code = code;
    err.name='BusinessError';
    err.message = message;
    return err;
}

export namespace uri {

  export class URI {
    uriEntry: UriEntry;
    constructor(input: string) {
      this.uriEntry = new UriEntry(input);
      let errStr: string = this.uriEntry.getErrStr();
      if (errStr != "") {
        throw createBusinessError(SyntaxErrorCodeId,`Syntax Error. Invalid Uri string: The ${errStr}`)
      }
    }

    static createFromParts(scheme: string, ssp: string, fragment: string): URI {
      let uriStr: string = scheme;
      uriStr += ':' + encodeURIComponent(ssp);
      if (fragment != "") {
        uriStr += '#' + encodeURIComponent(fragment);
      }
      return createNewUri(uriStr);
    }

    toString(): string {
      return toAscllString(this.uriEntry.toString());
    }

    equals(other: URI): boolean {
      return this.uriEntry.equals(other.uriEntry);
    }

    equalsTo(other: URI): boolean {
      return this.uriEntry.equals(other.uriEntry);
    }

    checkIsAbsolute(): boolean {
      return this.uriEntry.isAbsolute();
    }

    checkRelative(): boolean {
      return this.uriEntry.isRelative();
    }

    checkOpaque(): boolean {
      return this.uriEntry.isOpaque();
    }

    checkHierarchical(): boolean {
      return this.uriEntry.isHierarchical();
    }

    addQueryValue(key: string, value: string): URI {
      let uriStr = this.uriEntry.addQueryValue(encodeURIComponent(key), encodeURIComponent(value));
      return createNewUri(uriStr);
    }

    addEncodedSegment(encodedPathSegment: string): URI {
      let uriStr = this.uriEntry.addSegment(encodedPathSegment);
      return createNewUri(uriStr);
    }

    addSegment(pathSegment: string): URI {
      let uriStr = this.uriEntry.addSegment(encodeURIComponent(pathSegment));
      return createNewUri(uriStr);
    }

    getQueryValue(key: string): string | null {
      let query = this.uriEntry.getQuery();
      if (query === '') {
        return null;
      }
      const queryStrs = query.split('&');
      for (const item of queryStrs) {
        const eqPos = item.indexOf('=');
        let currentKey = item;
        let currentValue = '';
        if (eqPos !== -1) {
          currentKey = item.substring(0, eqPos);
          currentValue = item.substring(eqPos + 1);
        }

        const decodedKey = this.decodeSafelyInner(currentKey);
        if (decodedKey === key) {
          const decodedValue = this.decodeSafelyInner(currentValue.replaceAll('+', ' '));
          return decodedValue || '';
        }
      }
      return null;
    }

    private decodeSafelyOut(input: string): string {
      let decodedString = "";
      let decodedTemp = "";
      let index: number = 0;
      while (index < input.length) {
        let isHexDigit: boolean = Char.isHexDigit(input[index + 1]) && Char.isHexDigit(input[index + 2]);
        if (input[index] == c'%' && isHexDigit) {
          const encodedChar = input.slice(index, index + 3);
          try {
            decodedString += decodeURIComponent(decodedTemp + encodedChar);
            decodedTemp = "";
          } catch (e) {
            decodedTemp += encodedChar;
          }
          index += 3;
          continue;
        }
        decodedString += decodedTemp + input[index];
        decodedTemp = "";
        index++;
      }
      return decodedString + decodedTemp;
    }

    private decodeSafelyInner(input: string): string {
      if (input == "") {
        return input;
      }
      try {
        return decodeURIComponent(input);
      } catch (e) {
        return this.decodeSafelyOut(input);
      }
    }

    getQueryNames(): Array<string> {
      if (this.query == null || this.query == "") {
        return new Array<string>();
      }
      let names = new Set<string>();
      let query = this.query!;
      let start: number = 0;
      while (start < query.length) {
        let next: number = query.indexOf('&', start);
        let end: number = (next == -1) ? query.length : next;
        let separator: number = query.indexOf('=', start);
        if (separator > end || separator == -1) {
          separator = end;
        }
        let name = query.substring(start, separator);
        names.add(this.decodeSafelyInner(name));
        start = end + 1;
      }
      return Array.from<string>(names);
    }

    getQueryValues(key: string): Array<string> {
      if (this.query == null || this.query == "") {
        return new Array<string>();
      }
      let values = new Array<string>();
      let queryStrs: string[] = this.query!.split('&');
      let isKeyEmpty = key == "";
      for (let item of queryStrs) {
        if (isKeyEmpty && item == "") {
          values.push(item);
        }
        let strArr: string[] = item.split('=');
        let isKeyEqualStr0 = this.decodeSafelyInner(strArr[0]) == key;
        if (strArr.length == 1 && isKeyEqualStr0) {
          values.push("");
        } else if (isKeyEqualStr0) {
          values.push(this.decodeSafelyInner(item.substring(strArr[0].length + 1)));
        }
      }
      return values;
    }

    getBooleanQueryValue(key: string, defaultValue: boolean): boolean {
      let flag = this.getQueryValue(key);
      if (flag == null) {
        return defaultValue;
      }
      flag = flag.toLocaleLowerCase();
      return 'false' != flag && '0' != flag;
    }

    getLastSegment(): string {
      let segments = this.uriEntry.getSegment();
      if (segments.length == 0) {
        return "";
      }
      return this.decodeSafelyInner(segments[segments.length - 1]);
    }

    getSegment(): Array<string> {
      let array = new Array<string>();
      let segments = this.uriEntry.getSegment();
      segments.forEach(
        (element: string, i: number) => {
          array.push(this.decodeSafelyInner(element));
        });
      return array;
    }

    clearQuery(): URI {
      let uriStr: string = this.uriEntry.clearQuery();
      return createNewUri(uriStr);
    }

    normalize(): URI {
      let uriStr: string = this.uriEntry.normalize();
      return createNewUri(uriStr);
    }

    get scheme(): string | null {
      if (this.uriEntry.getScheme() == '') {
        return null;
      }
      return this.uriEntry.getScheme();
    }

    get authority(): string | null {
      let thisAuthority: string = this.uriEntry.getAuthority();
      if (thisAuthority == '') {
        return null;
      }
      return this.dealDecode(thisAuthority, true);
    }

    get ssp(): string {
      return this.dealDecode(this.uriEntry.getSsp(), true)
    }

    private dealDecode(input: string, decode?: boolean): string {
      let index1 = input.indexOf('[');
      let result = "";
      if (index1 != -1) {
        let index2 = input.indexOf(']', index1);
        let split1 = input.substring(0, index1);
        let split2 = input.substring(index1 + 1, index2);
        let split3 = input.substring(index2 + 1);
        if (decode) {
          result = this.decodeSafelyInner(split1) + '[' + split2 + ']' + this.decodeSafelyInner(split3);
        } else {
          result = split1 + '[' + split2 + ']' + split3;
        }
      } else {
        result = decode ? this.decodeSafelyInner(input) : input;
      }
      return result;
    }

    get userInfo(): string | null {
      if (this.uriEntry.getUserinfo() == '') {
        return null;
      }
      return this.decodeSafelyInner(this.uriEntry.getUserinfo());
    }

    get host(): string | null {
      if (this.uriEntry.getHost() == '') {
        return null;
      }
      return this.uriEntry.getHost();
    }

    get port(): string {
      return Number.toString(this.uriEntry.getPort());
    }

    get path(): string | null {
      if (this.uriEntry.getPath() == '') {
        return null;
      }
      return this.decodeSafelyInner(this.uriEntry.getPath());
    }

    get query(): string | null {
      if (this.uriEntry.getQuery() == '') {
        return null;
      }
      return this.decodeSafelyInner(this.uriEntry.getQuery());
    }

    get fragment(): string | null {
      if (this.uriEntry.getFragment() == '') {
        return null;
      }
      return this.decodeSafelyInner(this.uriEntry.getFragment());
    }

    get encodedUserInfo(): string | null {
      if (this.uriEntry.getUserinfo() == '') {
        return null;
      }
      return this.uriEntry.getUserinfo();
    }

    get encodedPath(): string | null {
      if (this.uriEntry.getPath() == '') {
        return null;
      }
      return this.uriEntry.getPath();
    }

    get encodedQuery(): string | null {
      if (this.uriEntry.getQuery() == '') {
        return null;
      }
      return this.uriEntry.getQuery();
    }

    get encodedFragment(): string | null {
      if (this.uriEntry.getFragment() == '') {
        return null;
      }
      return this.uriEntry.getFragment();
    }

    get encodedAuthority(): string | null {
      let thisAuthority: string = this.uriEntry.getAuthority();
      if (thisAuthority == '') {
        return null;
      }
      return this.dealDecode(thisAuthority);
    }

    get encodedSSP(): string {
      let thisSsp: string = this.uriEntry.getSsp();
      return this.dealDecode(thisSsp);
    }

    set scheme(input: string | null) {
      if (input === null || input.length === 0) {
        return;
      }
      this.uriEntry.setScheme(input);
    }

    set path(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setPath(encodeURI(input));
    }

    /**
     * NOTE(zhangziye):#25267 Unable to use different getter/setter signature 
     */
    set ssp(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setSsp(encodeURI(input))
    }

    set authority(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setAuthority(encodeURI(input))
    }

    set userInfo(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setUserInfo(encodeURIComponent(input));
    }

    set query(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setQuery(encodeURIComponent(input));
    }

    set fragment(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setFragment(encodeURIComponent(input));
    }

    set encodedUserInfo(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setUserInfo(input);
    }

    set encodedPath(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setPath(input);
    }

    set encodedQuery(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setQuery(input);
    }

    set encodedFragment(input: string | null) {
      if (input === null) {
        return;
      }
      this.uriEntry.setFragment(input);
    }

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

  function toAscllString(uriStr: string): string {
    return encodeURI(uriStr)
      .replaceAll("%5B", '[')
      .replaceAll("%5D", ']')
      .replaceAll("%25", '%');
  }

  function createNewUri(uriStr: string): URI {
    return new URI(uriStr);
  }

  const MAX_BIT_SIZE: number = 128;

  class UriData {
    port: number = -1;
    scheme: string = "";
    userInfo: string = "";
    host: string = "";
    query: string = "";
    fragment: string = "";
    path: string = "";
    authority: string = "";
    SchemeSpecificPart: string = "";
  };

  export class UriEntry {
    uriData: UriData = new UriData();
    data: string = "";
    inputUri: string = "";
    errStr: string = "";

    constructor(input: string) {
      this.errStr = "";
      if (input == "") {
        this.errStr = "Uri is empty.";
        return;
      }
      this.inputUri = input;
      try {
        this.analysisUri();
      } catch (e) { }
    }

    private assignSchemeSpecificPart() {
      this.uriData.SchemeSpecificPart += this.data;
      if (this.uriData.query != "") {
        this.uriData.SchemeSpecificPart += "?";
        this.uriData.SchemeSpecificPart += this.uriData.query;
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
        this.uriData.path = this.data;
        this.assignSchemeSpecificPart();
        this.data = "";
      } else {
        this.assignSchemeSpecificPart();
        this.uriData.query = "";
        this.data = "";
      }
    }

    private checkCharacter(data: string, rule: boolean[], flag: boolean): boolean {
      let dataLen = data.length;
      for (let i = 0; i < dataLen; i++) {
        let charCode = data.charCodeAt(i);
        if (charCode >= 0 && charCode < MAX_BIT_SIZE) {
          if (!rule[charCode]) {
            return false;
          }
        } else if (!flag) {
          return false;
        }
      }
      return true;
    }

    private specialPath() {
      if (!this.checkCharacter(this.data, UriRule.rulePath, true)) {
        this.errStr = "SpecialPath does not conform to the rule.";
        return;
      }
      this.uriData.path = this.data;
      this.data = "";
    }

    private analysisFragment(pos: number) {
      if (pos == 0) {
        this.errStr = "#It can't be the first.";
        return;
      }
      let fragment = this.data.substring(pos + 1);
      if (!this.checkCharacter(fragment, UriRule.ruleUrlc, true)) {
        this.errStr = "Fragment does not conform to the rule.";
        return;
      }
      this.uriData.fragment = fragment;
      this.data = this.data.substring(0, pos);
    }

    private analysisQuery(pos: number) {
      let query = this.data.substring(pos + 1);
      if (!this.checkCharacter(query, UriRule.ruleUrlc, true)) {
        this.errStr = "Query does not conform to the rule.";
        return;
      }
      this.uriData.query = query;
      this.data = this.data.substring(0, pos);
    }

    private analysisScheme(pos: number) {
      let slashPos = this.data.indexOf('/');
      if (slashPos != -1 && slashPos < pos) {
        this.specialPath();
        this.uriData.SchemeSpecificPart += (this.uriData.path);
        this.uriData.SchemeSpecificPart += ("?");
        this.uriData.SchemeSpecificPart += (this.uriData.query);
        this.data = "";
      } else {
        let code = this.data.charCodeAt(0);
        if (code >= 0 && code < MAX_BIT_SIZE &&
          !UriRule.ruleAlpha[code]) {
          this.errStr = "Scheme the first character must be a letter.";
          return;
        }
        let scheme = this.data.substring(0, pos);
        if (!this.checkCharacter(scheme, UriRule.ruleScheme, false)) {
          this.errStr = "Scheme does not conform to the rule.";
          return;
        }
        this.uriData.scheme = scheme;
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
          this.uriData.port = -1;
          this.uriData.host = "";
          this.uriData.userInfo = "";
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
      this.uriData.authority = this.data;
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
      if (!this.checkCharacter(path, UriRule.rulePath, true)) {
        this.errStr = "Path does not conform to the rule.";
        return;
      }
      this.uriData.path = path;
      this.data = this.data.substring(0, pos);
    }

    private analysisUserInfo(pos: number) {
      let userInfo = this.data.substring(0, pos);
      if (!this.checkCharacter(userInfo, UriRule.ruleUserInfo, true)) {
        this.errStr = "UserInfo does not conform to the rule.";
        return;
      }
      this.uriData.userInfo = userInfo;
      this.data = this.data.substring(pos + 1);
    }

    private analysisPort(pos: number) {
      let port = this.data.substring(pos + 1);
      if (!this.checkCharacter(port, UriRule.rulePort, true)) {
        this.errStr = "Port does not conform to the rule.";
        return false;
      } else if (this.checkCharacter(port, UriRule.ruleDigit, false)) {
        // 10:The maximum number of bits for int value
        if (port.length == 0 || port.length > 10) {
          return false;
        }
        let tempPort = new Number(port);
        if (tempPort < 0 || tempPort > Int.MAX_VALUE) {
          return false;
        }
        this.uriData.port = tempPort;
        this.data = this.data.substring(0, pos);
        return true;
      } else {
        this.data = this.data.substring(0, pos);
        return false;
      }
    }

    private analysisIPV4() {
      /**
       * 正则图解析 https://www.jyshare.com/front-end/7625
         regex_match默认使用的是完全匹配模式    js需要要加上^$
      */
      let ipv4 = new RegExp("^((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)$");
      let hostname = new RegExp("^(([a-zA-Z0-9]([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9])?\\.)+([a-zA-Z]"
        + "([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s])?))$|^([a-zA-Z0-9]([a-zA-Z0-9\\-~_|\\+{}!$&=,;:'()\\*\\s]*[a-zA-Z0-9])?)$");
      let isIpv4 = ipv4.test(this.data);
      let isHosName = hostname.test(this.data);
      if (!isIpv4 && !isHosName) {
        return false;
      } else {
        this.uriData.host = this.data;
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
      this.uriData.host = this.data;
      this.data = "";
    }

    equals(other: UriEntry): boolean {
      if (this.uriData.port != other.uriData.port) {
        return false;
      }
      if (this.uriData.scheme != other.uriData.scheme) {
        return false;
      }
      if (this.uriData.userInfo != other.uriData.userInfo) {
        return false;
      }
      if (this.uriData.host != other.uriData.host) {
        return false;
      }
      if (this.uriData.query != other.uriData.query) {
        return false;
      }
      if (this.uriData.fragment != other.uriData.fragment) {
        return false;
      }
      if (this.uriData.path != other.uriData.path) {
        return false;
      }
      if (this.uriData.authority != other.uriData.authority) {
        return false;
      }
      if (this.uriData.SchemeSpecificPart != other.uriData.SchemeSpecificPart) {
        return false;
      }
      return true;
    }

    toString(): string {
      return this.inputUri;
    }

    isAbsolute(): boolean {
      return this.uriData.scheme != "";
    }

    isRelative(): boolean {
      return this.uriData.scheme == "";
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
      if (this.uriData.scheme != "") {
        result += this.uriData.scheme + ":";
      }
      if (this.uriData.authority != "") {
        result += "//" + this.uriData.authority;
      }
      if (this.uriData.path != "") {
        result += this.uriData.path;
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
        if (this.uriData.query == "") {
          if (str == "query") {
            result += "?" + param;
          }
        } else {
          result += "?" + this.uriData.query;
          if (str == "query") {
            result += "&" + param;
          }
        }
      }
      if (this.uriData.fragment != "") {
        result += "#" + this.uriData.fragment;
      }
      return result;
    }

    getSegment(): Array<string> {
      let segments = new Array<string>();
      if (this.uriData.path == "") {
        return segments;
      }
      let previous = 0;
      let current = 0;
      for (current = this.uriData.path.indexOf('/', previous); current != -1;
        current = this.uriData.path.indexOf('/', previous)) {
        if (previous < current) {
          let segment = this.uriData.path.substring(previous, current);
          segments.push(segment);
        }
        previous = current + 1;
      }
      if (previous < this.uriData.path.length) {
        segments.push(this.uriData.path.substring(previous));
      }
      return segments;
    }

    getErrStr(): string {
      return this.errStr;
    }

    private checkErrAndThrow() {
      if (this.errStr != "") {
        throw new Error();
      }
    }

    normalize(): string {
      let temp = new Array<string>;
      let pathLen = this.uriData.path.length;
      if (pathLen == 0) {
        return this.inputUri;
      }
      let pos = 0;
      let left = 0;
      while ((pos = this.uriData.path.indexOf('/', left)) != -1) {
        temp.push(this.uriData.path.substring(left, pos));
        left = pos + 1;
      }
      if (left != pathLen) {
        temp.push(this.uriData.path.substring(left));
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
      if (this.uriData.scheme != "") {
        normalizeUri += this.uriData.scheme + ":";
      }
      if (this.uriData.path == "") {
        normalizeUri += this.uriData.SchemeSpecificPart;
      } else {
        if (this.uriData.host != "") {
          normalizeUri += "//";
          if (this.uriData.userInfo != "") {
            normalizeUri += this.uriData.userInfo + "@";
          }
          normalizeUri += this.uriData.host;
          if (this.uriData.port != -1) {
            normalizeUri += ":" + Number.toString(this.uriData.port);
          }
        } else if (this.uriData.authority != "") {
          normalizeUri += "//" + this.uriData.authority;
        }
        normalizeUri += path;
      }
      if (this.uriData.query != "") {
        normalizeUri += "?" + this.uriData.query;
      }
      if (this.uriData.fragment != "") {
        normalizeUri += '#' + this.uriData.fragment;
      }
      return normalizeUri;
    }

    getScheme() {
      return this.uriData.scheme;
    }

    getAuthority() {
      return this.uriData.authority;
    }

    getSsp() {
      return this.uriData.SchemeSpecificPart;
    }

    getUserinfo() {
      return this.uriData.userInfo;
    }

    getHost() {
      return this.uriData.host;
    }

    getPort() {
      return this.uriData.port;
    }

    getPath() {
      return this.uriData.path;
    }

    getQuery() {
      return this.uriData.query;
    }

    getFragment() {
      return this.uriData.fragment;
    }

    clearQuery() {
      return this.buildUriString("clearquery", "");
    }

    setScheme(scheme: string): void {
      let code = scheme.charCodeAt(0);
      if (code >= 0 && code < MAX_BIT_SIZE &&
        !UriRule.ruleAlpha[code]) {
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

      if (!this.checkCharacter(temp, UriRule.ruleScheme, false)) {
        this.errStr = "Scheme does not conform to the rule.";
        return;
      }
      this.uriData.scheme = temp;
      this.inputUri = this.updateToString();
    }

    setAuthority(authorityStr: string): void {
      // Reset values
      this.uriData.port = -1;
      this.uriData.host = "";
      this.uriData.userInfo = "";
      this.uriData.authority = authorityStr;
      this.data = authorityStr;

      // Find UserInfo
      const atPos = this.data.indexOf('@');
      if (atPos !== -1) {
        const userStr = this.data.substring(0, atPos);
        this.data = this.data.substring(atPos + 1);

        if (!this.checkCharacter(userStr, UriRule.ruleUserInfo, true)) {
          this.errStr = "userInfo does not conform to the rule";
          return;
        }
        this.uriData.userInfo = userStr;
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
      this.uriData.authority = "";
      this.uriData.port = -1;
      this.uriData.host = "";
      this.uriData.userInfo = "";
      this.uriData.query = "";
      this.uriData.path = "";
      this.uriData.SchemeSpecificPart = "";
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
        this.uriData.path = this.data;
        this.assignSchemeSpecificPart();
        this.data = "";
      } else {
        this.assignSchemeSpecificPart();
        this.uriData.path = "";
        this.uriData.query = "";
        this.data = "";
      }
      this.inputUri = this.updateToString();
    }

    setUserInfo(userInfo: string): void {
      if (!this.uriData.host || !this.checkCharacter(userInfo, UriRule.ruleUserInfo, true)) {
        this.errStr = "userInfo does not conform to the rule";
        return;
      }
      this.uriData.userInfo = userInfo;
      this.updateAuthority();
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setPath(pathStr: string): void {
      if (!this.checkCharacter(pathStr, UriRule.rulePath, true)) {
        this.errStr = "pathStr does not conform to the rule";
        return;
      }
      this.uriData.path = pathStr;
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setQuery(queryStr: string): void {
      if (!this.checkCharacter(queryStr, UriRule.ruleUrlc, true)) {
        this.errStr = "QueryStr does not conform to the rule";
        return;
      }
      this.uriData.query = queryStr;
      this.updateSsp();
      this.inputUri = this.updateToString();
    }

    setFragment(fragmentStr: string): void {
      if (!this.checkCharacter(fragmentStr, UriRule.ruleUrlc, true)) {
        this.errStr = "Fragment does not conform to the rule";
        return;
      }
      this.uriData.fragment = fragmentStr;
      this.inputUri = this.updateToString();
    }

    private updateAuthority(): void {
      let temp = this.uriData.userInfo;
      if (this.uriData.userInfo) {
        temp += "@";
      }
      temp += this.uriData.host;
      if (this.uriData.port !== -1) {
        temp += `:${this.uriData.port}`;
      }
      this.uriData.authority = temp;
    }

    private updateSsp(): void {
      let temp = "";
      if (this.uriData.authority) {
        temp += `//${this.uriData.authority}`;
      }
      if (this.uriData.path) {
        temp += this.uriData.path.startsWith('/')
          ? this.uriData.path
          : `/${this.uriData.path}`;
      }
      if (this.uriData.query) {
        temp += `?${this.uriData.query}`;
      }
      this.uriData.SchemeSpecificPart = temp;
    }

    private updateToString(): string {
      let uriStr = "";
      let addQuery = false;
      if (this.uriData.scheme) {
        uriStr += this.uriData.scheme + ":";
      }
      if (!this.uriData.path) {
        uriStr += this.uriData.SchemeSpecificPart;
        addQuery = true;
      } else {
        if (this.uriData.host) {
          uriStr += "//";
          if (this.uriData.userInfo) {
            uriStr += this.uriData.userInfo + "@";
          }
          uriStr += this.uriData.host;
          if (this.uriData.port !== -1) {
            uriStr += ":" + this.uriData.port;
          }
        } else if (this.uriData.authority) {
          uriStr += "//" + this.uriData.authority;
        }
        if (this.uriData.path) {
          uriStr += this.uriData.path.startsWith('/')
            ? this.uriData.path
            : "/" + this.uriData.path;
        }
      }
      if (this.uriData.query && !addQuery) {
        uriStr += "?" + this.uriData.query;
      }

      if (this.uriData.fragment) {
        uriStr += "#" + this.uriData.fragment;
      }
      return uriStr;
    }
  }

  class UriRule {
    static ruleAlpha = new boolean[MAX_BIT_SIZE];
    static ruleScheme = new boolean[MAX_BIT_SIZE];
    static ruleUrlc = new boolean[MAX_BIT_SIZE];
    static rulePath = new boolean[MAX_BIT_SIZE];
    static ruleUserInfo = new boolean[MAX_BIT_SIZE];
    static ruleDigit = new boolean[MAX_BIT_SIZE];
    static rulePort = new boolean[MAX_BIT_SIZE];
    static {
      let digitAggregate = "0123456789";
      UriRule.setBit(UriRule.ruleDigit, digitAggregate);

      let alphasAggregate = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      UriRule.setBit(UriRule.ruleAlpha, alphasAggregate);

      let schemeAggregate = digitAggregate + alphasAggregate + "+-.| _-~!$&=,;'(){}*";
      UriRule.setBit(UriRule.ruleScheme, schemeAggregate);

      let uricAggregate = schemeAggregate + ";/?:@&=$,[]_!~*'()%\"";
      UriRule.setBit(UriRule.ruleUrlc, uricAggregate);

      let pathAggregate = schemeAggregate + ";/:@&=$,_!~*'()%";
      UriRule.setBit(UriRule.rulePath, pathAggregate);

      let userInfoAggregate = schemeAggregate + ";:&=$,_!~*'()%";
      UriRule.setBit(UriRule.ruleUserInfo, userInfoAggregate);

      let portAggregate = digitAggregate + alphasAggregate + ".:@-;&=+$,-_!~*'()";
      UriRule.setBit(UriRule.rulePort, portAggregate);
    }

    private static setBit(arr: boolean[], str: string) {
      for (let i = 0; i < arr.length; i++) {
        arr[i] = false;
      }
      for (let i = 0; i < str.length; i++) {
        let code = str.charCodeAt(i);
        arr[code] = true;
      }
    }
  }
}
