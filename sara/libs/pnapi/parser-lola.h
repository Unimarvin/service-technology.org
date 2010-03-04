/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LCONTROL = 258,
     RCONTROL = 259,
     KEY_TRUE = 260,
     KEY_FALSE = 261,
     KEY_SAFE = 262,
     KEY_PLACE = 263,
     KEY_TRANSITION = 264,
     KEY_MARKING = 265,
     KEY_CONSUME = 266,
     KEY_PRODUCE = 267,
     COLON = 268,
     SEMICOLON = 269,
     COMMA = 270,
     NUMBER = 271,
     NEGATIVE_NUMBER = 272,
     IDENT = 273
   };
#endif
/* Tokens.  */
#define LCONTROL 258
#define RCONTROL 259
#define KEY_TRUE 260
#define KEY_FALSE 261
#define KEY_SAFE 262
#define KEY_PLACE 263
#define KEY_TRANSITION 264
#define KEY_MARKING 265
#define KEY_CONSUME 266
#define KEY_PRODUCE 267
#define COLON 268
#define SEMICOLON 269
#define COMMA 270
#define NUMBER 271
#define NEGATIVE_NUMBER 272
#define IDENT 273




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 52 "parser-lola.yy"
{
  int yt_int;
  char * yt_str;
}
/* Line 1489 of yacc.c.  */
#line 90 "parser-lola.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE pnapi_lola_yylval;

