%code requires {
/**
 * @file
 * @brief Грамматика языка OQL
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/IQuery.h>

/**
 * @brief Тип переменной для хранения текущего состояния парсера и
 *        лексического анализатора
 */
#define OQL_STYPE OAF::URef<OAF::IExpression>

//
// Для сборки с помощью MSVS-компилятора отключаем установку атрибутов
//
#ifdef _MSC_VER
#define __attribute__(x)
#endif
}

%code provides {
//
// Переопределяем C-шные операторы управления памятью
//
#define malloc(_size) new char[_size]
#define free(_ptr)    delete [] (reinterpret_cast<char*> (_ptr));

/**
 * @brief Функция получения данных из входного потока
 */
int OQL_readInputStream (char* _buf, int _max_size);
}

%{
#include <QString>
#include <QDataStream>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/IQuery.h>

#include <OAF/CUnknown.h>
#include <OAF/CExpression.h>
#include <OAF/OQL.h>

/**
 * @brief Класс для хранения данных запроса
 */
class OQL_CQuery :
	//
	// Экспортируемые интерфейсы
	//
	virtual public OAF::IInterface,
	virtual public OAF::IUnknown,
	virtual public OAF::IQuery,
	//
	// Импортируемые реализации
	//
	virtual public OAF::CUnknown
{
	/**
	 * @brief WHERE
	 */
	OAF::URef<OAF::IExpression> m_where;

	/**
	 * @brief ORDER BY
	 */
	OAF::URef<OAF::IExpression> m_order;

	/**
	 * @brief LIMIT
	 */
	OAF::URef<OAF::IExpression> m_limit;

public:
	OQL_CQuery (OAF::IExpression* _where, OAF::IExpression* _order, OAF::IExpression* _limit);

	/**
	 * @name Реализация интерфейса OAF::IQuery
	 */
	/** @{*/
	OAF::IExpression* where () const;

	OAF::IExpression* order () const;

	OAF::IExpression* limit () const;
	/** @}*/
};

/**
 * @brief Псевдо-CID для OQL-запроса
 */
static const QString g_query_cid = "OQL/CQuery:1.0";

/**
 * @brief Фабрика функций
 */
static OAF::IFunctionFactory* g_function_factory;

/**
 * @brief Построенный запрос
 */
static OAF::URef<OQL_CQuery> g_result;

/**
 * @brief Сообщение о последней ошибке парсера
 */
static QString g_last_error_str;

/**
 * @brief Сохранение данных об ошибке
 */
static void
OQL_error (const char* _str)
{
	g_last_error_str = QString::fromUtf8 (_str);
}

/**
 * @brief Лексический анализатор
 */
extern int OQL_lex (void);

/**
 * @brief Сброс сканера в начальное состояние
 */
extern int OQLscanlex_destroy (void);

/**
 * @brief Указатель на входной поток
 */
static QDataStream* OQL_input_stream = NULL;

int
OQL_readInputStream (char* _buf, int _max_size)
{
	if (OQL_input_stream && (OQL_input_stream->status () == QDataStream::Ok))
		return OQL_input_stream->readRawData (_buf, _max_size);
	return 0;
}

OQL_CQuery::OQL_CQuery (OAF::IExpression* _where, OAF::IExpression* _order, OAF::IExpression* _limit) :
	OAF::CUnknown (g_query_cid), m_where (_where), m_order (_order), m_limit (_limit)
{}

OAF::IExpression*
OQL_CQuery::where () const
{
	return m_where;
}

OAF::IExpression*
OQL_CQuery::order () const
{
	return m_order;
}

OAF::IExpression*
OQL_CQuery::limit () const
{
	return m_limit;
}

//
// Размер стека парсера. Максимальный размер должен быть равен начальному
// размеру, чтобы не было попыток пересоздать область стека, так как
// копирование при этом выполняется с помощью memcpy, что совершенно не
// приемлемо для ссылок
//
#define YYINITDEPTH 2048
#define YYMAXDEPTH  YYINITDEPTH
%}

%define parse.error verbose
%define api.header.include {"OQLgram_yacc.h"}
%define api.prefix {OQL_}
%define api.token.prefix {OQL_T_}
%locations

%token    _INTEGER _DECIMAL _FLOAT NAME CNAME STRING
%token    ORDER BY ASC DESC LIMIT _BOOLEAN
%left     XOR
%left     OR
%left     AND
%left     EQ NE LT GT LE GE
%right    NOT UMINUS
%nonassoc NULL_P
%left     ADD SUB
%left     MUL DIV
%left     '(' ')'
%left     '.'

%%
stmt:
		where_clause order_clause limit_clause
		{
			try
			{
				g_result = new OQL_CQuery ($1, $2, $3);
			}
			catch (const std::exception& _ex)
			{
				yyerror (_ex.what ());
				YYERROR;
			}
			catch (...)
			{
				yyerror ("Unknown parse exception");
				YYERROR;
			}
		}
		;

where_clause:
		/* EMPTY */
		{
			$$ = NULL;
		}
		| expr
		{
			$$ = $1;
		}
		;

order_clause:
		/* EMPTY */
		{
			$$ = NULL;
		}
		| ORDER BY order_list
		{
			$$ = $3;
		}
		;

limit_clause:
		/* EMPTY */
		{
			$$ = NULL;
		}
		| LIMIT expr
		{
			$$ = $2;
		}
		;

order_list:
		expr opt_asc_desc
		{
			$$ = new OQL::CExpressionList (new OQL::CExpressionOrder ($1, $2));
		}
		| order_list ',' expr opt_asc_desc
		{
			$$ = new OQL::CExpressionList ($1, new OQL::CExpressionOrder ($3, $4));
		}
		;

opt_asc_desc:
		/* EMPTY */
		{
			$$ = NULL;
		}
		| ASC
		{
			$$ = new OQL::CExpressionConst (false);
		}
		| DESC
		{
			$$ = new OQL::CExpressionConst (true);
		}
		;

expr_list:
		/* EMPTY */
		{
			$$ = NULL;
		}
		| expr
		{
			$$ = new OQL::CExpressionList ($1);
		}
		| expr_list ',' expr
		{
			$$ = new OQL::CExpressionList ($1, $3);
		}
		;

expr:
		_BOOLEAN
		{
			$$ = $1;
		}
		| _INTEGER
		{
			$$ = $1;
		}
		| _DECIMAL
		{
			$$ = $1;
		}
		| _FLOAT
		{
			$$ = $1;
		}
		| STRING
		{
			$$ = $1;
		}
		| NULL_P
		{
			$$ = new OQL::CExpressionConst (QVariant ());
		}
		| string_list
		{
			$$ = $1;
		}
		| NAME
		{
			$$ = new OQL::CExpressionVariable (($1)->eval (NULL).value<QString> ());
		}
		| CNAME
		{
			$$ = new OQL::CExpressionVariable (($1)->eval (NULL).value<QString> ());
		}
		| NAME '(' expr_list ')'
		{
			//
			// Имя функции
			//
			QString function_name = ($1)->eval (NULL).value<QString> ();

			try
			{
				$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, function_name, $3));
			}
			catch (const OAF::ParseException& _ex)
			{
				yyerror (_ex.what ());
				YYERROR;
			};
		}
		| CNAME '(' expr_list ')'
		{
			//
			// Разбираем на элементы
			//
			QStringList property = ($1)->eval (NULL).value<QString> ().split ('.');

			//
			// Последний элемент становится именем функции
			//
			QString function_name = property.last ();

			//
			// Остальные - именем переменной
			//
			property.removeLast ();
			QString variable_name = property.join (".");

			try
			{
				$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, function_name, new OQL::CExpressionVariable (variable_name), $3));
			}
			catch (const OAF::ParseException& _ex)
			{
				yyerror (_ex.what ());
				YYERROR;
			};
		}
		| '(' expr ')'
		{
			$$ = $2;
		}
		| expr AND expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "AND", $1, $3));
		}
		| expr OR expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "OR", $1, $3));
		}
		| expr XOR expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "XOR", $1, $3));
		}
		| NOT expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "NOT", $2));
		}
		| expr EQ expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "EQ", $1, $3));
		}
		| expr NE expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "NE", $1, $3));
		}
		| expr LT expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "LT", $1, $3));
		}
		| expr GT expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "GT", $1, $3));
		}
		| expr LE expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "LE", $1, $3));
		}
		| expr GE expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "GE", $1, $3));
		}
		| ADD expr %prec UMINUS
		{
			$$ = $2;
		}
		| SUB expr %prec UMINUS
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "NEG", $2));
		}
		| expr ADD expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "ADD", $1, $3));
		}
		| expr SUB expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "SUB", $1, $3));
		}
		| expr MUL expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "MUL", $1, $3));
		}
		| expr DIV expr
		{
			$$ = OQL::optimize (new OQL::CExpressionFunction (g_function_factory, "DIV", $1, $3));
		}
		;

string_list:
		'[' strings ']'
		{
			$$ = $2;
		}
		;

strings:
		/* EMPTY */
		{
			$$ = new OQL::CExpressionConst (QStringList ());
		}
		| STRING
		{
			$$ = new OQL::CExpressionConst (QStringList (($1)->eval (NULL).value<QString> ()));
		}
		| strings ',' STRING
		{
			QStringList strings = ($1)->eval (NULL).value<QStringList> ();
			strings.append (($3)->eval (NULL).value<QString> ());

			$$ = new OQL::CExpressionConst (strings);
		}
		;
%%

OAF::URef<OAF::IQuery>
OQL::parseQuery (const QString& _query, OAF::IFunctionFactory* _factory)
{
	//
	// Устанавливаем фабрику функций
	//
	g_function_factory = _factory;

	//
	// Настраиваем входной поток
	//
	QDataStream is (_query.toUtf8 ());
	OQL_input_stream = &is;

	//
	// Запускаем парсер
	//
	if (OQL_parse () || !g_result)
	{
		//
		// Сброс сканера в начальное состояние
		//
		OQLscanlex_destroy ();

		//
		// Очищаем запрос, если он был создан, чтобы он не висел в памяти
		//
		if (g_result)
			g_result = NULL;

		//
		// Очищаем переменную сканера. Если этого не сделать, то последний
		// токен останется висеть, пока не будет снова вызван парсер. Ничего
		// страшного конечно в этом нет, но неаккуратно
		//
		OQL_lval = NULL;

		//
		// Сообщаем об ошибке
		//
		throw OAF::ParseException (g_last_error_str);
	}

	//
	// Очищаем переменную сканера (причина аналогична изложенной ранее)
	//
	OQL_lval = NULL;

	//
	// Возвращаем запрос и очищаем переменную результата
	//
	OAF::URef<OAF::IQuery> r = g_result.queryInterface<OAF::IQuery> ();
	g_result = NULL;
	return r;
}
