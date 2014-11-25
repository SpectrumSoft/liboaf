/**
 * @file
 * @brief Реализация вспомогательных классов и функций для ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QDir>
#include <QFileInfo>

#include <OAF/StreamUtils.h>

using namespace OAF;

static const QString device_derived_cid = "OAF/DeviceDerived:1.0";

CDeviceDerived::CDeviceDerived (QByteArray* _data, OAF::IIODevice* _base) : CUnknown (device_derived_cid), m_base (_base)
{
	m_buffer = new QBuffer (this);
	m_buffer->setBuffer (_data);
}

CDeviceDerived::~CDeviceDerived ()
{}

QVariant
CDeviceDerived::getInfo (DeviceInfo _what)
{
	if (m_base)
		return m_base->getInfo (_what);

	//
	// В остальных случаях результат не определён
	//
	return QVariant ();
}

void
CDeviceDerived::setInfo (DeviceInfo _what, const QVariant& _v)
{
	if (m_base)
		m_base->setInfo (_what, _v);
}

QIODevice*
CDeviceDerived::device ()
{
	return m_buffer;
}

bool
CDeviceDerived::exists () const
{
	return (m_base ? m_base->exists () : false);
}

QString
OAF::makeRelativePath (const QString& _origin, const QString& _absolute)
{
	//
	// Разбираем оба пути
	//
	QUrl origin (_origin);
	QUrl absolute (_absolute);

	//
	// Если у обоих путей совпадают схемы и параметры доступа
	//
	if ((origin.scheme () == absolute.scheme ()) && (origin.authority () == absolute.authority ()))
	{
		//
		// Определяем путь ко второму файлу относительно первого
		//
		return QDir::cleanPath (QFileInfo (origin.path ()).dir ().relativeFilePath (absolute.path ()));
	}

	return _absolute;
}

QString
OAF::makeAbsolutePath (const QString& _origin, const QString& _relative)
{
	//
	// Разбираем оба пути
	//
	QUrl origin   (_origin);
	QUrl relative (_relative);

	//
	// Если второй путь не имеет схемы и параметров доступа
	//
	if (relative.scheme ().isEmpty () && relative.authority ().isEmpty ())
	{
		//
		// Копируем во второй путь схему и параметры доступа первого
		//
		relative.setScheme (origin.scheme ());
		relative.setAuthority (origin.authority ());
		relative.setEncodedQuery (origin.encodedQuery ());

		//
		// И формируем его путь с учётом пути к первому файлу
		//
		relative.setPath (QDir::cleanPath (QFileInfo (origin.path ()).dir ().absoluteFilePath (relative.path ())));

		//
		// Возвращаем полученный абсолютный путь к файлу
		//
		return relative.toString ();
	}

	return _relative;
}

QUrl
OAF::fromLocalFile (const QString& _local_file)
{
	QUrl url;

	url.setScheme (QLatin1String ("file"));

	QString deslashified = _local_file;
	deslashified.replace (QLatin1Char ('\\'), QLatin1Char ('/'));

	//
	// magic for drives on windows
	//
	if ((deslashified.length () > 1) && (deslashified.at (1) == QLatin1Char (':')) && (deslashified.at (0) != QLatin1Char ('/')))
	{
		url.setPath (QLatin1Char('/') + deslashified);
	}
	else if (deslashified.startsWith (QLatin1String("//")))
	{
		int index_of_path = deslashified.indexOf (QLatin1Char ('/'), 2);

		url.setHost(deslashified.mid (2, index_of_path - 2));
		if (index_of_path > 2)
			url.setPath (deslashified.right (deslashified.length () - index_of_path));
	}
	else
	{
		url.setPath (deslashified);
	}

	return url;
}

QString
OAF::toLocalFile (const QUrl& _url)
{
	QString tmp;

	QString our_path = _url.path ();
	if (_url.scheme ().isEmpty() || (_url.scheme ().toLower () == QLatin1String ("file")))
	{
		//
		// magic for shared drive on windows
		//
		if (!_url.host ().isEmpty ())
		{
			tmp = QLatin1String ("//") + _url.host () + ((our_path.length () > 0) && (our_path.at (0) != QLatin1Char('/')) ?
															 QLatin1String ("/") + our_path : our_path);
		}
		else
		{
			tmp = our_path;

			//
			// magic for drives on windows
			//
			if ((our_path.length () > 2) && (our_path.at (0) == QLatin1Char ('/')) && (our_path.at (2) == QLatin1Char (':')))
				tmp.remove (0, 1);
		}
	}

	return tmp;
}
