/**
 * @file
 * @brief Реализация устройства бинарного ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QUrl>
#include <QCryptographicHash>

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
			if (!m_copy_id.isNull ())
				url.addQueryItem ("oaf_copy_id", m_copy_id.toString ());
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
		QUrl raw_data (_v.value<QString> ());

		//
		// Специальная обработка параметра as_copy
		//
		if (raw_data.hasQueryItem ("oaf_as_copy") &&
			(raw_data.queryItemValue ("oaf_as_copy").compare ("no"   , Qt::CaseInsensitive) != 0) &&
			(raw_data.queryItemValue ("oaf_as_copy").compare ("false", Qt::CaseInsensitive) != 0))
			m_copy_id = QUuid::createUuid ();

		//
		// Специальная обработка параметра oaf_copy_id
		//
		if (raw_data.hasQueryItem ("oaf_copy_id"))
			m_copy_id = QUuid (raw_data.queryItemValue ("oaf_copy_id"));

		//
		// Устанавливаем реальные данные
		//
		m_data = QByteArray::fromHex (raw_data.toString (QUrl::RemoveScheme|QUrl::RemoveQuery).toAscii ());
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
