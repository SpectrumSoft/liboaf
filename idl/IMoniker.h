/**
 * @file
 * @brief Пакет интерфейсов подсистемы именования объектов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IMONIKER_H
#define __IMONIKER_H

#include <QString>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

namespace OAF
{
	/**
	 * @brief Моникер
	 *
	 * Интерфейс моникера реализует создание объекта по параметрам, заданным для моникера и
	 * связывание его с объектом, созданным "левым" моникером. Моникеры идентифицируются
	 * с помощью префиксов.
	 *
	 * Основная задача системы моникеров - построить цепочку объектов по строке, описывающей
	 * цепочку моникеров. С помощью интерфейса расширения моникеров данная схема дополняется
	 * возможностью задать требуемый интерфейс объекта, создаваемого с помощью моникеров.
	 */
	struct IMoniker : virtual public OAF::IInterface
	{
		/**
		 * @brief Задать параметры моникера
		 *
		 * Моникер обязан сохранить ссылку на заданный "левый" моникер
		 */
		virtual void set (OAF::IMoniker* _left, const QString& _prefix, const QString& _suffix) = 0;

		/**
		 * @brief Левый моникер
		 */
		virtual OAF::IMoniker* left () = 0;

		/**
		 * @brief Префикс моникера
		 */
		virtual const QString& prefix () = 0;

		/**
		 * @brief Суффикс моникера
		 */
		virtual const QString& suffix () = 0;

		/**
		 * @brief Создать с помощью моникера объект с заданным интерфейсом
		 *
		 * @return Ссылка на созданный объект
		 */
		virtual OAF::URef<OAF::IUnknown> resolve (const QString& _interface) = 0;
	};
}

#endif /* __IMONIKER_H */
