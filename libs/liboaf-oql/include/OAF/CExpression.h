/**
 * @file
 * @brief Интерфейс вычисляемых выражений
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * Данный интерфейс открывает доступ к внутренним параметрам выражений. Он предназначен
 * для тех компонентов, которым необходимо больше информации о дереве выражений, чем
 * это предоставляется интерфейсом OAF::IExpression.
 */
#ifndef __OQL_CEXPRESSION_H
#define __OQL_CEXPRESSION_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/IQuery.h>

#include <OAF/CUnknown.h>
#include <OAF/OafOqlGlobal.h>

namespace OQL
{
	/**
	 * @brief Константное значение
	 */
	class OAFOQL_EXPORT CExpressionConst :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IExpression,
		//
		// Импортируемые реализации
		//
		virtual public OAF::CUnknown
	{
		/**
		 * @brief Значение константы
		 */
		QVariant m_value;

	public:
		CExpressionConst (const QVariant& _value);

		/**
		 * @brief Реализация интерфейса OAF::IExpression
		 */
		/** @{*/
		Type type () const;
		QVariant eval (const OAF::IPropertyBag* _vars) const;
		/** @}*/
	};

	/**
	 * @brief Значение заданной переменной
	 */
	class OAFOQL_EXPORT CExpressionVariable :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IExpression,
		//
		// Импортируемые реализации
		//
		virtual public OAF::CUnknown
	{
		/**
		 * @brief Имя переменной
		 */
		QString m_name;

	public:
		CExpressionVariable (const QString& _name);

		/**
		 * @brief Реализация интерфейса OAF::IExpression
		 */
		/** @{*/
		Type type () const;
		QVariant eval (const OAF::IPropertyBag* _vars) const;
		/** @}*/

		/**
		 * @brief Имя переменной
		 */
		const QString& name () const;
	};

	/**
	 * @brief Функция
	 */
	class OAFOQL_EXPORT CExpressionFunction :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IExpression,
		//
		// Импортируемые реализации
		//
		virtual public OAF::CUnknown
	{
		/**
		 * @brief Функция
		 */
		const OAF::IFunctionFactory::FunctionDesc* m_func;

		/**
		 * @brief Аргументы
		 */
		QList<OAF::URef<OAF::IExpression> > m_args;

		/**
		 * @brief Добавить выражение к списку
		 */
		void add (OAF::IExpression* _expr);

		/**
		 * @brief Проверка аргументов на константность
		 */
		bool isConstArgs () const;

	public:
		/**
		 * @brief Функция
		 */
		CExpressionFunction (OAF::IFunctionFactory* _factory, const QString& _name, OAF::IExpression* _expr1 = NULL, OAF::IExpression* _expr2 = NULL);

		/**
		 * @brief Реализация интерфейса OAF::IExpression
		 */
		/** @{*/
		Type type () const;
		QVariant eval (const OAF::IPropertyBag* _vars) const;
		/** @{*/

		/**
		 * @brief Описатель функции
		 */
		const OAF::IFunctionFactory::FunctionDesc* func () const;

		/**
		 * @brief Список аргументов функции
		 */
		const QList<OAF::URef<OAF::IExpression> >& args () const;
	};

	/**
	 * @brief Список выражений
	 */
	class OAFOQL_EXPORT CExpressionList :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IExpression,
		//
		// Импортируемые реализации
		//
		virtual public OAF::CUnknown
	{
		/**
		 * @brief Список выражений
		 */
		QList<OAF::URef<OAF::IExpression> > m_list;

		/**
		 * @brief Добавить выражение к списку
		 */
		void add (OAF::IExpression* _expr);

	public:
		CExpressionList (OAF::IExpression* _expr1, OAF::IExpression* _expr2 = NULL);

		/**
		 * @brief Реализация интерфейса OAF::IExpression
		 */
		/** @{*/
		Type type () const;
		QVariant eval (const OAF::IPropertyBag* _vars) const;
		/** @}*/

		/**
		 * @brief Список выражений
		 */
		const QList<OAF::URef<OAF::IExpression> >& list () const;
	};

	/**
	 * @brief Выражение сортировки
	 *
	 * Это "хитрое" выражение, состоящее из двух частей:
	 *     -# обычное вычисляемое выражение;
	 *     -# направление сортировки.
	 *
	 * В случае вычисления этого выражения для списка переменных возвращается
	 * результат вычисления выражения. В случае вычисления для NULL возвращается
	 * направление сортировки в виде булевской переменной, где false - означает
	 * сортировку по возрастанию, а true - по убыванию.
	 */
	class OAFOQL_EXPORT CExpressionOrder :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IExpression,
		//
		// Импортируемые реализации
		//
		virtual public OAF::CUnknown
	{
		/**
		 * @brief Вычисляемое выражение
		 */
		OAF::URef<OAF::IExpression> m_expr;

		/**
		 * @brief Направление сортировки
		 */
		bool m_desc;

	public:
		CExpressionOrder (OAF::IExpression* _expr, OAF::IExpression* _opt_desc);

		/**
		 * @brief Реализация интерфейса OAF::IExpression
		 */
		/** @{*/
		Type type () const;
		QVariant eval (const OAF::IPropertyBag* _vars) const;
		/** @}*/
	};

	/**
	 * @brief Оптимизация заданного выражения
	 *
	 * Пока используется только один вид оптимизации - предвычисление функцией
	 * с константными аргументами не зависящих от контекста вычисления и замена
	 * их результатом вычисления
	 */
	OAFOQL_EXPORT OAF::URef<OAF::IExpression> optimize (const OAF::URef<OAF::IExpression>& _e);

	/**
	 * @brief Функция преобразования к заданному типу
	 */
	OAFOQL_EXPORT QVariant castAs (const QVariant& _v, QVariant::Type _type);

	/**
	 * @brief Функция сравнения двух вариантов
	 */
	OAFOQL_EXPORT int compare (const QVariant& _v1, const QVariant& _v2);

	/**
	 * @brief Функция сложения двух вариантов
	 */
	OAFOQL_EXPORT QVariant add (const QVariant& _v1, const QVariant& _v2);

	/**
	 * @brief Функция вычитания двух вариантов
	 */
	OAFOQL_EXPORT QVariant sub (const QVariant& _v1, const QVariant& _v2);

	/**
	 * @brief Функция умножения двух вариантов
	 */
	OAFOQL_EXPORT QVariant mul (const QVariant& _v1, const QVariant& _v2);

	/**
	 * @brief Функция деления двух вариантов
	 */
	OAFOQL_EXPORT QVariant div (const QVariant& _v1, const QVariant& _v2);

	/**
	 * @brief Функция обращения варианта
	 */
	OAFOQL_EXPORT QVariant neg (const QVariant& _v);

	/**
	 * @brief Шаблон функтора сравнения двух объектов
	 */
	template<class _T>
	class Comparator
	{
		/**
		 * @brief Список направлений сортировки
		 */
		QList<QVariant> m_desc;

	public:
		Comparator (const QList<QVariant>& _desc) : m_desc (_desc)
		{}
		;

		/**
		 * @brief Оператор сравнения двух объектов по вычисленным для них
		 *        выражениям сортировки
		 *
		 * Сравнение выполняется с учётом заданных направлений сортировки по
		 * каждому из выражений в списке сортировки
		 */
		bool operator() (const QPair<_T, QList<QVariant> >& _v1, const QPair<_T, QList<QVariant> >& _v2)
		{
			//
			// По всему списку направлений сортировки
			//
			for (int i = 0; i < m_desc.size (); ++i)
			{
				//
				// Направление сортировки по заданному параметру
				//
				bool desc = m_desc[i].toBool ();

				//
				// Результат сравнения
				//
				int comp = OQL::compare (_v1.second[i], _v2.second[i]);

				//
				// Результат сравнения в зависимости от направления сортировки
				//
				if ((desc && (comp <= 0)) || (!desc && (comp >= 0)))
					return false;
			}

			return true;
		};
	};
}

#endif /* __OQL_CEXPRESSION_H */
