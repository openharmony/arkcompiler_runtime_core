/*---
desc: Multi-line comments
---*/

/**/

/*
  block
*/

/*
  /** /
    nested block
  / **/
*/

/*// nested one-liner
  // nested one-liner
*/

// /* one-liner comment

// */ one-liner comment

class/**/C/**/{/***/public/****/foo/*****/
    ( /**/ p /***/ : /****/ int /******/ )
    : /***********************************
      /*
        /*
            nested comment
        */
      */
    */
    int
    /* method body with comments: */
    {return/***/p/***/*/***/2;}
}

function main(): int {
    /*1*/let/*let;*/x/*x*/:/*colon :*/int/***int type*/
    /*2*/let /***********************************************
                         ::: let statement :::
             ********************************************/////
    y // var name

    /*
        separation
        colon
    */

    :/* separation colon */

    /* /*
        nested comment
        // */
        another nested comment
        // */

    int/* element type */[ /*array dimension*/ ]=[
      /*** 1-st array element ***/44
      ,
      /*** 2-nd array element ***/55
    ]

    let s1: string = `
        // this is not a comment
        /* this is not a comment either */
    `

    // assert statements:
    assert y[0] == 44/*1*/ && y[1] == 55 /*2*/ && x == 0/*3*/
    assert new C().foo(2) == 4 /*4*/

    /* // */ return 0;
}
