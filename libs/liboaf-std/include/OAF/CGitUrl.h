/**
 * @file
 * @brief Интерфейс класса для работы с URL для git-репозитория
 *        (вида git:///path/to/file.txt?<hash=><oaf_copy_id=|oaf_as_copy>)
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_CGITURL_H
#define __OAF_CGITURL_H

#include <QtCore>

#include <OAF/OafStdGlobal.h>

struct git_repository;

namespace OAF
{
	/**
	 * @brief Вспомогательный класс, который лихо управляется с URL вида git:///path/to/file.txt
	 * или git:///c:/path/to/file.txt - в отличие от стандартного QUrl, который однако
	 * (с костылями) всё равно используется внутри
	 */
	class OAFSTD_EXPORT CGitUrl
	{
		/**
		 * @brief Полный URL вместе с параметрами
		 */
		QUrl m_url;

		/**
		 * @brief Нужна ли копия данных
		 *
		 * Копия данных реализуется через добавление специального URL параметра oaf_copy_id={uuid}
		 * для каждой ссылки. При существующем механизме открытия данных это гарантирует,
		 * что будет открыта копия данных, а не получена ссылка на уже загруженные данные.
		 */
		QUuid m_copy_id;

		/**
		 * @brief Идентификатор ревизии, из которой будет загружен объект
		 */
		QString m_hash;

		/**
		 * @brief Путь и имя файла в репозитории
		 */
		QString m_path;

	public:
		CGitUrl ();
		CGitUrl (const CGitUrl& _url);
		CGitUrl (const QUrl& _url);

		CGitUrl& operator= (const CGitUrl& _url);
		CGitUrl& operator= (const QUrl& _url);

		/**
		 * @brief Полный URL файла вместе с параметрами
		 */
		QString url () const;

		/**
		 * @brief Идентификатор коммита
		 */
		QString commit () const;

		/**
		 * @brief Имя файла
		 */
		QString path () const;

		/**
		 * @brief Расширение файла
		 */
		QString ext () const;

		/**
		 * @brief Возвращает путь файла относительно указанного git-репозитория (например, docs/PIKET-TZ.pkm)
		 */
		QString relativeRepoPath (git_repository* _repo) const;
	};
}

#endif /* __OAF_CGITURL_H */
