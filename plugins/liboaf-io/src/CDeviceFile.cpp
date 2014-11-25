/**
 * @file
 * @brief Реализация устройства ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QUrl>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDateTime>

#include <OAF/StreamUtils.h>
#include <OAF/MimeHelpers.h>

#include "CDeviceFile.h"

using namespace OAF::IO;

CDeviceFile::CDeviceFile (CFactory* _factory) : CUnknown (device_file_cid), m_factory (_factory)
{
	m_file = new QFile (this);
}

CDeviceFile::~CDeviceFile ()
{}

QVariant
CDeviceFile::getInfo (DeviceInfo _what)
{
	QVariant res;

	switch (_what)
	{
		case PATH:
		{
			QUrl url = OAF::fromLocalFile (QFileInfo (*m_file).absoluteFilePath ());
			if (!m_copy_id.isNull ())
				url.addQueryItem ("oaf_copy_id", m_copy_id.toString ());
			res = url.toString ();
			break;
		}

		case IDENTITY:
		{
			QCryptographicHash hash (QCryptographicHash::Sha1);
			QFile file (QFileInfo (*m_file).absoluteFilePath ());
			if (file.open (QIODevice::ReadOnly))
				hash.addData (file.readAll ());
			res = QString (hash.result ().toHex ());
			break;
		}

		case CONTENT_TYPE:
		{
			res = OAF::CMimeDatabase::instance ().lookupMimeTypes (QFileInfo (m_file->fileName ()).suffix (),
								   QFileInfo (m_file->fileName ()).size (),
								   m_file);
			break;
		}

		case LAST_MODIFIED:
		{
			res = QFileInfo (*m_file).lastModified ();
			break;
		}

		default: break;
	}

	return res;
}

void
CDeviceFile::setInfo (DeviceInfo _what, const QVariant& _v)
{
	//
	// Если задана установка пути к файлу
	//
	if (_what == PATH)
	{
		//
		// Разбираем переданный URI
		//
		QUrl file_name (_v.value<QString> ());

		//
		// Специальная обработка параметра as_copy
		//
		if (file_name.hasQueryItem ("oaf_as_copy") &&
			(file_name.queryItemValue ("oaf_as_copy").compare ("no"   , Qt::CaseInsensitive) != 0) &&
			(file_name.queryItemValue ("oaf_as_copy").compare ("false", Qt::CaseInsensitive) != 0))
			m_copy_id = QUuid::createUuid ();

		//
		// Специальная обработка параметра oaf_copy_id
		//
		if (file_name.hasQueryItem ("oaf_copy_id"))
			m_copy_id = QUuid (file_name.queryItemValue ("oaf_copy_id"));

		//
		// Устанавливаем реальное имя файла
		//
		m_file->setFileName (OAF::toLocalFile (file_name));
	}
}

QIODevice*
CDeviceFile::device ()
{
	return m_file;
}

bool
CDeviceFile::exists () const
{
	return m_file->exists ();
}
