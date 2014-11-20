/**
 * @file
 * @brief Реализация вспомогательных функций
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QApplication>

#include <OAF/Utils.h>

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

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

int
OAF::findById (QComboBox* _c, const QUuid& _id)
{
	int i = 0;
	for (; i < _c->count (); ++i)
	{
		if (_c->itemData (i).value<QUuid> () == _id)
			break;
	}
	if (i == _c->count ())
		i = -1;

	return i;
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

QImage
OAF::getImage (const QUrl& _url)
{
	//
	// Внутренний кэш картинок, загруженных из внешних ресурсов
	//
	static QMap<QUrl, QImage> cache;

	//
	// Ищем картинку в кэше и если находим, то возвращаем
	//
	QMap<QUrl, QImage>::const_iterator i = cache.find (_url);
	if (i != cache.end ())
		return i.value ();

	//
	// Вставляем картинку в кэш и возвращаем её
	//
	return (cache[_url] = QImage (OAF::toLocalFile (_url)));
}

QString
OAF::defaultDataPath ()
{
#ifdef Q_OS_UNIX
		QString piket_data_path = "/usr/share/qpiket/";
#else
		QString piket_data_path = qApp->applicationDirPath ();
		QDir piket_share_dir (piket_data_path);
		if (piket_share_dir.cd (QString ("..") + QDir::separator () + "share"))
			piket_data_path = piket_share_dir.path ();

		if (!piket_data_path.endsWith (QDir::separator ()))
			piket_data_path += QDir::separator ();
#endif

		return piket_data_path;
}

void
OAF::sleep (int _ms)
{
	Q_ASSERT (_ms > 0);

#ifdef Q_OS_WIN
	Sleep (static_cast<uint> (_ms));
#else
	struct timespec ts = {_ms/1000, (_ms%1000)*1000*1000};
	nanosleep (&ts, NULL);
#endif
}
