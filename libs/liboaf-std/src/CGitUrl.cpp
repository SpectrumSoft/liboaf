/**
 * @file
 * @brief Реализация класса для работы с URL для git-репозитория
 *        (вида git:///path/to/file.txt<?hash=><oaf_copy_id=|oaf_as_copy>)
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <git2.h>

#include <OAF/StreamUtils.h>
#include <OAF/CGitUrl.h>

using namespace OAF;

CGitUrl::CGitUrl ()
{}

CGitUrl::CGitUrl (const CGitUrl& _url)
{
	*this = _url;
}

CGitUrl::CGitUrl (const QUrl& _url)
{
	*this = _url;
}

CGitUrl&
CGitUrl::operator= (const CGitUrl& _url)
{
	m_url     = _url.m_url;
	m_copy_id = _url.m_copy_id;
	m_hash    = _url.m_hash;
	m_path    = _url.m_path;

	return *this;
}

CGitUrl&
CGitUrl::operator= (const QUrl& _url)
{
	//
	// Очищаем старые данные
	//
	m_url.clear ();
	m_copy_id = QUuid ();
	m_hash.clear ();
	m_path.clear ();

	//
	// Сохраняем URL
	//
	m_url = _url;

	//
	// Разбираем параметры
	//
#if (QT_VERSION < QT_VERSION_CHECK (5, 0, 0))
	QUrl query (_url);
#else
	QUrlQuery query (_url);
#endif

	//
	// Специальная обработка параметра as_copy
	//
	if (query.hasQueryItem ("oaf_as_copy") &&
		(query.queryItemValue ("oaf_as_copy").compare ("no"   , Qt::CaseInsensitive) != 0) &&
		(query.queryItemValue ("oaf_as_copy").compare ("false", Qt::CaseInsensitive) != 0))
	{
		//
		// Генерируем новый уникальный идентификатор копии
		//
		m_copy_id = QUuid::createUuid ();
		//
		// ... и заменяем на него oaf_as_copy
		//
		query.removeQueryItem ("oaf_as_copy");
		query.addQueryItem ("oaf_copy_id", m_copy_id.toString ());
	}

	//
	// Специальная обработка параметра oaf_copy_id
	//
	if (query.hasQueryItem ("oaf_copy_id"))
		m_copy_id = QUuid (query.queryItemValue ("oaf_copy_id"));

	//
	// Специальная обработка параметра hash (SHA1-идентификатора объекта в git-репозитории)
	//
	if (query.hasQueryItem ("hash"))
		m_hash = query.queryItemValue ("hash");

	//
	// Устанавливаем новые параметры
	//
#if (QT_VERSION < QT_VERSION_CHECK (5, 0, 0))
	m_url.setEncodedQuery (query.encodedQuery ());
#else
	m_url.setQuery (query);
#endif

	//
	// Абсолютный путь и имя файла для извлечения из репозитория
	//
	// NOTE: под Windows нужно три разделителя, т.к. только в Unix все пути начинаются с корня ("/")
	//
	m_path = _url.toString (QUrl::RemoveScheme|QUrl::RemoveQuery);
#ifdef Q_OS_WIN32
	if (m_file_name.startsWith ("///"))
		m_file_name.remove (0, 3);
#else
	if (m_path.startsWith ("//"))
		m_path.remove (0, 2);
#endif

	if (!QFileInfo (m_path).exists ())
		qWarning ("File does not exists: %s", qPrintable (m_path));

	return *this;
}

QString
CGitUrl::url () const
{
	return m_url.toString ();
}

QString
CGitUrl::commit () const
{
	return m_hash;
}

QString
CGitUrl::path () const
{
	return m_path;
}

QString
CGitUrl::ext () const
{
	return QFileInfo (m_path).suffix ();
}

QString
CGitUrl::relativeRepoPath (git_repository* _repo) const
{
	Q_ASSERT (_repo != NULL);

	//
	// Получаем путь до директории, где был обнаружен git-репозиторий (/path/.git),
	// и поднимаемся на уровень вверх (чтобы получить корректный путь
	// к файлу вида docs/PIKET-TZ.pkm)
	//
	QDir repo_dir (git_repository_path (_repo));
	if (!repo_dir.cdUp ())
		qWarning () << Q_FUNC_INFO << "Incorrect git repo directory";

	//
	// Получаем путь к заданному файлу относительно git-репозитория, в котором он хранится
	//
	QString path = OAF::makeRelativePath (OAF::fromLocalFile (repo_dir.path ()).toString (),
										  OAF::fromLocalFile (m_path).toString ());

	//
	// Убираем самый верхний каталог (чтобы получить docs/PIKET-TZ.pkm вместо qpiket.git/docs/PIKET-TZ.pkm)
	//
	// NOTE: использование прямого слэша кроссплатформенно
	//
	path.remove (0, path.indexOf ('/') + 1);

	return path;
}
