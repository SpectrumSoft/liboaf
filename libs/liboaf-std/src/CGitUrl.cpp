/**
 * @file
 * @brief Реализация класса для работы с URL для git-репозитория
 *        (вида git:///path/to/file.txt<?hash=><oaf_copy_id=|oaf_as_copy>)
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <git2.h>

#include <OAF/Utils.h>
#include <OAF/CGitUrl.h>

OAF::CGitUrl&
OAF::CGitUrl::operator= (const QUrl& _other)
{
	//
	// Защищаемся от последствий самоприсваивания
	//
	if (m_git_url != _other)
	{
		//
		// Очищаем старые данные
		//
		m_git_url.clear ();
		m_copy_id = QUuid ();
		m_hash.clear ();
		m_file_name.clear ();

		//
		// Разбираем переданный URI и сохраняем интересующие нас части в переменных класса,
		// для последующего быстрого обращения к ним
		//
		m_git_url = _other;
		Q_ASSERT (m_git_url.isValid ());

		//
		// Специальная обработка параметра as_copy
		//
		if (_other.hasQueryItem ("oaf_as_copy") &&
				(_other.queryItemValue ("oaf_as_copy").compare ("no"   , Qt::CaseInsensitive) != 0) &&
				(_other.queryItemValue ("oaf_as_copy").compare ("false", Qt::CaseInsensitive) != 0))
		{
			//
			// Генерируем новый уникальный идентификатор копии
			//
			m_copy_id = QUuid::createUuid ();
			//
			// ...и заменяем на него oaf_as_copy
			//
			m_git_url.removeQueryItem ("oaf_as_copy");
			m_git_url.addQueryItem ("oaf_copy_id", m_copy_id.toString ());
		}

		//
		// Специальная обработка параметра oaf_copy_id
		//
		if (_other.hasQueryItem ("oaf_copy_id"))
			m_copy_id = QUuid (_other.queryItemValue ("oaf_copy_id"));

		//
		// Специальная обработка параметра hash (SHA1-идентификатора объекта в git-репозитории)
		//
		if (_other.hasQueryItem ("hash"))
			m_hash = _other.queryItemValue ("hash");

		//
		// Абсолютное путь и имя файла для извлечения из репозитория
		//
		// NOTE: под Windows нужно три разделителя, т.к. только в Unix все пути начинаются с корня ("/")
		//
		m_file_name = _other.toString (QUrl::RemoveScheme | QUrl::RemoveQuery);
	#ifdef Q_OS_WIN32
		if (m_file_name.startsWith ("///"))
			m_file_name.remove (0, 3);
	#else
		if (m_file_name.startsWith ("//"))
			m_file_name.remove (0, 2);
	#endif

		if (!QFileInfo (m_file_name).exists ())
			qWarning () << Q_FUNC_INFO << "File does not exists:" << m_file_name;
	}

	//
	// Возвращаем ссылку на себя согласно общепринятой практике (для использования сложных цепочек присваивания)
	//
	return (*this);
}

QString
OAF::CGitUrl::urledPath () const
{
	return m_git_url.toString ();
}

QString
OAF::CGitUrl::commitId () const
{
	return m_hash;
}

QString
OAF::CGitUrl::fileName () const
{
	return m_file_name;
}

QString
OAF::CGitUrl::fileExt () const
{
	return QFileInfo (m_file_name).suffix ();
}

QString
OAF::CGitUrl::relativeRepoPath (git_repository* _repo) const
{
	Q_ASSERT (_repo);
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
	QString rel_file_path = OAF::makeRelativePath (OAF::fromLocalFile (repo_dir.path ()).toString (),
												   OAF::fromLocalFile (fileName ()).toString ());
	//
	// Убираем самый верхний каталог (чтобы получить docs/PIKET-TZ.pkm вместо qpiket.git/docs/PIKET-TZ.pkm)
	//
	// NOTE: использование прямого слэша кроссплатформенно
	//
	rel_file_path.remove (0, rel_file_path.indexOf ('/') + 1);

	return rel_file_path;
}
