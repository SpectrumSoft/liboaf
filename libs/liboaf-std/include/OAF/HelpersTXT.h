/**
 * @file
 * @brief Вспомогательные классы и функции для загрузки/записи текстовых документов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_HELPERS_TXT_H
#define __OAF_HELPERS_TXT_H

#include <OAF/OafStdGlobal.h>

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextDocument>

#include <idl/IUnknown.h>
#include <idl/IIODevice.h>
#include <idl/ITextDocument.h>

#include <OAF/CUnknown.h>

namespace OAF
{
	struct CSaveParams;

	/**
	 * @brief Вспомогательный класс для создания вторичного устройства ввода/вывода
	 *        на основе заданного
	 *
	 * Такие сложности нужны, чтобы обеспечить трансляцию абсолютных путей для ссылок
	 * на файлы в относительные и обратно для загрузки/сохранения текстовых документов
	 * в/из XML-потока связанного с файлом.
	 */
	class OAFSTD_EXPORT CDeviceDerived : public QObject,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUnknown,
		virtual public OAF::IIODevice,
		//
		// Импортируемые реализации
		//
		virtual public CUnknown
	{
		Q_OBJECT

		/**
		 * @brief Основное устройство ввода/вывода
		 */
		URef<OAF::IIODevice> m_base;

		/**
		 * @brief Буфер для ввода/вывода
		 */
		QBuffer* m_buffer;

	public:
		CDeviceDerived (QByteArray* _data, OAF::IIODevice* _base = NULL);
		~CDeviceDerived ();

		/**
		 * @name Реализация интерфейса OAF::IIODevice
		 */
		/** @{*/
		QVariant getInfo (DeviceInfo _what);
		void setInfo (DeviceInfo _what, const QVariant& _v);
		QIODevice* device ();
		bool exists () const;
		/** @}*/
	};

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
	OAFSTD_EXPORT QXmlStreamWriter& saveTXTToXML (QXmlStreamWriter& _os, const QStringList& _mime_types, OAF::ITextDocument* _document, OAF::CSaveParams& _save_params);

	/**
	 * @brief Сделать аннотацию для QTextDocument
	 */
	OAFSTD_EXPORT QString getAnnonation (const QTextDocument& _doc);
}

#endif /* __OAF_HELPERS_TXT_H */
