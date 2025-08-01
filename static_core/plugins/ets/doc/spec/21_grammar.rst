..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Grammar Summary:

Grammar Summary
###############

.. code-block:: abnf

    literal: Literal;

    identifier: Identifier;

    indexType: 'number';

    type:
        annotationUsage?
        ( typeReference
        | 'readonly'? arrayType
        | 'readonly'? tupleType
        | functionType
        | functionTypeWithReceiver
        | unionType
        | keyofType
        | StringLiteral
        )
        | '(' type ')'
        ;

    typeReference:
        typeReferencePart ('.' typeReferencePart)*
        |  identifier '!'
        ;

    typeReferencePart:
        identifier typeArguments?
        ;

    arrayType:
       type '[' ']'
       ;

    functionType:
        '(' ftParameterList? ')' ftReturnType
        ;

    ftParameterList:
        ftParameter (',' ftParameter)* (',' ftRestParameter)?
        | ftRestParameter
        ;

    ftParameter:
        identifier ('?')? ':' type
        ;

    ftRestParameter:
        '...' ftParameter
        ;

    ftReturnType:
        '=>' type
        ;

    tupleType:
        '[' (type (',' type)* ','?)? ']'
        ;

    unionType:
        type ('|' type)*
        ;

    keyofType:
        'keyof' typeReference
        ;

    qualifiedName:
      identifier ('.' identifier )*
      ;

    typeDeclaration:
        classDeclaration
        | interfaceDeclaration
        | enumDeclaration
        | typeAlias
        ;

    typeAlias:
        'type' identifier typeParameters? '=' type
        ;

    variableDeclarations:
        'let' variableDeclarationList
        ;

    variableDeclarationList:
        variableDeclaration (',' variableDeclaration)*
        ;

    variableDeclaration:
        identifier ':' type initializer?
        | identifier initializer
        ;

    initializer:
        '=' expression
        ;

    constantDeclarations:
        'const' constantDeclarationList
        ;

    constantDeclarationList:
        constantDeclaration (',' constantDeclaration)*
        ;

    constantDeclaration:
        identifier (':' type)? initializer
        ;

    functionDeclaration:
        modifiers? 'function' identifier
        typeParameters? signature block?
        ;

    modifiers:
        'native' | 'async'
        ;

    signature:
        '(' parameterList? ')' returnType?
        ;

    returnType:
        ':' (type | 'this')
        ;

    parameterList:
        parameter (',' parameter)* (',' restParameter)? ','?
        | restParameter ','?
        ;

    parameter:
        annotationUsage? (requiredParameter | optionalParameter)
        ;

    requiredParameter:
        identifier ':' type
        ;

    optionalParameter:
        identifier ':' type '=' expression
        | identifier '?' ':' type
        ;

    restParameter:
        annotationUsage? '...' identifier ':' type
        ;

    typeParameters:
        '<' typeParameterList '>'
        ;

    typeParameterList:
        typeParameter (',' typeParameter)*
        ;

    typeParameter:
        ('in' | 'out')? identifier constraint? typeParameterDefault?
        ;

    constraint:
        'extends' type
        ;

    typeParameterDefault:
        '=' typeReference ('[]')?
        ;

    typeArguments:
        '<' type (',' type)* '>'
        ;

    expression:
        primaryExpression
        | castExpression
        | instanceOfExpression
        | typeOfExpression
        | nullishCoalescingExpression
        | spreadExpression
        | unaryExpression
        | binaryExpression
        | assignmentExpression
        | conditionalExpression
        | stringInterpolation
        | lambdaExpression
        | lambdaExpressionWithReceiver
        | launchExpression
        | awaitExpression
        ;

    primaryExpression:
        literal
        | namedReference
        | arrayLiteral
        | objectLiteral
        | recordLiteral
        | thisExpression
        | parenthesizedExpression
        | methodCallExpression
        | fieldAccessExpression
        | indexingExpression
        | functionCallExpression
        | newExpression
        | ensureNotNullishExpression
        ;

    binaryExpression:
        multiplicativeExpression
        | additiveExpression
        | shiftExpression
        | relationalExpression
        | equalityExpression
        | bitwiseAndLogicalExpression
        | conditionalAndExpression
        | conditionalOrExpression
        ;

    objectReference:
        typeReference
        | 'super'
        | primaryExpression
        ;

    arguments:
        '(' argumentSequence? ')'
        ;

    argumentSequence:
        restArgument
        | expression (',' expression)* (',' restArgument)? ','?
        ;

    restArgument:
        '...'? expression
        ;

    namedReference:
      qualifiedName typeArguments?
      ;

    arrayLiteral:
        '[' expressionSequence? ']'
        ;

    expressionSequence:
        expression (',' expression)* ','?
        ;

    objectLiteral:
       '{' objectLiteralMembers? '}'
       ;

    objectLiteralMembers:
       objectLiteralMember (',' objectLiteralMember)* ','?
       ;

    objectLiteralMember:
       objectLiteralField | objectLiteralMethod
       ;

    objectLiteralField:
       identifier ':' expression
       ;

    objectLiteralMethod
       identifier typeParameters? signature block
       ;

    spreadExpression:
        '...' expression
        ;

    parenthesizedExpression:
        '(' expression ')'
        ;

    thisExpression:
        'this'
        ;

    fieldAccessExpression:
        objectReference ('.' | '?.') identifier
        ;

    methodCallExpression:
        objectReference ('.' | '?.') identifier typeArguments? arguments block?
        ;

    functionCallExpression:
        expression ('?.' | typeArguments)? arguments block?
        ;

    indexingExpression:
        expression ('?.')? '[' expression ']'
        ;

    newExpression:
        newClassInstance
        | newArrayInstance
        ;

    newClassInstance:
        'new' typeArguments? typeReference arguments?
        ;

    castExpression:
        expression 'as' type
        ;

    instanceOfExpression:
        expression 'instanceof' type
        ;

    typeOfExpression:
        'typeof' expression
        ;

    ensureNotNullishExpression:
        expression '!'
        ;

    nullishCoalescingExpression:
        expression '??' expression
        ;

    unaryExpression:
        expression '++'
        | expression '--'
        | '++' expression
        | '--' expression
        | '+' expression
        | '-' expression
        | '~' expression
        | '!' expression
        ;

    multiplicativeExpression:
        expression '*' expression
        | expression '/' expression
        | expression '%' expression
        ;

    additiveExpression:
        expression '+' expression
        | expression '-' expression
        ;

    shiftExpression:
        expression '<<' expression
        | expression '>>' expression
        | expression '>>>' expression
        ;

    relationalExpression:
        expression '<' expression
        | expression '>' expression
        | expression '<=' expression
        | expression '>=' expression
        ;

    equalityExpression:
        expression ('==' | '===' | '!=' | '!==') expression
        ;

    bitwiseAndLogicalExpression:
        expression '&' expression
        | expression '^' expression
        | expression '|' expression
        ;

    conditionalAndExpression:
        expression '&&' expression
        ;

    conditionalOrExpression:
        expression '||' expression
        ;

    assignmentExpression:
        lhsExpression assignmentOperator rhsExpression
        ;

    assignmentOperator
        : '='
        | '+='  | '-='  | '*='   | '='  | '%='
        | '<<=' | '>>=' | '>>>='
        | '&='  | '|='  | '^='
        ;

    lhsExpression:
        expression
        ;

    rhsExpression:
        expression
        ;

    conditionalExpression:
        expression '?' expression ':' expression
        ;

    stringInterpolation:
        '`' (BacktickCharacter | embeddedExpression)* '`'
        ;

    embeddedExpression:
        '${' expression '}'
        ;

    lambdaExpression:
        annotationUsage? ('async'|typeParameters)? lambdaSignature '=>' lambdaBody
        ;

    lambdaBody:
        expression | block
        ;

    lambdaSignature:
        '(' lambdaParameterList? ')' returnType?
        | identifier    
        ;

    lambdaParameterList:
        lambdaParameter (',' lambdaParameter)* (',' restParameter)? ','?
        | restParameter ','?
        ;

    lambdaParameter:
        annotationUsage? (lambdaRequiredParameter | lambdaOptionalParameter)    
        ;

    lambdaRequiredParameter:
        identifier (':' type)?
        ;

    lambdaOptionalParameter:
        identifier '?' (':' type)?
        ;

    lambdaRestParameter:
        '...' lambdaRequiredParameter
        ;

    constantExpression:
        expression
        ;

    statement:
        expressionStatement
        | block
        | localDeclaration
        | ifStatement
        | loopStatement
        | breakStatement
        | continueStatement
        | returnStatement
        | switchStatement
        | throwStatement
        | tryStatement
        ;

    expressionStatement:
        expression
        ;

    block:
        '{' statement* '}'
        ;

    localDeclaration:
        annotationUsage?
        ( variableDeclaration
        | constantDeclaration
        )
        ;

    ifStatement:
        'if' '(' expression ')' thenStatement
        ('else' elseStatement)?
        ;

    thenStatement:
        statement
        ;

    elseStatement:
        statement
        ;

    loopStatement:
        (identifier ':')?
        whileStatement
        | doStatement
        | forStatement
        | forOfStatement
        ;

    whileStatement:
        'while' '(' expression ')' statement
        ;

    doStatement
        : 'do' statement 'while' '(' expression ')'
        ;

    forStatement:
        'for' '(' forInit? ';' expression? ';' forUpdate? ')' statement
        ;

    forInit:
        expressionSequence
        | variableDeclarations
        ;

    forUpdate:
        expressionSequence
        ;

    forOfStatement:
        'for' '(' forVariable 'of' expression ')' statement
        ;

    forVariable:
        identifier | ('let' | 'const') identifier (':' type)?
        ;

    breakStatement:
        'break' identifier?
        ;

    continueStatement:
        'continue' identifier?
        ;

    returnStatement:
        'return' expression?
        ;

    switchStatement:
        (identifier ':')? 'switch' '(' expression ')' switchBlock
        ;

    switchBlock
        : '{' caseClause* defaultClause? caseClause* '}'
        ;

    caseClause
        : 'case' expression ':' statement*
        ;

    defaultClause
        : 'default' ':' statement*
        ;

    throwStatement:
        'throw' expression
        ;

    tryStatement:
          'try' block catchClauses finallyClause?
          ;

    catchClauses:
          typedCatchClause* catchClause?
          ;

    catchClause:
          'catch' '(' identifier ')' block
          ;

    typedCatchClause:
          'catch' '(' identifier ':' typeReference ')' block
          ;

    finallyClause:
          'finally' block
          ;

    classDeclaration:
        classModifier? ('class' | 'struct') identifier typeParameters?
          classExtendsClause? implementsClause? classBody
        ;

    classModifier:
        'abstract' | 'final'
        ;

    classExtendsClause:
        'extends' typeReference
        ;

    implementsClause:
        'implements' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

    classBody:
        '{'
           classBodyDeclaration* globalInitializer? classBodyDeclaration*
        '}'
        ;

    classBodyDeclaration:
        annotationUsage?
        accessModifier?
        ( constructorDeclaration
        | classFieldDeclaration
        | classMethodDeclaration
        | classAccessorDeclaration
        )
        ;

    accessModifier:
        'private'
        | 'internal'
        | 'protected'
        | 'public'
        ;

    classFieldDeclaration:
        fieldModifier* 
        identifier
        ( '?'? ':' type initializer?
        | '?'? initializer
        | '!' ':' type
        )
        ;

    fieldModifier:
        'static' | 'readonly' | 'override'
        ;

    classMethodDeclaration:
        methodModifier* typeParameters? identifier signature block?
        ;

    methodModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        | 'native'
        | 'async'
        ;

    classAccessorDeclaration:
        accessorModifier*
        ( 'get' identifier '(' ')' returnType block?
        | 'set' identifier '(' parameter ')' block?
        )
        ;

    accessorModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        ;

    constructorDeclaration:
        'constructor' parameters constructorBody
        ;


    constructorBody:
        '{' statement* '}'
        ;

    interfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceMember* '}'
        ;

    interfaceExtendsClause:
        'extends' interfaceTypeList
        ;

    interfaceMember:
        annotationUsage?
        ( interfaceProperty
        | interfaceMethodDeclaration
        )
        ;

    interfaceProperty:
        'readonly'? identifier '?'? ':' type
        | 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        ;

    interfaceMethodDeclaration:
        identifier signature
        | interfaceDefaultMethodDeclaration
        ;

    enumDeclaration:
        'const'? 'enum' identifier (':' type)? '{' enumConstantList? '}'
        ;

    enumConstantList:
        enumConstant (',' enumConstant)* ','?
        ;

    enumConstant:
        identifier ('=' constantExpression)?
        ;

    compilationUnit:
        separateModuleDeclaration
        | packageDeclaration
        | declarationModule
        ;

    packageDeclaration:
        packageModule+
        ;

    separateModuleDeclaration:
        importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

    importDirective:
        'import' 'type'? bindings 'from' importPath
        ;

    bindings:
        defaultBinding
        | (defaultBinding ',')? allBinding
        | (defaultBinding ',')? selectiveBindings
    ;

    allBinding:
        '*' bindingAlias
        ;

    bindingAlias:
        'as' identifier
        ;

    defaultBinding:
        identifier
        ;

    selectiveBindings:
        nameBinding (',' nameBinding)*
        ;

    nameBinding:
        identifier bindingAlias?
        | 'default' 'as' identifier
        ;

    importPath:
        StringLiteral
        ;

    declarationModule:
        importDirective*
        ( 'export'? ambientDeclaration
        | 'export'? typeAlias
        | selectiveExportDirective
        )*
        ;

    topDeclaration:
        ('export' 'default'?)?
        annotationUsage?
        ( typeDeclaration
        | variableDeclarations
        | constantDeclarations
        | functionDeclaration
        | functionWithReceiverDeclaration
        | accessorWithReceiverDeclaration
        | namespaceDeclaration
        | ambientDeclaration
        | annotationDeclaration
        )
        ;

    namespaceDeclaration:
        'namespace' qualifiedName '{' topDeclaration* '}'
        ;

    exportDirective:
        selectiveExportDirective
        | singleExportDirective
        | exportTypeDirective
        | reExportDirective
        ;

    selectiveExportDirective:
        'export' selectiveBindings
        ;

    singleExportDirective:
        'export'
        ( identifier 
        | 'default' (expression | identifier)
        | '{' identifier 'as' 'default' '}'
        )
        ;

    exportTypeDirective:
        'export' 'type' selectiveBindings
        ;

    reExportDirective:
        'export' 
        ('*' bindingAlias? 
        | selectiveBindings 
        | '{' 'default' bindingAlias? '}'
        )
        'from' importPath
        ;

    topLevelStatements:
        statement*
        ;

    ambientDeclaration:
        'declare'
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | ambientAnnotationDeclaration
        | 'const'? enumDeclaration
        | typeAlias
        )
        ;

    ambientConstantDeclaration:
        'const' ambientConstList ';'
        ;

    ambientConstList:
        ambientConst (',' ambientConst)*
        ;

    ambientConst:
        identifier ((':' type) | ('=' (IntegerLiteral|FloatLiteral|StringLiteral|MultilineStringLiteral)))
        ;

    ambientFunctionDeclaration:
        'function' identifier typeParameters? signature
        ;

    ambientClassDeclaration:
        'class'|'struct' identifier typeParameters?
        classExtendsClause? implementsClause?
        '{' ambientClassBodyDeclaration* '}'
        ;

    ambientClassBodyDeclaration:
        ambientAccessModifier?
        ( ambientFieldDeclaration
        | ambientConstructorDeclaration
        | ambientMethodDeclaration
        | ambientAccessorDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        )
        ;

    ambientAccessModifier:
        'public' | 'protected'
        ;

    ambientFieldDeclaration:
        ambientFieldModifier* identifier ':' type
        ;

    ambientFieldModifier:
        'static' | 'readonly'
        ;

    ambientConstructorDeclaration:
        'constructor' parameters 
        ;

    ambientMethodDeclaration:
        ambientMethodModifier* identifier signature
        ;

    ambientMethodModifier:
        'static'
        ;

    ambientAccessorDeclaration:
        ambientMethodModifier*
        ( 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        )
        ;

    ambientIndexerDeclaration:
        'readonly'? '[' identifier ':' indexType ']' returnType
        ;

    ambientCallSignatureDeclaration:
        signature
        ;

    ambientIterableDeclaration:
        '[Symbol.iterator]' '(' ')' returnType
        ;

    ambientInterfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause?
        '{' ambientInterfaceMember* '}'
        ;

    ambientInterfaceMember
        : interfaceProperty
        | interfaceMethodDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        ;

    ambientNamespaceDeclaration:
        'namespace' qualifiedName '{' ambientNamespaceElement* '}'
        ;

    ambientNamespaceElement:
        ambientNamespaceElementDeclaration | selectiveExportDirective
    ;

    ambientNamespaceElementDeclaration:
        'export'?
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | 'const'? enumDeclaration
        | typeAlias
        )
        ;

      newArrayInstance:
          'new' arrayElementType dimensionExpression+ (arrayElement)?
          ;

      arrayElementType:
          typeReference
          | '(' type ')'
          ;

      dimensionExpression:
          '[' expression ']'
          ;

      arrayElement:
          '(' expression ')'
          ;

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

    functionWithReceiverDeclaration:
        'function' identifier typeParameters? signatureWithReceiver block
        ;

    signatureWithReceiver:
        '(' receiverParameter (', ' parameterList)? ')' returnType? 
        ;

    receiverParameter:
        annotationUsage? 'this' ':' type
        ;

    accessorWithReceiverDeclaration:
          'get' identifier '(' receiverParameter ')' returnType block
        | 'set' identifier '(' receiverParameter ',' parameter ')' block
        ;

    functionTypeWithReceiver:
        '(' receiverParameter (',' ftParameterList)? ')' ftReturnType
        ;

    lambdaExpressionWithReceiver:
        annotationUsage? typeParameters? '(' receiverParameter (',' lambdaParameterList)? ')'
        returnType? '=>' lambdaBody
        ;

    trailingLambdaCall:
        ( objectReference '.' identifier typeArguments?
        | expression ('?.' | typeArguments)?
        )
        arguments block
        ;

      launchExpression:
        'launch' functionCallExpression|methodCallExpression|lambdaExpression;

      awaitExpression:
        'await' expression
        ;

      packageModule:
          packageHeader packageModuleDeclaration
          ;

      packageHeader:
          'package' qualifiedName
          ;


    packageModuleDeclaration:
        importDirective* packageModuleDeclaration*
        ;

    packageModuleDeclaration:
        packageTopDeclaration | initializerBlock
        ;

    packageTopDeclaration:
        ('export' 'default'?)?
        annotationUsage?
        ( typeDeclaration
        | variableDeclarations
        | packageConstantDeclarations
        | functionDeclaration
        | functionWithReceiverDeclaration
        | accessorWithReceiverDeclaration
        | namespaceDeclaration
        | ambientDeclaration
        )
        ;

    packageConstantDeclaration:
        identifier ':' type initializer?
        | identifier initializer
        ;


      initializerBlock:
          'static' block
          ;

    annotationDeclaration:
        '@interface' identifier '{' annotationField* '}'
        ;

    annotationField:
        identifier ':' type constInitializer?
        ;

    constInitializer:
        '=' constantExpression
        ;

    annotationUsage:
        '@' qualifiedName annotationValues?
        ;

    annotationValues:
        '(' (objectLiteral | constantExpression)? ')'
        ;

    ambientAnnotationDeclaration:
        'declare' annotationDeclaration
        ;

    Identifier:
      IdentifierStart IdentifierPart*
      ;

    IdentifierStart:
      UnicodeIDStart
      | '$'
      | '_'
      | '\\' EscapeSequence
      ;

    IdentifierPart:
      UnicodeIDContinue
      | '$'
      | ZWNJ
      | ZWJ
      | '\\' EscapeSequence
      ;

    ZWJ:
     '\u200C'
    ;

    ZWNJ:
     '\u200D'
    ;

    UnicodeIDStart
      : Letter
      | ['$']
      | '\\' UnicodeEscapeSequence;

    UnicodeIDContinue
      : UnicodeIDStart
      | UnicodeDigit
      | '\u200C'
      | '\u200D';

    UnicodeEscapeSequence:
      'u' HexDigit HexDigit HexDigit HexDigit
      | 'u' '{' HexDigit HexDigit+ '}'
      ;

    Letter
      : UNICODE_CLASS_LU
      | UNICODE_CLASS_LL
      | UNICODE_CLASS_LT
      | UNICODE_CLASS_LM
      | UNICODE_CLASS_LO
      ;

    UnicodeDigit
      : UNICODE_CLASS_ND
      ;


    Literal:
      IntegerLiteral
      | FloatLiteral
      | BigIntLiteral
      | BooleanLiteral
      | StringLiteral
      | MultilineStringLiteral
      | RegExpLiteral
      | NullLiteral
      | UndefinedLiteral
      | CharLiteral
      ;

    IntegerLiteral:
      DecimalIntegerLiteral
      | HexIntegerLiteral
      | OctalIntegerLiteral
      | BinaryIntegerLiteral
      ;

    DecimalIntegerLiteral:
      '0'
      | DecimalDigitNotZero ('_'? DecimalDigit)*
      ;

    DecimalDigit:
      [0-9]
      ;

    DecimalDigitNotZero:
      [1-9]
      ;

    HexIntegerLiteral:
      '0' [xX]  ( HexDigit
      | HexDigit (HexDigit | '_')* HexDigit
      )
      ;

    HexDigit:
      [0-9a-fA-F]
      ;

    OctalIntegerLiteral:
      '0' [oO] ( OctalDigit
      | OctalDigit (OctalDigit | '_')* OctalDigit )
      ;

    OctalDigit:
      [0-7]
      ;

    BinaryIntegerLiteral:
      '0' [bB] ( BinaryDigit
      | BinaryDigit (BinaryDigit | '_')* BinaryDigit )
      ;

    BinaryDigit:
      [0-1]
      ;

    FloatLiteral:
        DecimalIntegerLiteral '.' FractionalPart? ExponentPart? FloatTypeSuffix?
        | '.' FractionalPart ExponentPart? FloatTypeSuffix?
        | DecimalIntegerLiteral ExponentPart FloatTypeSuffix?
        ;

    ExponentPart:
        [eE] [+-]? DecimalIntegerLiteral
        ;

    FractionalPart:
        DecimalDigit
        | DecimalDigit (DecimalDigit | '_')* DecimalDigit
        ;

    FloatTypeSuffix:
        'f'
        ;

    BigIntLiteral: IntegerLiteral 'n';

    BooleanLiteral:
        'true' | 'false'
        ;

    StringLiteral:
        '"' DoubleQuoteCharacter* '"'
        | '\'' SingleQuoteCharacter* '\''
        ;

    DoubleQuoteCharacter:
        ~["\\\r\n]
        | '\\' EscapeSequence
        ;

    SingleQuoteCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        ;

    EscapeSequence:
        ['"bfnrtv0\\]
        | 'x' HexDigit HexDigit
        | 'u' HexDigit HexDigit HexDigit HexDigit
        | 'u' '{' HexDigit+ '}'
        | ~[1-9xu\r\n]
        ;

    MultilineStringLiteral:
        '`' (BacktickCharacter)* '`'
        ;

    BacktickCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        | LineContinuation
        ;

     LineContinuation:
        '\\' [\r\n\u2028\u2029]+
        ;

    NullLiteral:
        'null'
        ;

    UndefinedLiteral:
        'undefined'
        ;

    CharLiteral:
        'c\'' SingleQuoteCharacter '\''
        ;

    RegexLiteral:
        '/' RegexCharSequence '$'? '/' RegExFlags?
        ;

    RegexCharSequence:
        (
            RegexCharacter
            |RegexSpecialForms
            |'(' RegexSpecialForms ')'
            |'(' '?<' Identifier '>' RegexSpecialForms ')'
            |'(' '?:' RegexSpecialForms ')'
        )+
        ;

    RegexCharacter:
        ~["'\\\r\n] ('*'|'+'|'?'|('{' DecimalIntegerLiteral (',' DecimalIntegerLiteral? )? '}'))?
        ;

    RegexSpecialForms:   
        CharacterClass ('(' '?='|'?!' CharacterClasse ')')? 
        ('(' '?<='|'?<!' CharacterClasse ')') CharacterClass
        ;

    CharacterClass: 
        '[' '^'? '\b'? (RegexCharacter | (RegexCharacter '-' RegexCharacter) '\B'?)+ '\b'? ']'
        | '.'
        | '\' ('d' | 'D' | 'w' | 'W' | 's' | 'S' | 't' | 'r' | 'n' | 'v' | 'f' | '0' | 'c' ['A'-'Z'] | 'x' DecimalDigit DecimalDigit | DecimalIntegerLiteral | 'k<' Identifier '>')
        | 'u' HexDigit HexDigit HexDigit HexDigit 
        | 'u{' HexDigit HexDigit HexDigit HexDigit HexDigit? '}'
        | '[\b]' 
        | (RegexCharacter '|' RegexCharacter)
        ;

    RegExFlags:
        'g'? 'i'? 'm'? 's'? 'u'? 'v'? 'y'? 
        ;



.. raw:: pdf

   PageBreak
