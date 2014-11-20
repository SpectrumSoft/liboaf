/**
 * @file
 * @brief Реализация вычисляемых выражений
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CExpression.h>

using namespace OQL;

//
// Псевдо-CID для выражений
//
static const QString expression_const_cid      = "OQL/CExpressionConst:1.0";
static const QString expression_variable_cid   = "OQL/CExpressionVariable:1.0";
static const QString expression_function_cid   = "OQL/CExpressionFunction:1.0";
static const QString expression_list_cid       = "OQL/CExpressionList:1.0";
static const QString expression_order_cid      = "OQL/CExpressionOrder:1.0";

CExpressionConst::CExpressionConst (const QVariant& _value) :
	CUnknown (expression_const_cid), m_value (_value)
{}

OAF::IExpression::Type
CExpressionConst::type () const
{
	return _CONST;
}

QVariant
CExpressionConst::eval (const OAF::IPropertyBag* /*_vars*/) const
{
	return m_value;
}

CExpressionVariable::CExpressionVariable (const QString& _name) :
	CUnknown (expression_variable_cid), m_name (_name)
{}

OAF::IExpression::Type
CExpressionVariable::type () const
{
	return _VARIABLE;
}

QVariant
CExpressionVariable::eval (const OAF::IPropertyBag* _vars) const
{
	if ((_vars == NULL) || !_vars->isDefined (m_name))
		return QVariant (); // NULL

	return _vars->getValue (m_name);
}

const QString&
CExpressionVariable::name () const
{
	return m_name;
}

void
CExpressionFunction::add (OAF::IExpression* _expr)
{
	if (_expr)
	{
		switch (_expr->type ())
		{
			//
			// Простые выражения
			//
			case _CONST:
			case _VARIABLE:
			case _FUNCTION:
				m_args.append (_expr);
				break;

			//
			// Список выражений
			//
			case _EXPR_LIST:
				m_args.append (OAF::queryInterface<CExpressionList> (_expr)->list ());
				break;

			//
			// Остальное игнорируем
			//
			default:
				break;
		}
	}
}

bool
CExpressionFunction::isConstArgs () const
{
	for (QList<OAF::URef<OAF::IExpression> >::const_iterator i = m_args.begin (); i != m_args.end (); ++i)
	{
		if ((*i)->type () != _CONST)
			return false;
	}

	return true;
}

CExpressionFunction::CExpressionFunction (OAF::IFunctionFactory* _factory, const QString& _name, OAF::IExpression* _expr1, OAF::IExpression* _expr2) :
	CUnknown (expression_function_cid), m_func (NULL)
{
	Q_ASSERT (_factory != NULL);

	m_func = _factory->lookup (_name);

	add (_expr1);
	add (_expr2);
}

OAF::IExpression::Type
CExpressionFunction::type () const
{
	//
	// Функция, зависящая только от аргументов, у которой все аргументы
	// константы сама является константой
	//
	if ( (m_func->type == OAF::IFunctionFactory::IMMUTABLE) && isConstArgs ())
		return _CONST;

	return _FUNCTION;
}

QVariant
CExpressionFunction::eval (const OAF::IPropertyBag* _vars) const
{
	Q_ASSERT ( (m_func != NULL) && (m_func->func != NULL));

	return m_func->func (_vars, m_args);
}

const OAF::IFunctionFactory::FunctionDesc*
CExpressionFunction::func () const
{
	return m_func;
}

const QList<OAF::URef<OAF::IExpression> >&
CExpressionFunction::args () const
{
	return m_args;
}

void
CExpressionList::add (OAF::IExpression* _expr)
{
	if (_expr)
	{
		switch (_expr->type ())
		{
			//
			// Простые выражения
			//
			case _CONST:
			case _VARIABLE:
			case _FUNCTION:
			case _EXPR_ORDER:
				m_list.append (_expr);
				break;

			//
			// Список выражений
			//
			case _EXPR_LIST:
				m_list.append (OAF::queryInterface<CExpressionList> (_expr)->list ());
				break;

			//
			// Остальное игнорируем
			//
			default:
				break;
		}
	}
}

CExpressionList::CExpressionList (OAF::IExpression* _expr1, OAF::IExpression* _expr2) :
	CUnknown (expression_list_cid)
{
	add (_expr1);
	add (_expr2);
}

OAF::IExpression::Type
CExpressionList::type () const
{
	return _EXPR_LIST;
}

QVariant
CExpressionList::eval (const OAF::IPropertyBag* _vars) const
{
	QList<QVariant> result;

	for (QList<OAF::URef<OAF::IExpression> >::const_iterator i = m_list.begin (); i != m_list.end (); ++i)
		result.append ((*i)->eval (_vars));

	return QVariant (result);
}

const QList<OAF::URef<OAF::IExpression> >&
CExpressionList::list () const
{
	return m_list;
}

CExpressionOrder::CExpressionOrder (OAF::IExpression* _expr, OAF::IExpression* _opt_desc) :
	CUnknown (expression_order_cid), m_expr (_expr), m_desc (false)
{
	if (_opt_desc && (_opt_desc->type () == _CONST))
		m_desc = _opt_desc->eval (NULL).value<bool> ();
}

OAF::IExpression::Type
CExpressionOrder::type () const
{
	return _EXPR_ORDER;
}

QVariant
CExpressionOrder::eval (const OAF::IPropertyBag* _vars) const
{
	if (_vars)
		return m_expr->eval (_vars);

	return QVariant (m_desc);
}

OAF::URef<OAF::IExpression>
OQL::optimize (const OAF::URef<OAF::IExpression>& _e)
{
	//
	// Замена константной функции её значением
	//
	if (_e && (_e->type () == OAF::IExpression::_CONST))
		return OAF::URef<OAF::IExpression> (new CExpressionConst (_e->eval (NULL)));

	return _e;
}

QVariant
OQL::castAs (const QVariant& _v, QVariant::Type _type)
{
	if (!_v.isNull ())
	{
		//
		// @todo Проверить как кастуется QString к QStringList и обратно
		//
		QVariant v (_v);

		if (!v.convert (_type))
			return QVariant ();

		return v;
	}

	return _v;
}

namespace OQL
{
	/**
	 * @brief Выбор типа данных для сравнения
	 */
	static QVariant::Type
	compareType (QVariant::Type _t1, QVariant::Type _t2)
	{
		if ((_t1 == QVariant::Invalid) || (_t2 == QVariant::Invalid))
			return QVariant::Invalid;

		if ((_t1 == QVariant::DateTime) || (_t2 == QVariant::DateTime))
			return QVariant::DateTime;

		if ((_t1 == QVariant::Date) || (_t2 == QVariant::Date))
			return QVariant::Date;

		if ((_t1 == QVariant::Double) || (_t2 == QVariant::Double))
			return QVariant::Double;

		if ((_t1 == QVariant::ULongLong) || (_t2 == QVariant::ULongLong))
			return QVariant::ULongLong;

		if ((_t1 == QVariant::LongLong) || (_t2 == QVariant::LongLong))
			return QVariant::LongLong;

		if ((_t1 == QVariant::UInt) || (_t2 == QVariant::UInt))
			return QVariant::UInt;

		if ((_t1 == QVariant::Int) || (_t2 == QVariant::Int))
			return QVariant::Int;

		if ((_t1 == QVariant::String) || (_t2 == QVariant::String))
			return QVariant::String;

		return QVariant::Invalid;
	}

	template<typename _T>
	inline int
	compare (const _T& _v1, const _T& _v2)
	{
		if (_v1 < _v2)
			return -1;
		else if (_v2 < _v1)
			return 1;
		else
			return 0;
	}

	template<>
	inline int
	compare<QString> (const QString& _v1, const QString& _v2)
	{
		return QString::compare (_v1, _v2);
	}
}

int
OQL::compare (const QVariant& _v1, const QVariant& _v2)
{
	switch (OQL::compareType (_v1.type (), _v2.type ()))
	{
		case QVariant::DateTime:
			{
				QDateTime v1 = _v1.toDateTime ();
				QDateTime v2 = _v2.toDateTime ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::Date:
			{
				QDate v1 = _v1.toDate ();
				QDate v2 = _v2.toDate ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::Double:
			{
				double v1 = _v1.toDouble ();
				double v2 = _v2.toDouble ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::ULongLong:
			{
				qulonglong v1 = _v1.toULongLong ();
				qulonglong v2 = _v2.toULongLong ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::LongLong:
			{
				qlonglong v1 = _v1.toLongLong ();
				qlonglong v2 = _v2.toLongLong ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::UInt:
			{
				uint v1 = _v1.toUInt ();
				uint v2 = _v2.toUInt ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::Int:
			{
				int v1 = _v1.toInt ();
				int v2 = _v2.toInt ();

				return OQL::compare (v1, v2);
			}
			break;

		case QVariant::String:
			{
				QString v1 = _v1.toString ();
				QString v2 = _v2.toString ();

				return OQL::compare (v1, v2);
			}
			break;

		default:
			break;
	}

	//
	// Недопустимое сравнение
	//
	return -2;
}

QVariant
OQL::add (const QVariant& _v1, const QVariant& _v2)
{
	if ((_v1.type () == QVariant::Double) || (_v2.type () == QVariant::Double))
		return QVariant (_v1.toDouble () + _v2.toDouble ());

	if ((_v1.type () == QVariant::ULongLong) || (_v2.type () == QVariant::ULongLong))
		return QVariant (_v1.toULongLong () + _v2.toULongLong ());

	if ((_v1.type () == QVariant::LongLong) || (_v2.type () == QVariant::LongLong))
		return QVariant (_v1.toLongLong () + _v2.toLongLong ());

	if ((_v1.type () == QVariant::UInt) || (_v2.type () == QVariant::UInt))
		return QVariant (_v1.toUInt () + _v2.toUInt ());

	if ((_v1.type () == QVariant::Int) || (_v2.type () == QVariant::Int))
		return QVariant (_v1.toInt () + _v2.toInt ());

	if ((_v1.type () == QVariant::StringList) || (_v2.type () == QVariant::StringList))
		return QVariant (_v1.toStringList () + _v2.toStringList ());

	if ((_v1.type () == QVariant::String) || (_v2.type () == QVariant::String))
		return QVariant (_v1.toString () + _v2.toString ());

	return QVariant ();
}

QVariant
OQL::sub (const QVariant& _v1, const QVariant& _v2)
{
	if ((_v1.type () == QVariant::Double) || (_v2.type () == QVariant::Double))
		return QVariant (_v1.toDouble () - _v2.toDouble ());

	if ((_v1.type () == QVariant::ULongLong) || (_v2.type () == QVariant::ULongLong))
		return QVariant (_v1.toULongLong () - _v2.toULongLong ());

	if ((_v1.type () == QVariant::LongLong) || (_v2.type () == QVariant::LongLong))
		return QVariant (_v1.toLongLong () - _v2.toLongLong ());

	if ((_v1.type () == QVariant::UInt) || (_v2.type () == QVariant::UInt))
		return QVariant (_v1.toUInt () - _v2.toUInt ());

	if ((_v1.type () == QVariant::Int) || (_v2.type () == QVariant::Int))
		return QVariant (_v1.toInt () - _v2.toInt ());

	return QVariant ();
}

QVariant
OQL::mul (const QVariant& _v1, const QVariant& _v2)
{
	if ((_v1.type () == QVariant::Double) || (_v2.type () == QVariant::Double))
		return QVariant (_v1.toDouble () * _v2.toDouble ());

	if ((_v1.type () == QVariant::ULongLong) || (_v2.type () == QVariant::ULongLong))
		return QVariant (_v1.toULongLong () * _v2.toULongLong ());

	if ((_v1.type () == QVariant::LongLong) || (_v2.type () == QVariant::LongLong))
		return QVariant (_v1.toLongLong () * _v2.toLongLong ());

	if ((_v1.type () == QVariant::UInt) || (_v2.type () == QVariant::UInt))
		return QVariant (_v1.toUInt () * _v2.toUInt ());

	if ((_v1.type () == QVariant::Int) || (_v2.type () == QVariant::Int))
		return QVariant (_v1.toInt () * _v2.toInt ());

	return QVariant ();
}

QVariant
OQL::div (const QVariant& _v1, const QVariant& _v2)
{
	if ((_v1.type () == QVariant::Double) || (_v2.type () == QVariant::Double))
		return QVariant (_v1.toDouble () / _v2.toDouble ());

	if ((_v1.type () == QVariant::ULongLong) || (_v2.type () == QVariant::ULongLong))
		return QVariant (_v1.toULongLong () / _v2.toULongLong ());

	if ((_v1.type () == QVariant::LongLong) || (_v2.type () == QVariant::LongLong))
		return QVariant (_v1.toLongLong () / _v2.toLongLong ());

	if ((_v1.type () == QVariant::UInt) || (_v2.type () == QVariant::UInt))
		return QVariant (_v1.toUInt () / _v2.toUInt ());

	if ((_v1.type () == QVariant::Int) || (_v2.type () == QVariant::Int))
		return QVariant (_v1.toInt () / _v2.toInt ());

	return QVariant ();
}

QVariant
OQL::neg (const QVariant& _v)
{
	if (_v.type () == QVariant::Double)
		return QVariant (-_v.value<double> ());

	if (_v.type () == QVariant::LongLong)
		return QVariant (-_v.value<qlonglong> ());

	if (_v.type () == QVariant::Int)
		return QVariant (-_v.value<int> ());

	return QVariant ();
}
