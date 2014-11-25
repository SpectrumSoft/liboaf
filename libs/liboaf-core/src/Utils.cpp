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
#include <QApplication>

#include <OAF/StreamUtils.h>
#include <OAF/Utils.h>

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

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
