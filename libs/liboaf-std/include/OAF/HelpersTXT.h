/**
 * @file
 * @brief Вспомогательные функции для текстовых документов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_HELPERS_TXT_H
#define __OAF_HELPERS_TXT_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <idl/ITextDocument.h>

#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Создание текстовых документов из XML-потоков
	 *
	 * Документ создаётся из CDATA секции XML-потока
	 */
	OAFSTD_EXPORT URef<OAF::ITextDocument> createTXTFromXML (QXmlStreamReader& _is, const QStringList& _mime_types);

	/**
	 * @brief Запись текстового документа в XML-поток
	 *
	 * Запись выполняется в CDATA-секцию XML-потока
	 */
	OAFSTD_EXPORT QXmlStreamWriter& saveTXTToXML (QXmlStreamWriter& _os, const QStringList& _mime_types, OAF::ITextDocument* _document);

	/**
	 * @brief Сделать аннотацию для QTextDocument
	 */
	OAFSTD_EXPORT QString getAnnonation (const QTextDocument& _doc);
}

#endif /* __OAF_HELPERS_TXT_H */
