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
ORDER return OQL_T_ORDER;
BY    return OQL_T_BY;
ASC   return OQL_T_ASC;
DESC  return OQL_T_DESC;
LIMIT return OQL_T_LIMIT;
NULL  return OQL_T_NULL_P;

AND  return OQL_T_AND; // Логическое 'И'
OR   return OQL_T_OR;  // Логическое 'ИЛИ'
XOR  return OQL_T_XOR; // Исключающее 'ИЛИ'
NOT  return OQL_T_NOT; // Логическое 'НЕ'
"="  return OQL_T_EQ;  // Оператор 'РАВНО'
"<>" return OQL_T_NE;  // Оператор 'НЕ РАВНО'
"<"  return OQL_T_LT;  // Оператор 'МЕНЬШЕ'
">"  return OQL_T_GT;  // Оператор 'БОЛЬШЕ'
"<=" return OQL_T_LE;  // Оператор 'МЕНЬШЕ ИЛИ РАВНО'
">=" return OQL_T_GE;  // Оператор 'БОЛЬШЕ ИЛИ РАВНО'
"+"  return OQL_T_ADD; // Оператор 'СЛОЖИТЬ'
"-"  return OQL_T_SUB; // Оператор 'ВЫЧЕСТЬ'
"*"  return OQL_T_MUL; // Оператор 'УМНОЖИТЬ'
"/"  return OQL_T_DIV; // Оператор 'РАЗДЕЛИТЬ'

(TRUE|YES)	{
				OQL_lval = new OQL::CExpressionConst (true);
				return OQL_T__BOOLEAN;
			}

(FALSE|NO)	{
				OQL_lval = new OQL::CExpressionConst (false);
				return OQL_T__BOOLEAN;
			}

{_INTEGER}	{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toLongLong ());
				return OQL_T__INTEGER;
			}

{_DECIMAL}	{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toDouble ());
				return OQL_T__FLOAT;
			}

{_FLOAT}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext).toDouble ());
				return OQL_T__FLOAT;
			}

{NAME}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext));
				return OQL_T_NAME;
			}

{CNAME}		{
				OQL_lval = new OQL::CExpressionConst (QString::fromUtf8 (yytext));
				return OQL_T_CNAME;
			}

{STRING}	{
				OQL_lval = new OQL::CExpressionConst (OQL_unescape (QString::fromUtf8 (yytext)));
				return OQL_T_STRING;
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
