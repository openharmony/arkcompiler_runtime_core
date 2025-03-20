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

import stylistic from '@stylistic/eslint-plugin';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import js from '@eslint/js';
import { FlatCompat } from '@eslint/eslintrc';
import { includeIgnoreFile } from '@eslint/compat';
import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const compat = new FlatCompat({
  baseDirectory: __dirname,
  recommendedConfig: js.configs.recommended,
  allConfig: js.configs.all
});
const gitignorePath = path.resolve(__dirname, '.gitignore');

export default tseslint.config(
  {
    ignores: [
      'bin/**/*',
      'build/**/*',
      'bundle/**/*',
      'dist/**/*',
      'docs/**/*',
      'node_modules/**/*',
      'scripts/**/*',
      'test/**/*',
      '**/**.json',
      '**/**.js'
    ]
  },
  includeIgnoreFile(gitignorePath),
  {
    files: ['**/*.ts'],
    extends: [eslint.configs.recommended, tseslint.configs.recommended],
    plugins: {
      '@typescript-eslint': tseslint.plugin,
      '@stylistic': stylistic
    },

    languageOptions: {
      parser: tseslint.parser,
      ecmaVersion: 'latest',
      sourceType: 'module',
      parserOptions: {
        project: './tsconfig.json',
        tsconfigRootDir: import.meta.dirname
      }
    },

    rules: {
      'arrow-body-style': ['error', 'always'],
      camelcase: 'off',

      'class-methods-use-this': [
        'error',
        {
          exceptMethods: [],
          enforceForClassFields: true
        }
      ],

      complexity: [
        'error',
        {
          max: 15
        }
      ],

      'consistent-return': [
        'error',
        {
          treatUndefinedAsUnspecified: false
        }
      ],

      curly: ['error', 'all'],
      'dot-notation': 'error',
      eqeqeq: ['error', 'always'],

      'max-depth': [
        'error',
        {
          max: 4
        }
      ],

      'multiline-comment-style': ['error', 'starred-block'],
      '@stylistic/no-confusing-arrow': 'error',

      'no-else-return': [
        'error',
        {
          allowElseIf: true
        }
      ],

      'no-extra-bind': 'error',
      '@stylistic/no-floating-decimal': 'error',
      'no-lonely-if': 'error',
      'no-unneeded-ternary': 'error',
      'no-useless-return': 'error',
      'no-var': 'error',
      'prefer-const': 'error',
      'spaced-comment': ['error', 'always'],
      'one-var': ['error', 'never'],

      'max-lines-per-function': [
        'error',
        {
          max: 50
        }
      ],

      '@stylistic/array-bracket-newline': ['error', 'consistent'],
      '@stylistic/array-bracket-spacing': ['error', 'never'],
      '@stylistic/array-element-newline': ['error', 'consistent'],
      '@stylistic/arrow-parens': ['error', 'always'],

      '@stylistic/arrow-spacing': [
        'error',
        {
          before: true,
          after: true
        }
      ],

      '@stylistic/block-spacing': ['error', 'always'],

      '@stylistic/brace-style': [
        'error',
        '1tbs',
        {
          allowSingleLine: false
        }
      ],

      '@stylistic/comma-dangle': [
        'error',
        {
          arrays: 'never',
          objects: 'never',
          imports: 'never',
          exports: 'never',
          functions: 'never'
        }
      ],

      '@stylistic/comma-spacing': [
        'error',
        {
          before: false,
          after: true
        }
      ],

      '@stylistic/comma-style': ['error', 'last'],

      '@stylistic/computed-property-spacing': [
        'error',
        'never',
        {
          enforceForClassMembers: true
        }
      ],

      '@stylistic/dot-location': ['error', 'object'],
      '@stylistic/eol-last': ['error', 'always'],
      '@stylistic/func-call-spacing': ['error', 'never'],
      '@stylistic/function-call-argument-newline': ['error', 'consistent'],
      '@stylistic/function-paren-newline': ['error', 'consistent'],

      '@stylistic/generator-star-spacing': [
        'error',
        {
          before: true,
          after: false
        }
      ],

      '@stylistic/implicit-arrow-linebreak': ['error', 'beside'],

      '@stylistic/indent': [
        'error',
        2,
        {
          ignoredNodes: [],
          SwitchCase: 1,
          VariableDeclarator: 1,
          outerIIFEBody: 1,
          MemberExpression: 1,

          FunctionDeclaration: {
            parameters: 1,
            body: 1
          },

          FunctionExpression: {
            parameters: 1,
            body: 1
          },

          CallExpression: {
            arguments: 1
          },

          ArrayExpression: 1,
          ObjectExpression: 1,
          ImportDeclaration: 1,
          flatTernaryExpressions: true,
          offsetTernaryExpressions: false,
          ignoreComments: false
        }
      ],

      '@stylistic/jsx-quotes': ['error', 'prefer-double'],

      '@stylistic/keyword-spacing': [
        'error',
        {
          before: true,
          after: true
        }
      ],

      'line-comment-position': [
        'error',
        {
          position: 'above'
        }
      ],

      '@stylistic/linebreak-style': ['error', 'unix'],

      '@stylistic/lines-around-comment': [
        'error',
        {
          beforeBlockComment: true
        }
      ],

      '@stylistic/lines-between-class-members': [
        'error',
        {
          enforce: [
            {
              blankLine: 'always',
              prev: '*',
              next: 'method'
            },
            {
              blankLine: 'always',
              prev: 'method',
              next: '*'
            }
          ]
        }
      ],

      '@stylistic/max-len': [
        'error',
        {
          code: 120,
          tabWidth: 2,
          ignoreComments: true,
          ignoreStrings: true
        }
      ],

      '@stylistic/max-statements-per-line': [
        'error',
        {
          max: 1
        }
      ],

      '@stylistic/multiline-ternary': ['error', 'always-multiline'],
      '@stylistic/new-parens': ['error', 'always'],
      '@stylistic/newline-per-chained-call': ['off'],
      '@stylistic/no-extra-parens': ['error', 'all'],
      '@stylistic/no-mixed-spaces-and-tabs': 'error',
      '@stylistic/no-multi-spaces': 'error',

      '@stylistic/no-multiple-empty-lines': [
        'error',
        {
          max: 2,
          maxEOF: 1
        }
      ],

      '@stylistic/no-tabs': 'error',

      '@stylistic/no-trailing-spaces': [
        'error',
        {
          skipBlankLines: false,
          ignoreComments: false
        }
      ],

      '@stylistic/no-whitespace-before-property': 'error',
      '@stylistic/nonblock-statement-body-position': ['error', 'beside'],

      '@stylistic/object-curly-newline': [
        'error',
        {
          consistent: true
        }
      ],

      '@stylistic/object-curly-spacing': ['error', 'always'],
      '@stylistic/operator-linebreak': ['error', 'after'],
      '@stylistic/padded-blocks': 'off',
      '@stylistic/quotes': ['error', 'single'],
      '@stylistic/rest-spread-spacing': ['error', 'never'],
      '@stylistic/semi': ['error', 'always'],

      '@stylistic/semi-spacing': [
        'error',
        {
          before: false,
          after: true
        }
      ],

      '@stylistic/semi-style': ['error', 'last'],
      '@stylistic/space-before-blocks': ['error', 'always'],
      '@stylistic/space-before-function-paren': ['error', 'never'],
      '@stylistic/space-in-parens': ['error', 'never'],
      '@stylistic/space-infix-ops': ['error'],

      '@stylistic/space-unary-ops': [
        'error',
        {
          words: true,
          nonwords: false,
          overrides: {}
        }
      ],

      '@stylistic/switch-colon-spacing': [
        'error',
        {
          after: true,
          before: false
        }
      ],

      '@stylistic/template-curly-spacing': ['error', 'never'],
      '@stylistic/template-tag-spacing': ['error', 'never'],
      'unicode-bom': ['error', 'never'],
      '@stylistic/wrap-iife': ['error', 'outside'],
      '@stylistic/wrap-regex': 'error',

      '@stylistic/yield-star-spacing': [
        'error',
        {
          before: true,
          after: false
        }
      ],

      '@typescript-eslint/explicit-function-return-type': 'error',
      '@typescript-eslint/adjacent-overload-signatures': 'error',

      '@typescript-eslint/explicit-member-accessibility': [
        'error',
        {
          accessibility: 'no-public'
        }
      ],

      '@typescript-eslint/method-signature-style': ['error', 'method'],
      '@typescript-eslint/no-confusing-non-null-assertion': 'error',
      '@typescript-eslint/no-confusing-void-expression': 'error',
      '@typescript-eslint/no-explicit-any': 'warn',
      '@typescript-eslint/no-extra-non-null-assertion': 'error',
      '@typescript-eslint/no-meaningless-void-operator': 'error',
      '@typescript-eslint/no-unnecessary-boolean-literal-compare': 'error',
      '@typescript-eslint/no-unnecessary-condition': 'off',
      '@typescript-eslint/no-unnecessary-type-assertion': 'error',
      '@typescript-eslint/prefer-as-const': 'error',
      '@typescript-eslint/prefer-optional-chain': 'error',
      '@typescript-eslint/prefer-readonly': 'error',
      '@typescript-eslint/consistent-type-imports': 'error',

      '@typescript-eslint/naming-convention': [
        'error',
        {
          selector: 'typeLike',
          format: ['PascalCase']
        }
      ]
    }
  }
);
