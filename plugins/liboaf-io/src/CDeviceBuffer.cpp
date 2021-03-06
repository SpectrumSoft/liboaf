/**
 * @file
 * @brief Реализация устройства бинарного ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CDeviceBuffer.h"

using namespace OAF::IO;

CDeviceBuffer::CDeviceBuffer (CFactory* _factory) : CUnknown (device_buffer_cid), m_factory (_factory)
{
	m_buffer = new QBuffer (this);
	m_buffer->setBuffer (&m_data);
}

CDeviceBuffer::~CDeviceBuffer ()
{}

QVariant
CDeviceBuffer::getInfo (DeviceInfo _what)
{
	QVariant res;

	switch (_what)
	{
		case PATH:
		{
			QUrl url (QString ("raw:%1").arg (QString (m_data.toHex ())));

			//
			// Если задан идентификатор копии
			//
			if (!m_copy_id.isNull ())
			{
				//
				// Параметры URL
				//
#if (QT_VERSION < QT_VERSION_CHECK (5, 0, 0))
				QUrl query (url);
#else
				QUrlQuery query (url);
#endif

				//
				// Добавляем идентификатор копии
				//
				query.addQueryItem ("oaf_copy_id", m_copy_id.toString ());

				//
				// Устанавливаем новые параметры
				//
#if (QT_VERSION < QT_VERSION_CHECK (5, 0, 0))
				url.setEncodedQuery (query.encodedQuery ());
#else
				url.setQuery (query);
#endif
			}

			res = url.toString ();
			break;
		}

		case IDENTITY:
		{
			QCryptographicHash hash (QCryptographicHash::Sha1);
			hash.addData (m_data);
			res = QString (hash.result ().toHex ());
			break;
		}

		case CONTENT_TYPE:
		{
			res = QStringList ("application/octet-stream");
			break;
		}

		case LAST_MODIFIED:
		{
			//
			// Для бинарного потока невозможно определить дату его создания, так что вернем
			// пустой (невалидный) QVariant
			//
			break;
		}

		default:
			break;
	}

	return res;
}

void
CDeviceBuffer::setInfo (DeviceInfo _what, const QVariant& _v)
{
	if (_what == PATH)
	{
		//
		// Разбираем переданный URI
		//
		QUrl url (_v.value<QString> ());

		//
		// Разбираем параметры
		//
#if (QT_VERSION < QT_VERSION_CHECK (5, 0, 0))
		QUrl query (url);
#else
		QUrlQuery query (url);
#endif

		//
		// Специальная обработка параметра as_copy
		//
		if (query.hasQueryItem ("oaf_as_copy") &&
			(query.queryItemValue ("oaf_as_copy").compare ("no"   , Qt::CaseInsensitive) != 0) &&
			(query.queryItemValue ("oaf_as_copy").compare ("false", Qt::CaseInsensitive) != 0))
			m_copy_id = QUuid::createUuid ();

		//
		// Специальная обработка параметра oaf_copy_id
		//
		if (query.hasQueryItem ("oaf_copy_id"))
			m_copy_id = QUuid (query.queryItemValue ("oaf_copy_id"));

		//
		// Устанавливаем реальные данные
		//
		m_data = QByteArray::fromHex (url.toString (QUrl::RemoveScheme|QUrl::RemoveQuery).toLatin1 ());
	}
}

QIODevice*
CDeviceBuffer::device ()
{
	return m_buffer;
}

bool
CDeviceBuffer::exists () const
{
	return false;
}
