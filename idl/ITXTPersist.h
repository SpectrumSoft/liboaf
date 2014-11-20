/**
 * @file
 * @brief Загрузка и запись из/в текстовый поток
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __ITXTPERSIST_H
#define __ITXTPERSIST_H

#include <QStringList>
#include <QTextStream>

#include <idl/IInterface.h>

namespace OAF
{
	/*
	 * FIXME: в определении интерфейсов не должны использоваться функции и классы,
	 *        реализуемые в библиотеках. Передачу параметров необходимо реализовать
	 *        с помощью OAF::IPropertyBag. А дополнительные функции можно реализовать
	 *        и во вспомогательной библиотеке.
	 *
	 * FIXME: если уж делать передачу параметров, то делать её зеркальной, то есть и
	 *        в методе load тоже. Так же лучше их передавать по указателю с дефолтным
	 *        NULL-значением, тогда те, кому такие параметры не нужны не будут их
	 *        использовать.
	 */
	struct CSaveParams;

	/**
	 * @brief Интерфейс загрузки и записи из/в текстовый поток
	 *
	 * @note Вспомогательные функции загрузки компонентов из потоков смотри в
	 *       libs/liboaf-std/include/OAF/Helpers.h
	 */
	struct ITXTPersist : virtual public OAF::IInterface
	{
		/**
		 * @brief Загрузить объект из заданного текстового потока
		 */
		virtual QTextStream& load (QTextStream& _is, const QStringList& _mime_types) = 0;

		/**
		 * @brief Сохранить объект в заданном текстовом потоке
		 */
		virtual QTextStream& save (QTextStream& _os, const QStringList& _mime_types, CSaveParams& _save_params) = 0;
	};
}

#endif /* __ITXTPERSIST_H */
