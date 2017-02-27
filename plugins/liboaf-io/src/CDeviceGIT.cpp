/**
 * @file
 * @brief Реализация устройства ввода/вывода для объектов git-репозитория
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <git2.h>

#include <OAF/HelpersMIME.h>

#include "CDeviceGIT.h"

using namespace OAF::IO;

/**
 * @brief Возвращает строковое представление для указанного кода ошибки git
 * @param _error_code Код ошибки git
 * @param _action Действие, при котором произошла ошибка
 */
static QString
getGitError (int _error_code, const QString& _action)
{
	QString git_error_str;

	if (_error_code)
	{
		const git_error* error = giterr_last ();
		git_error_str = QString (QTranslator::tr ("Error with code %1 was occured during %2:\n %3"))
				.arg (_error_code)
				.arg (_action)
				.arg ((error && error->message) ? QString::fromUtf8 (error->message) : "<Unknown>");
	}
	else
		git_error_str = QTranslator::tr ("Error during ") + _action + QTranslator::tr (" was occured");

	return git_error_str;
}

/**
 * @brief Возвращает данные указанного файла в ревизии указанного коммита
 */
static void
readFileRevisionData (QByteArray& _data, const OAF::CGitUrl& _url)
{
	//
	// Описание ошибки Git
	//
	int error_code = GIT_OK;
	//
	// git-репозиторий, в котором находится заданный файл
	//
	git_repository* repo = NULL;
	//
	// Версия заданного файла, связанная с заданным коммитом
	//
	git_blob* blob = NULL;
	//
	// Абстрактный объект, хранящийся в git-репозитории (должен быть блобом)
	//
	git_object* obj = NULL;

	try
	{
		//
		// Открываем Git-репозиторий, в котором находится файл
		//
		error_code = git_repository_open_ext (&repo, QFile::encodeName (_url.path ()), GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
		if (error_code != GIT_OK)
			throw QTranslator::tr ("opening repository");

		//
		// Получаем путь до директории, где был обнаружен git-репозиторий,
		// и поднимаемся на уровень вверх (чтобы получить корректный путь
		// к файлу вида docs/PIKET-TZ.pkm)
		//
		QString file_path = _url.relativeRepoPath (repo);

		//
		// Открываем версию указанного файла из коммита, идентифицируемый заданным _commit_id,
		// в качестве абстрактного объекта git
		//
		// NOTE: формат доступа к конкретному файлу коммита таков: <commit_hash>:<relative path to file>
		//
		error_code = git_revparse_single (&obj, repo, QFile::encodeName (_url.commit () + ":" + file_path));
		//
		// Найденный объект обязан быть блобом
		//
		if (obj && (git_object_type (obj) != GIT_OBJ_BLOB))
			error_code = GIT_ENOTFOUND;

		//
		// Завершаем обработку по ошибке
		//
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching object with specified SHA-1 id");

		//
		// Получив объект, по его id находим собственно файл (blob)
		//
		error_code = git_blob_lookup (&blob, repo, git_object_id (obj));
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching blob with specified SHA-1 id");

		//
		// Загружаем "сырые" данные файла из коммита
		//
		_data = QByteArray ((const char*)git_blob_rawcontent (blob), git_blob_rawsize (blob));
	}
	catch (const QString& _action)
	{
		qWarning ("%s : %s", Q_FUNC_INFO, qPrintable (getGitError (error_code, _action)));
	}

	//
	// Освобождаем все ранее распределённые ресурсы libgit2
	//
	git_object_free (obj);
	git_blob_free (blob);
	git_repository_free (repo);
}

/**
 * @brief Проверяет, есть ли указанный файл в ревизии указанного коммита
 */
static bool
fileInRevisionExists (const OAF::CGitUrl& _url)
{
	//
	// Описание ошибки Git
	//
	int error_code = GIT_OK;
	//
	// git-репозиторий, в котором находится заданный файл
	//
	git_repository* repo = NULL;
	//
	// Версия заданного файла, связанная с заданным коммитом
	//
	git_blob* blob = NULL;
	//
	// Абстрактный объект, хранящийся в git-репозитории (должен быть блобом)
	//
	git_object* obj = NULL;

	try
	{
		//
		// Открываем Git-репозиторий, в котором находится файл
		//
		error_code = git_repository_open_ext (&repo, QFile::encodeName (_url.path ()), GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
		if (error_code != GIT_OK)
			throw QTranslator::tr ("opening repository");

		//
		// Получаем путь до директории, где был обнаружен git-репозиторий,
		// и поднимаемся на уровень вверх (чтобы получить корректный путь
		// к файлу вида docs/PIKET-TZ.pkm)
		//
		QString file_path = _url.relativeRepoPath (repo);

		//
		// Открываем версию указанного файла из коммита, идентифицируемый заданным _commit_id,
		// в качестве абстрактного объекта git
		//
		// NOTE: формат доступа к конкретному файлу коммита таков: <commit_hash>:<relative path to file>
		//
		error_code = git_revparse_single (&obj, repo, QFile::encodeName (_url.commit () + ":" + file_path));
		//
		// Найденный объект обязан быть блобом
		//
		if (obj && (git_object_type (obj) != GIT_OBJ_BLOB))
			error_code = GIT_ENOTFOUND;

		//
		// Завершаем обработку по ошибке
		//
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching object with specified SHA-1 id");

		//
		// Получив объект, по его id находим собственно файл (blob); ну или не находим
		//
		error_code = git_blob_lookup (&blob, repo, git_object_id (obj));
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching blob with specified SHA-1 id");
	}
	catch (const QString& _action)
	{
		qWarning ("%s : %s", Q_FUNC_INFO, qPrintable (getGitError (error_code, _action)));
	}

	//
	// Освобождаем все ранее распределённые ресурсы libgit2
	//
	git_object_free (obj);
	git_blob_free (blob);
	git_repository_free (repo);

	return (error_code == GIT_OK);
}

/**
 * @brief Возвращает дату создания коммита, если он содержит указанный файл; иначе вернет пустую дату
 */
static QDateTime
fileInRevisionDate (const OAF::CGitUrl& _url)
{
	QDateTime commit_date;
	if (!fileInRevisionExists (_url))
		return commit_date;

	//
	// Описание ошибки Git
	//
	int error_code = GIT_OK;
	//
	// git-репозиторий, в котором находится заданный файл
	//
	git_repository* repo = NULL;
	//
	// Версия заданного файла, связанная с заданным коммитом
	//
	git_blob* blob = NULL;
	//
	// Абстрактный объект, хранящийся в git-репозитории (первый должен быть блобом, второй - коммитом)
	//
	git_object* obj = NULL, *obj_commit = NULL;
	//
	// Собственно коммит
	//
	git_commit* commit = NULL;

	try
	{
		//
		// Открываем Git-репозиторий, в котором находится файл
		//
		error_code = git_repository_open_ext (&repo, QFile::encodeName (_url.path ()), GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
		if (error_code != GIT_OK)
			throw QTranslator::tr ("opening repository");

		//
		// Получаем путь до директории, где был обнаружен git-репозиторий,
		// и поднимаемся на уровень вверх (чтобы получить корректный путь
		// к файлу вида docs/PIKET-TZ.pkm)
		//
		QString file_path = _url.relativeRepoPath (repo);

		//
		// Открываем версию указанного файла из коммита, идентифицируемый заданным _commit_id,
		// в качестве абстрактного объекта git
		//
		// NOTE: формат доступа к конкретному файлу коммита таков: <commit_hash>:<relative path to file>
		//
		error_code = git_revparse_single (&obj, repo, QFile::encodeName (_url.commit () + ":" + file_path));
		//
		// Найденный объект обязан быть блобом
		//
		if (obj && (git_object_type (obj) != GIT_OBJ_BLOB))
			error_code = GIT_ENOTFOUND;

		//
		// Завершаем обработку по ошибке
		//
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching object with specified SHA-1 id");

		//
		// Получив объект, по его id находим собственно файл (blob)
		//
		error_code = git_blob_lookup (&blob, repo, git_object_id (obj));
		if (error_code != GIT_OK)
			throw QTranslator::tr ("searching blob with specified SHA-1 id");

		//
		// Получаем объект коммита
		//
		error_code = git_revparse_single (&obj_commit, repo, QFile::encodeName (_url.commit ()));
		//
		// Найденный объект обязан быть коммитом
		//
		if (obj_commit && (git_object_type (obj_commit) != GIT_OBJ_COMMIT))
			error_code = GIT_ENOTFOUND;
		commit = reinterpret_cast <git_commit *> (obj);
		//
		// Получаем дату коммита
		//
		commit_date = QDateTime::fromTime_t (git_commit_time (commit));
	}
	catch (const QString& _action)
	{
		qWarning ("%s : %s", Q_FUNC_INFO, qPrintable (getGitError (error_code, _action)));
	}

	//
	// Освобождаем все ранее распределённые ресурсы libgit2
	//
	git_object_free (obj);
	git_object_free (obj_commit);
	git_blob_free (blob);
	git_repository_free (repo);

	return commit_date;
}

CDeviceGIT::CDeviceGIT (CFactory* _factory) : CUnknown (device_git_cid), m_factory (_factory)
{
	//
	// Указываем буферу, откуда следует забирать данные
	//
	m_buffer = new QBuffer (this);
	m_buffer->setBuffer (&m_data);
}

CDeviceGIT::~CDeviceGIT ()
{}

QVariant
CDeviceGIT::getInfo (DeviceInfo _what)
{
	QVariant res;

	switch (_what)
	{
		case PATH:
		{
			res = m_git_url.url ();
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
			res = OAF::CMimeDatabase::instance ().lookupMimeTypes (m_git_url.ext (),
																   m_buffer->size (), m_buffer);
			break;
		}

		case LAST_MODIFIED:
		{
			//
			// За дату последнего изменения файла примем дату создания коммита,
			// которому он принадлежит
			//
			res = fileInRevisionDate (m_git_url);
			break;
		}

		default:
			break;
	}

	return res;
}

void
CDeviceGIT::setInfo (DeviceInfo _what, const QVariant& _v)
{
	if (_what == PATH)
	{
		m_git_url = QUrl (_v.toString ());

		//
		// Считываем указанную ревизию файла из git-репозитория
		//
		readFileRevisionData (m_data, m_git_url);
	}
}

QIODevice*
CDeviceGIT::device ()
{
	return m_buffer;
}

bool
CDeviceGIT::exists () const
{
	return fileInRevisionExists (m_git_url);
}

