/**
 * @file
 * @brief Реализация классов и функций для загрузки значков MIME-типов
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>

#include <OAF/MimeHelpers.h>
#include <OAF/Utils.h>

#include <OAF/IconHelpers.h>

QImage
OAF::CIconDatabase::findImageByName (const QString& _name)
{
	//
	// Сначала ищем в кэше значков
	//
	QMap<QString, QImage>::const_iterator ci = m_icons.find (_name);
	if (ci != m_icons.end ())
		return ci.value ();

	//
	// Если значок не найден, то ищем его по заданному списку путей
	//
	for (QList<QDir>::const_iterator idir = m_icon_dirs.begin (); idir != m_icon_dirs.end (); ++idir)
	{
		//
		// Имя значка с учётом заданной директории (пока нас интересует только файлы в формате png)
		//
		QFileInfo icon_file (idir->filePath (_name + ".png"));

		//
		// Если такой файл существует, то загружаем его в кэш и возвращаем в качестве результата
		//
		if (icon_file.isFile () && icon_file.isReadable ())
			return m_icons.insert (_name, QImage (icon_file.filePath ())).value ();
	}

	//
	// Значок не найден
	//
	return QImage ();
}

OAF::CIconDatabase::CIconDatabase ()
{
	QProcessEnvironment pe = QProcessEnvironment::systemEnvironment ();

	//
	// Проверяем наличие переменной $OAFICONS
	//
	if (pe.contains ("OAFICONS"))
	{
		QString icon_dir_value = pe.value ("OAFICONS");
		QStringList icon_dirs = icon_dir_value.split (";", QString::SkipEmptyParts);

		foreach (const QString& s, icon_dirs)
		{
			QDir d (s);
			if (d.exists ())
				m_icon_dirs.append (d);
		}
	}
	else
	{
		//
		// Переменная не задана: остаётся только телепатическим путём найти каталог share
		//
		m_icon_dirs.append (QDir (OAF::defaultDataPath () + QDir::separator () + "icons"));
	}
}

OAF::CIconDatabase&
OAF::CIconDatabase::instance ()
{
	static CIconDatabase icd;
	return icd;
}

QImage
OAF::CIconDatabase::mimeImage (const MimeTypeInfo& _mime_type)
{
	if (_mime_type.isValid ())
	{
		//
		// 1) Проверяем существование файла, указанного в атрибуте icon MIME-типа
		//
		if (!_mime_type.icon.isEmpty ())
		{
			QImage result = findImageByName (_mime_type.icon);
			if (!result.isNull ())
				return result;
		}

		//
		// 2) Проверяем существование файла, указанного в атрибуте generic-icon MIME-типа
		//
		if (!_mime_type.generic_icon.isEmpty ())
		{
			QImage result = findImageByName (_mime_type.generic_icon);
			if (!result.isNull ())
				return result;
		}

		//
		// 3) Формируем название значка (например, "application-pdf")
		//
		QString mime_icon_name = _mime_type.mime_type;
		mime_icon_name.replace ("/", "-");
		QImage result = findImageByName (mime_icon_name);
		if (!result.isNull ())
			return result;
	}

	//
	// 4) Если не нашли значок с таким путём, то вернём значок по умолчанию
	//
	return findImageByName ("unknown");
}

QIcon
OAF::CIconDatabase::mimeIcon (const MimeTypeInfo& _mime_type)
{
	return QIcon (QPixmap::fromImage (mimeImage (_mime_type)));
}

void
OAF::convertToGrayscale (QImage& _img)
{
	for (int r = 0; r < _img.height (); ++r)
	{
		for (int c = 0; c < _img.width (); ++c)
		{
			QRgb rgba = _img.pixel (c, r);

			int gray = qGray  (rgba);
			int a    = qAlpha (rgba);

			_img.setPixel (c, r, qRgba (gray, gray, gray, a));
		}
	}
}
