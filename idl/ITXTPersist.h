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

#include <QtCore>

#include <idl/IInterface.h>

namespace OAF
{
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
		virtual QTextStream& save (QTextStream& _os, const QStringList& _mime_types) = 0;
	};
}

#endif /* __ITXTPERSIST_H */
