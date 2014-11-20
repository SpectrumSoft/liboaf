/**
 * @file
 * @brief Интерфейс запроса
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IQUERY_H
#define __IQUERY_H

#include <QVariant>
#include <QList>
#include <QtCore>
//
// Необходимо для сборки с Qt 5.0 и выше
//
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/IPropertyBag.h>

namespace OAF
{
	/**
	 * @brief Ошибка при разборе текста запроса
	 */
	class ParseException : public QtConcurrent::Exception
	{
		/**
		 * @brief Описание исключения
		 */
		QString m_reason;

	public:
		ParseException (const QString& _reason) : m_reason (_reason)
		{}
		;

		ParseException (const OAF::ParseException& _pe) : QtConcurrent::Exception (_pe), m_reason (_pe.m_reason)
		{}
		;

		~ParseException () throw ()
		{}
		;

		const char* what () const throw ()
		{
			return "OAF::ParseException";
		};

		void raise () const
		{
			throw *this;
		};

		QtConcurrent::Exception* clone () const
		{
			return new OAF::ParseException (*this);
		};

		const QString& reason () const
		{
			return m_reason;
		};
	};


	/**
	 * @brief Ошибка при вычислении выражения
	 */
	class EvaluateException : public QtConcurrent::Exception
	{
		/**
		 * @brief Описание исключения
		 */
		QString m_reason;

	public:
		EvaluateException (const QString& _reason) : m_reason (_reason)
		{}
		;

		EvaluateException (const OAF::EvaluateException& _e) : QtConcurrent::Exception (_e), m_reason (_e.m_reason)
		{}
		;

		~EvaluateException () throw ()
		{}
		;

		const char* what () const throw ()
		{
			return "OAF::EvaluateException";
		};

		void raise () const
		{
			throw *this;
		};

		QtConcurrent::Exception* clone () const
		{
			return new OAF::EvaluateException (*this);
		};

		const QString& reason () const
		{
			return m_reason;
		};
	};

	/**
	 * @brief Интерфейс вычислимого выражения
	 */
	struct IExpression : virtual public OAF::IInterface
	{
		/**
		 * @brief Тип выражения
		 */
		enum Type
		{
			_CONST,     //!< Константа
			_VARIABLE,  //!< Переменная
			_FUNCTION,  //!< Функция
			_EXPR_LIST, //!< Список выражений
			_EXPR_ORDER //!< Выражение сортировки
		};

		/**
		 * @brief Тип выражения
		 */
		virtual Type type () const = 0;

		/**
		 * @brief Вычислить выражение для заданного набора переменных
		 *
		 * Для CONST, VARIABLE и FUNCTION возвращает QVariant. Для EXPR_LIST возвращает
		 * QVariant<QList<QVariant> >. Для EXPR_ORDER возвращает:
		 *     - если @a _vars != NULL, то результат вычисления выражения сортировки;
		 *     - иначе - направление сортировки как QVariant::Bool (false - ASC, true - DESC).
		 */
		virtual QVariant eval (const OAF::IPropertyBag* _vars) const = 0;
	};

	/**
	 * @brief Интерфейс фабрики функций
	 */
	struct IFunctionFactory : virtual public OAF::IInterface
	{
		/**
		 * @brief Вычисляемая функция
		 */
		typedef QVariant (*Function) (const OAF::IPropertyBag* _vars, const QList<OAF::URef<OAF::IExpression> >& _args);

		/**
		 * @brief Тип функции
		 */
		enum FunctionType
		{
			VOLATILE  = 0, //!< Функция не имеет стабильного значения
			STABLE    = 1, //!< Функция имеет глобальные зависимости
			IMMUTABLE = 2  //!< Функция возвращает один и тот же результат для одних и тех же аргументов
		};

		/**
		 * @brief Описание функции
		 */
		struct FunctionDesc
		{
			QString      name; //!< Имя функции
			Function     func; //!< Код функции
			FunctionType type; //!< Тип функции
		};

		/**
		 * @brief Найти описатель функции по её имени
		 */
		virtual const FunctionDesc* lookup (const QString& _name) const = 0;
	};

	/**
	 * @brief Запрос
	 */
	struct IQuery : virtual public OAF::IInterface
	{
		/**
		 * @brief Выражение, описывающие условие отбора объектов
		 *
		 * Если возвращает NULL, значит условия отбора не заданы и подходит любой объект
		 */
		virtual OAF::IExpression* where () const = 0;

		/**
		 * @brief Список выражений сортировки
		 *
		 * Если возвращает NULL, значит сортировка не задана. Выражение сортировки всегда представляет
		 * собой EXPR_LIST<EXPR_ORDER>
		 */
		virtual OAF::IExpression* order () const = 0;

		/**
		 * @brief Выражение, описывающие ограничение на количество отобранных объектов
		 *
		 * Если возвращает NULL, значит ограничение не задано и надо вернуть весь список подходящих
		 * объектов
		 */
		virtual OAF::IExpression* limit () const = 0;
	};
}

#endif /* __IQUERY_H */
