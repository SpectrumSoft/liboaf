/**
 * @file
 * @brief Поддержка генерации HTML-отчета о различиях с другим объектом данных
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IDIFFERENCE_H
#define __IDIFFERENCE_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

namespace OAF
{
	/*
	 * FIXME: использовать поток вместо класса, а CHTMLGenerator сделать форматтером
	 *        поверх потока
	 */
	class CHtmlGenerator;

	/**
	 * @brief Поддержка генерации HTML-отчета о различиях с другим объектом данных
	 */
	struct IDifference : virtual public OAF::IInterface
	{
		/**
		 * @brief Проверяет, есть ли у данного объекта данных с указанным различия по содержимому
		 */
		virtual bool isDiffer (OAF::IUnknown* _other) = 0;

		/**
		 * @brief Генерирует HTML-таблицу c полным описанием объекта данных;
		 * весь текст выделяет либо зеленым, либо красным, в зависимости от параметра @a _inserted
		 *
		 * Нужно для случая, когда объект данных был удален/добавлен в другом модуле
		 *
		 * @param _inserted Был ли объект данных добавлен или удален в другом модуле
		 * @return HTML-таблица (зеленая - если объект данных добавлен, красная - если удален)
		 */
		virtual void diff (CHtmlGenerator& _hg, bool _inserted = true) = 0;

		/**
		 * @brief Возвращает HTML-отчет о различиях с указанным объектом данных;
		 * если объекты одинаковые, то возвращает содержимое одного из них
		 *
		 * Каждый реализующий данную функцию объект данных должен преобразовать @a _other
		 * к нужному типу, "пройтись" по всем своим важным для пользователя полям
		 * (например, для требования это текст требования, обоснования, комментария)
		 * и сгенерировать симпатичный HTML-отчет о найденных различиях.
		 * Нужно для случая, когда в другом модуле изменилось содержимое объекта данных
		 *
		 * @return HTML-таблица (зеленый текст - добавленный в другом модуле, красный - удаленный)
		 */
		virtual void diff (CHtmlGenerator& _hg, OAF::IUnknown* _other) = 0;
	};
}

#endif /* __IDIFFERENCE_H */
