/**
 * @file
 * @brief Реализация фабрики функций OAF
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CExpression.h>

#include "CFunctionFactory.h"

using namespace OAF;

static QVariant
_and (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () < 2)
		throw OAF::EvaluateException ("'AND' function expect 2 or more args");

	//
	// Ложна, если ложен хотя бы один из аргументов. Если функция
	// не ложна и один из аргументов не определён, то вся функция
	// неопределена
	//

	//
	// Изначально результат - true
	//
	QVariant result = QVariant (true);

	//
	// Для всех агрументов
	//
	for (QList<OAF::URef<OAF::IExpression> >::const_iterator i = _args.begin (); i != _args.end (); ++i)
	{
		//
		// Вычисляем очередное выражение и приводим его к bool
		//
		QVariant e = OQL::castAs ((*i)->eval (_vars), QVariant::Bool);

		//
		// Если одно из выражений - false, то всё выражение - false
		//
		if (!e.isNull () && !e.toBool ())
			return e;

		//
		// Иначе результат будет либо true, либо неопределён
		//
		if (!result.isNull ())
			result = e;
	}

	return result;
}

static QVariant
_or (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () < 2)
		throw OAF::EvaluateException ("'OR' function expect 2 or more args");

	//
	// Истинна, если истинен хотя бы один из аргументов. Если функция
	// не истинна и один из аргументов не определён, то вся функция
	// неопределена
	//

	//
	// Изначально результат - false
	//
	QVariant result = QVariant (false);

	//
	// Для всех агрументов
	//
	for (QList<OAF::URef<OAF::IExpression> >::const_iterator i = _args.begin (); i != _args.end (); ++i)
	{
		//
		// Вычисляем очередное выражение и приводим его к bool
		//
		QVariant e = OQL::castAs ((*i)->eval (_vars), QVariant::Bool);

		//
		// Если одно из выражений - true, то всё выражение - true
		//
		if (!e.isNull () && e.toBool ())
			return e;

		//
		// Иначе результат будет либо false, либо неопределён
		//
		if (!result.isNull ())
			result = e;
	}

	return result;
}

static QVariant
_xor (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () < 2)
		throw OAF::EvaluateException ("'XOR' function expect 2 or more args");
	//
	// Истинна, если истинен в точности один из аргументов. Если один
	// из аргументов не определён, то вся функция неопределена
	//

	//
	// Накапливаем сумму, считая false за 0, а true - за 1
	//
	int result = 0;

	//
	// Для всех агрументов
	//
	for (QList<OAF::URef<OAF::IExpression> >::const_iterator i = _args.begin (); i != _args.end (); ++i)
	{
		//
		// Вычисляем очередное выражение и приводим его к bool
		//
		QVariant e = OQL::castAs ((*i)->eval (_vars), QVariant::Bool);

		//
		// Если одно из выражений - не определено, то и всё выражение
		// не определено
		//
		if (e.isNull ())
			return e;

		//
		// Иначе добавляем результат к сумме
		//
		if (e.toBool ())
			result += 1;

		//
		// Если результат больше 1, то дальше можно не считать, общий
		// результат всё равно будет - false
		//
		if (result > 1)
			return QVariant (false);
	}

	//
	// Результат true если истинно ровно одно выражение
	//
	return QVariant (result == 1);
}

static QVariant
_not (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 1)
		throw OAF::EvaluateException ("'NOT' function expect exactly 1 args");

	QVariant v = OQL::castAs (_args[0]->eval (_vars), QVariant::Bool);
	if (!v.isNull ())
		v = QVariant (!v.value<bool> ());

	return v;
}

static QVariant
_eq (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'EQ' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r == 0);
}

static QVariant
_ne (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'NE' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r != 0);
}

static QVariant
_lt (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'LT' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r < 0);
}

static QVariant
_gt (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'GT' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r > 0);
}

static QVariant
_le (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'LE' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r <= 0);
}

static QVariant
_ge (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'GE' function expect exactly 2 args");

	QVariant v1 = _args[0]->eval (_vars);
	QVariant v2 = _args[1]->eval (_vars);

	int r = OQL::compare (v1, v2);
	return (r == -2) ? QVariant () : QVariant (r >= 0);
}

static QVariant
_add (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'ADD' function expect exactly 2 args");

	return OQL::add (_args[0]->eval (_vars), _args[1]->eval (_vars));
}

static QVariant
_sub (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'SUB' function expect exactly 2 args");

	return OQL::sub (_args[0]->eval (_vars), _args[1]->eval (_vars));
}

static QVariant
_mul (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'MUL' function expect exactly 2 args");

	return OQL::mul (_args[0]->eval (_vars), _args[1]->eval (_vars));
}

static QVariant
_div (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'DIV' function expect exactly 2 args");

	return OQL::div (_args[0]->eval (_vars), _args[1]->eval (_vars));
}

static QVariant
_neg (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 1)
		throw OAF::EvaluateException ("'NEG' function expect exactly 1 args");

	return OQL::neg (_args[0]->eval (_vars));
}

static QVariant
_defined (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 1)
		throw OAF::EvaluateException ("'defined' function expect exactly 1 args");

	return QVariant (!_args[0]->eval (_vars).isNull ());
}

static QVariant
_has (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'has' function expect exactly 2 args");

	QVariant arg1 = OQL::castAs (_args[0]->eval (_vars), QVariant::StringList);
	QVariant arg2 = OQL::castAs (_args[1]->eval (_vars), QVariant::String);

	if (!arg1.isNull () && !arg2.isNull ())
		return QVariant (arg1.value<QStringList> ().contains (arg2.value<QString> ()));

	return QVariant ();
}

static QVariant
_has_one (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'has_one' function expect exactly 2 args");

	QVariant arg1 = OQL::castAs (_args[0]->eval (_vars), QVariant::StringList);
	QVariant arg2 = OQL::castAs (_args[1]->eval (_vars), QVariant::StringList);

	if (!arg1.isNull () && !arg2.isNull ())
	{
		QStringList v1 = arg1.value<QStringList> ();
		QStringList v2 = arg2.value<QStringList> ();

		for (QStringList::const_iterator i = v2.begin (); i != v2.end (); ++i)
		{
			if (v1.contains (*i))
				return QVariant (true);
		}

		return QVariant (false);
	}

	return QVariant ();
}

static QVariant
_has_all (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'has_all' function expect exactly 2 args");

	QVariant arg1 = OQL::castAs (_args[0]->eval (_vars), QVariant::StringList);
	QVariant arg2 = OQL::castAs (_args[1]->eval (_vars), QVariant::StringList);

	if (!arg1.isNull () && !arg2.isNull ())
	{
		QStringList v1 = arg1.value<QStringList> ();
		QStringList v2 = arg2.value<QStringList> ();

		for (QStringList::const_iterator i = v2.begin (); i != v2.end (); ++i)
		{
			if (!v1.contains (*i))
				return QVariant (false);
		}

		return QVariant (true);
	}

	return QVariant ();
}

static QVariant
_prefer (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 2)
		throw OAF::EvaluateException ("'prefer' function expect exactly 2 args");

	QVariant arg1 = OQL::castAs (_args[0]->eval (_vars), QVariant::String);
	QVariant arg2 = OQL::castAs (_args[1]->eval (_vars), QVariant::StringList);

	if (!arg1.isNull () && !arg2.isNull ())
	{
		QString     v1 = arg1.value<QString> ();
		QStringList v2 = arg2.value<QStringList> ();

		int idx = v2.indexOf (v1);
		if (idx >= 0)
			idx = v2.size () - idx;

		return QVariant (idx);
	}

	return QVariant ();
}

static QVariant
_if (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	if (_args.size () != 3)
		throw OAF::EvaluateException ("'if' function expect 3 args");

	//
	// Проверочное выражение
	//
	QVariant check = OQL::castAs (_args[0]->eval (_vars), QVariant::Bool);
	if (check.isNull ())
		return QVariant ();

	//
	// В зависимости от проверочного условия возвращаем либо результат вычисления
	// первого либо второго выражения.
	//
	return check.value<bool> () ? _args[1]->eval (_vars) : _args[2]->eval (_vars);
}

static QVariant
_ifnull (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args)
{
	//
	// Если список не пуст
	//
	if (!_args.empty ())
	{
		//
		// Возвращаем первое не NULL значение
		//
		for (int i = 0; i < _args.size (); ++i)
		{
			QVariant v = _args[i]->eval (_vars);
			if (!v.isNull ())
				return v;
		}
	}

	//
	// Иначе возвращаем NULL значение
	//
	return QVariant ();
}

OAF::IFunctionFactory::FunctionDesc CFunctionFactory::funcs[] =
	{
		//
		// Функции - операторы
		//
		{"AND"      , _and, IMMUTABLE},
		{"OR"       , _or , IMMUTABLE},
		{"XOR"      , _xor, IMMUTABLE},
		{"NOT"      , _not, IMMUTABLE},
		{"EQ"       , _eq , IMMUTABLE}, // =
		{"NE"       , _ne , IMMUTABLE}, // <>
		{"LT"       , _lt , IMMUTABLE}, // <
		{"GT"       , _gt , IMMUTABLE}, // >
		{"LE"       , _le , IMMUTABLE}, // <=
		{"GE"       , _ge , IMMUTABLE}, // >=
		{"ADD"      , _add, IMMUTABLE}, // +
		{"SUB"      , _sub, IMMUTABLE}, // -
		{"MUL"      , _mul, IMMUTABLE}, // *
		{"DIV"      , _div, IMMUTABLE}, // /
		{"NEG"      , _neg, IMMUTABLE}, // унарный -

		//
		// Функции
		//
		{"DEFINED", _defined, IMMUTABLE},
		{"HAS"    , _has    , IMMUTABLE},
		{"HAS_ONE", _has_one, IMMUTABLE},
		{"HAS_ALL", _has_all, IMMUTABLE},
		{"PREFER" , _prefer , IMMUTABLE},
		{"IF"     , _if     , IMMUTABLE},
		{"IFNULL" , _ifnull , IMMUTABLE},
	};

const std::size_t CFunctionFactory::funcs_size = sizeof (CFunctionFactory::funcs)/sizeof (OAF::IFunctionFactory::FunctionDesc);

CFunctionFactory::CFunctionFactory ()
{}

CFunctionFactory::~CFunctionFactory ()
{}

const OAF::IFunctionFactory::FunctionDesc*
CFunctionFactory::lookup (const QString& _name) const
{
	for (std::size_t i = 0; i < funcs_size; ++i)
	{
		if (QString::compare (_name, funcs[i].name, Qt::CaseInsensitive) == 0)
			return &funcs[i];
	}

	throw OAF::ParseException ("Unknown function name: " + _name);
}
