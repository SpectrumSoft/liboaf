/**
 * @file
 * @brief Загрузка и запись из/в XML поток
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IXMLPERSIST_H
#define __IXMLPERSIST_H

#include <QtCore>

#include <idl/IInterface.h>

namespace OAF
{
	/**
	 * @brief Интерфейс загрузки и записи из/в XML поток
	 *
	 * Данный интерфейс предназначен для создания составных файлов, в которых и из которых
	 * сохраняют/загружают данные несколько компонентов. Например можно представить себе
	 * некоторое приложение для управления требованиями, которое позволяет редактировать и
	 * сохранять тексты требований в произвольном формате, лишь бы установленные в систему
	 * компоненты поддерживали их с одновременной поддержкой интерфейсов IUIComponent и
	 * IXMLPersist, первый из которых предназначен для отображения и редактирования текстов,
	 * а второй - для их загрузки и записи в составе общего документа. Например:
	 * @code
	 * ...
	 *
	 * <pkm:embed pkm:mime="application/x-piket-requirement" ...>
	 *     <pkr:reqiurement id="001" pkr:mime="application/x-piket-document">
	 *         ... здесь текст требования в формате документа "Пикет"
	 *     </pkr:reqiurement>
	 * </pkm:embed>
	 *
	 * ...
	 *
	 * <pkm:embed pkm:mime="application/x-piket-requirement" ...>
	 *     <pkr:reqiurement id="001" pkr:mime="text/plain">
	 *         ... здесь текст требования в виде простого текста
	 *     </pkr:reqiurement>
	 * </pkm:embed>
	 *
	 * ...
	 * @endcode
	 *
	 * Другое приложение может позволять работать с документом того или иного формата
	 * из установленных в системе, в виде отдельного файла, в котором содержится только
	 * документ. Подобное приложение может использоваться для отладки подобного рода
	 * компонентов.
	 *
	 * @note Вспомогательные функции загрузки компонентов из потоков смотри в
	 *       libs/liboaf-std/include/OAF/Helpers.h
	 */
	struct IXMLPersist : virtual public OAF::IInterface
	{
		/**
		 * @brief Загрузить объект из заданного XML-потока
		 */
		virtual QXmlStreamReader& load (QXmlStreamReader& _is, const QStringList& _mime_types) = 0;

		/**
		 * @brief Сохранить объект в заданном XML-потоке
		 */
		virtual QXmlStreamWriter& save (QXmlStreamWriter& _os, const QStringList& _mime_types) = 0;
	};
}

#endif /* __IXMLPERSIST_H */
