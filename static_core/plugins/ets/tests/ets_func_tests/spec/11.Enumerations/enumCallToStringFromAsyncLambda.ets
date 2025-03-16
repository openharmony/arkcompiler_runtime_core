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

/*---
desc: 11 Enumeration
name: 11.enumeration/11.2.enumeration_string_values
tags: []
---*/

type fooLambda = (arg: string) => Promise<string>

enum Enum1 {
    big_string = "mqpfcmvzctncgcjdjhxcghwcgnrxvxtuxnifrlumwpoukpezhupzbavqxkdwqkfviobuhjcnlajrjiarvboeatxpwuvbqwnbsngvxaux \
        gtyustiddvloyzddxuhafybdxkhovuswyzwtlzdvwwywilbqerbcvgfhbzowtvrmzagszlptlooopxdyrrzlpvjpgcnoldubdnhytftejlk \
        xiolxnojsoxmqwmxnttwaseebvtaqdtlizrxenpgxaipgmkvxmzakqsekqsypyccmgnwtvmltvepyeqjarfwgyxovoblgowqmwxcqjfoedttg \
        uueabuuabnmigvwqikokrwtkxsejrabhavwhnziitxdpqtiqrdlnqjvlazmhdlqhpswzroaialeytejnnfayafnhqguhbkpkjfcshggxlpudwapvet \
        nxzvtdfjdqdowivactmssjqkplmbmuzglhzqjsyzyagnpyiqsbghalglvdrmxzrnrwaknwbefebesrghmdqjoizgytevuxhgwlzpqaykgewzeyhylmiwznl \
        dnxlumkdlsuinwyjfzhhhcmfpojbzkyckwisrjcwrahhgouuqbxcnhdmjivcfckkaijrdlqrnripxhaiobvntvvnjvyzsrytbmlceiebgcqnawrnjgyurkohz \
        fpypbihbrjwqllxcwixqqhwoxkwwnmwrzxjryzkypxbpfuyutmluhgaqokkhryekpqnjxtftclaqkkjyhhnynpvdebjczvljykeylvaoarczedlerefjdajwbzy \
        ialonxdjbxpyjlujsocdmyeashsjfyirxqfegrzqrddazqjoxdmicesucznmetesyydytvnbzimbjzgooxitkkyknabncpilukmseddnimutiusxwwfrnvymcc \
        xnqgapdlfdwmhjnznidumclvsslzjrqaiaczecvndsezechbeverbuzzmlecvtgqsfniavsstwqclmmyp \
        mqpfcmvzctncgcjdjhxcghwcgnrxvxtuxnifrlumwpoukpezhupzbavqxkdwqkfviobuhjcnlajrjiarvboeatxpwuvbqwnbsngvxaux \
        gtyustiddvloyzddxuhafybdxkhovuswyzwtlzdvwwywilbqerbcvgfhbzowtvrmzagszlptlooopxdyrrzlpvjpgcnoldubdnhytftejlk \
        xiolxnojsoxmqwmxnttwaseebvtaqdtlizrxenpgxaipgmkvxmzakqsekqsypyccmgnwtvmltvepyeqjarfwgyxovoblgowqmwxcqjfoedttg \
        uueabuuabnmigvwqikokrwtkxsejrabhavwhnziitxdpqtiqrdlnqjvlazmhdlqhpswzroaialeytejnnfayafnhqguhbkpkjfcshggxlpudwapvet \
        nxzvtdfjdqdowivactmssjqkplmbmuzglhzqjsyzyagnpyiqsbghalglvdrmxzrnrwaknwbefebesrghmdqjoizgytevuxhgwlzpqaykgewzeyhylmiwznl \
        dnxlumkdlsuinwyjfzhhhcmfpojbzkyckwisrjcwrahhgouuqbxcnhdmjivcfckkaijrdlqrnripxhaiobvntvvnjvyzsrytbmlceiebgcqnawrnjgyurkohz \
        fpypbihbrjwqllxcwixqqhwoxkwwnmwrzxjryzkypxbpfuyutmluhgaqokkhryekpqnjxtftclaqkkjyhhnynpvdebjczvljykeylvaoarczedlerefjdajwbzy \
        ialonxdjbxpyjlujsocdmyeashsjfyirxqfegrzqrddazqjoxdmicesucznmetesyydytvnbzimbjzgooxitkkyknabncpilukmseddnimutiusxwwfrnvymcc \
        xnqgapdlfdwmhjnznidumclvsslzjrqaiaczecvndsezechbeverbuzzmlecvtgqsfniavsstwqclmmyp"
}

function getString(arg: string): string {
    return arg;
}

function main(): void {
    let expected_big_string = "mqpfcmvzctncgcjdjhxcghwcgnrxvxtuxnifrlumwpoukpezhupzbavqxkdwqkfviobuhjcnlajrjiarvboeatxpwuvbqwnbsngvxaux \
        gtyustiddvloyzddxuhafybdxkhovuswyzwtlzdvwwywilbqerbcvgfhbzowtvrmzagszlptlooopxdyrrzlpvjpgcnoldubdnhytftejlk \
        xiolxnojsoxmqwmxnttwaseebvtaqdtlizrxenpgxaipgmkvxmzakqsekqsypyccmgnwtvmltvepyeqjarfwgyxovoblgowqmwxcqjfoedttg \
        uueabuuabnmigvwqikokrwtkxsejrabhavwhnziitxdpqtiqrdlnqjvlazmhdlqhpswzroaialeytejnnfayafnhqguhbkpkjfcshggxlpudwapvet \
        nxzvtdfjdqdowivactmssjqkplmbmuzglhzqjsyzyagnpyiqsbghalglvdrmxzrnrwaknwbefebesrghmdqjoizgytevuxhgwlzpqaykgewzeyhylmiwznl \
        dnxlumkdlsuinwyjfzhhhcmfpojbzkyckwisrjcwrahhgouuqbxcnhdmjivcfckkaijrdlqrnripxhaiobvntvvnjvyzsrytbmlceiebgcqnawrnjgyurkohz \
        fpypbihbrjwqllxcwixqqhwoxkwwnmwrzxjryzkypxbpfuyutmluhgaqokkhryekpqnjxtftclaqkkjyhhnynpvdebjczvljykeylvaoarczedlerefjdajwbzy \
        ialonxdjbxpyjlujsocdmyeashsjfyirxqfegrzqrddazqjoxdmicesucznmetesyydytvnbzimbjzgooxitkkyknabncpilukmseddnimutiusxwwfrnvymcc \
        xnqgapdlfdwmhjnznidumclvsslzjrqaiaczecvndsezechbeverbuzzmlecvtgqsfniavsstwqclmmyp \
        mqpfcmvzctncgcjdjhxcghwcgnrxvxtuxnifrlumwpoukpezhupzbavqxkdwqkfviobuhjcnlajrjiarvboeatxpwuvbqwnbsngvxaux \
        gtyustiddvloyzddxuhafybdxkhovuswyzwtlzdvwwywilbqerbcvgfhbzowtvrmzagszlptlooopxdyrrzlpvjpgcnoldubdnhytftejlk \
        xiolxnojsoxmqwmxnttwaseebvtaqdtlizrxenpgxaipgmkvxmzakqsekqsypyccmgnwtvmltvepyeqjarfwgyxovoblgowqmwxcqjfoedttg \
        uueabuuabnmigvwqikokrwtkxsejrabhavwhnziitxdpqtiqrdlnqjvlazmhdlqhpswzroaialeytejnnfayafnhqguhbkpkjfcshggxlpudwapvet \
        nxzvtdfjdqdowivactmssjqkplmbmuzglhzqjsyzyagnpyiqsbghalglvdrmxzrnrwaknwbefebesrghmdqjoizgytevuxhgwlzpqaykgewzeyhylmiwznl \
        dnxlumkdlsuinwyjfzhhhcmfpojbzkyckwisrjcwrahhgouuqbxcnhdmjivcfckkaijrdlqrnripxhaiobvntvvnjvyzsrytbmlceiebgcqnawrnjgyurkohz \
        fpypbihbrjwqllxcwixqqhwoxkwwnmwrzxjryzkypxbpfuyutmluhgaqokkhryekpqnjxtftclaqkkjyhhnynpvdebjczvljykeylvaoarczedlerefjdajwbzy \
        ialonxdjbxpyjlujsocdmyeashsjfyirxqfegrzqrddazqjoxdmicesucznmetesyydytvnbzimbjzgooxitkkyknabncpilukmseddnimutiusxwwfrnvymcc \
        xnqgapdlfdwmhjnznidumclvsslzjrqaiaczecvndsezechbeverbuzzmlecvtgqsfniavsstwqclmmyp";

    let stringPromise: fooLambda = async (arg: string): Promise<string> => {
        arg = Enum1.big_string.toString();
        return Promise.resolve<string>(getString(arg));
    };

    let big_string_from_enum: string = await stringPromise();

    assertEQ(expected_big_string, big_string_from_enum)
}
