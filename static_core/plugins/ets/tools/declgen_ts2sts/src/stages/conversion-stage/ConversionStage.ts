/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import * as ts from 'typescript';
import { StageOptions, TransformationStage } from '../Stage';
import { TraverserState } from '../Traverser';
import { PreScan, ConversionLocalState } from './PreScan';
import { TypeConversionTraverser } from './TypeConversionTraverser';

type PrevState = undefined;
type NextState = undefined;

export class ConversionStage extends TransformationStage<PrevState, NextState, ConversionLocalState> {
  constructor(removeReservedKeywordIdentifier: boolean, options?: StageOptions) {
    const BoundTypeConversionTraverser = class extends TypeConversionTraverser {
      constructor(
        context: ts.TransformationContext,
        typeChecker: ts.TypeChecker,
        state: TraverserState<PrevState, ConversionLocalState>
      ) {
        super(removeReservedKeywordIdentifier, context, typeChecker, state);
      }
    };

    super(
      [PreScan, BoundTypeConversionTraverser],
      () => ({
        existingTypeNames: new Set<string>(),
        pendingDefaultImports: new Map<string, string>(),
        defaultAccessMap: new Map<string, string>()
      }),
      () => undefined,
      false,
      options
    );
  }
}
