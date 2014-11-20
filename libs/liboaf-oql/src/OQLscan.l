%{
/**
 * @file
 * @brief Лексический анализатор языка OQL
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CExpression.h>

#include "OQLgram_yacc.h"

/**
 * @brief Определение сканера входного потока
 *
 * Должно быть согласовано с соответствующим определением в OQLgram.y
 */
#define YY_DECL int OQL_lex (void)

/**
 * @brief Определение операции получения следующего блока данных из входного потока
 */
#define YY_INPUT(buf, result, max_size) (result = OQL_readInputStream (buf, max_size))

/**
 * @brief Преобразование из текстовой константы в строку
 */
static QString OQL_unescape (const QString& _text);
%}

%option 8bit
%option case-insensitive
%option never-interactive
%option nodefault
%option nounput
%option noyywrap

_INTEGER [[:digit:]]+
_DECIMAL (([[:digit:]]*\.[[:digit:]]+)|([[:digit:]]+\.[[:digit:]]*))
_FLOAT   ({_INTEGER}|{_DECIMAL})[Ee][-+]?[[:digit:]]+
NAME    [[:alpha:]_][[:alnum:]_]*
CNAME   ({NAME}\.)+{NAME}
STRING  "'"(\\'|[^'])*"'"

NON_NEWLINE [^\n\r]
COMMENT     "--"{NON_NEWLINE}*
WSPACE      [[:space:]]+|{COMMENT}

%%
ORDER return ORDER;
BY    return BY;
ASC   return ASC;
DESC  return DESC;
LIMIT return LIMIT;
NULL  return NULL_P;

AND  return AND; // Логическое 'И'
OR   return OR;  // Логическое 'ИЛИ'
XOR  return XOR; // Исключающее 'ИЛИ'
NOT  return NOT; // Логическое 'НЕ'
"="  return EQ;  // Оператор 'РАВНО'
"<>" return NE;  // Оператор 'НЕ РАВНО'
"<"  return LT;  // Оператор 'МЕНЬШЕ'
">"  return GT;  // Оператор 'БОЛЬШЕ'
"<=" return LE;  // Оператор 'МЕНЬШЕ ИЛИ РАВНО'
">=" return GE;  // Оператор 'БОЛЬШЕ ИЛИ РАВНО'
"+"  return ADD; // Оператор 'СЛОЖИТЬ'
"-"  return SUB; // Оператор 'ВЫЧЕСТЬ'
"*"  return MUL; // Оператор 'УМНОЖИТЬ'
"/"  return DIV; // Оператор 'РАЗДЕЛИТЬ'

(TRUE|YES)	{
				OQL_lval = new OQL::CExpressionConst (true);
				return _BOOLEAN;
			}

(FALSE|NO)	{
				OQL_lval = new OQL::CExpressionConst (false);
				return _BOOLEAN;
			}

{_INTEGER}	{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toLongLong ());
				return _INTEGER;
			}

{_DECIMAL}	{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toDouble ());
				return _FLOAT;
			}

{_FLOAT}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toDouble ());
				return _FLOAT;
			}

{NAME}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext));
				return NAME;
			}

{CNAME}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext));
				return CNAME;
			}

{STRING}	{
				OQL_lval = new OQL::CExpressionConst (OQL_unescape (QString::fromUtf8 (yytext)));
				return STRING;
			}

{WSPACE}	/*IGNORE*/

.			return yytext[0];
%%

static QString
OQL_unescape (const QString& _text)
{
	QString res;

	//
	// Отбрасываем первый и последний символы (апострофы)
	//
	for (int i = 1; i < _text.length () - 1; ++i)
	{
		//
		// Если это не экранирующий символ '\'
		//
		if (_text[i] != '\\')
			res += _text[i];

		//
		// Иначе обрабатываем экранировку
		//
		else
		{
			//
			// Если за экранирующим символом не следует никакого
			// другого символа, то прерываем обработку
			//
			if (++i == (_text.length () - 1))
				break;

			if (_text[i] == '\\')
				res += '\\';
			else if (_text[i] == 'n')
				res += '\n';
			else if (_text[i] == 'r')
				res += '\r';
			else if (_text[i] == 't')
				res += '\t';
			else
				res += _text[i];
		}
	}

	return res;
}
