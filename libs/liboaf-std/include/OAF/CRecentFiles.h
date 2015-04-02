/**
 * @file
 * @brief Интерфейс списка последних открытых файлов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_CRECENTFILES_H
#define __OAF_CRECENTFILES_H

#include <QtCore>

#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Список последних открытых/сохраненных файлов
	 */
	class OAFSTD_EXPORT CRecentFiles : public QObject
	{
		Q_OBJECT

		/**
		 * @brief Наименование организации, разработчика приложения
		 */
		QString m_org;

		/**
		 * @brief Наименование приложения
		 */
		QString m_app;

		/**
		 * @brief Ключ для сохранения списка файлов
		 */
		QString m_key;

		/**
		 * @brief Максимальное количество файлов в списке
		 */
		int m_max;

		/**
		 * @brief Список имен файлов <путь к файлу, название>
		 *
		 * В качестве названия может сохраняться произвольная текстовая
		 * информация, которая затем может использоваться для показа
		 * пользователю
		 */
		mutable QStack<QPair<QString, QString> > m_stack;

		/**
		 * @brief Глобальный список последних открытых файлов
		 */
		static QScopedPointer<CRecentFiles> m_recent_helper;

		/**
		 * @brief Проверить файлы в стеке
		 */
		bool check () const;

		/**
		 * @brief Сохранить список в конфигурации
		 */
		void save () const;

		CRecentFiles (const QString& _org, const QString& _app, const QString& _key, int _max);

	private slots:
		/**
		 * @brief Уведомление об изменении состояния
		 *
		 * Развязываем обработку изменения с реальным изменением состояния, так
		 * как изменение состояния может быть вызвано из меню, которое должно
		 * перестраиваться в реакции на это изменение.
		 */
		void aboutStateChanged ();

	signals:
		/**
		 * @brief Изменилось состояние списка
		 */
		void stateChanged ();

	public:
		/**
		 * @brief Добавить новый файл с его описанием
		 *
		 * Если описание не задано, то будет использовано имя файла
		 */
		void push (const QString& _path, const QString& _name = QString::null);

		/**
		 * @brief Очистить список файлов
		 */
		void clear ();

		/**
		 * @brief Получить список файлов
		 */
		const QStack<QPair<QString, QString> >& stack () const;

		/**
		 * @brief Глобальный список последних открытых файлов
		 */
		static CRecentFiles* instance (const QString& _org = QString::null,
									   const QString& _app = QString::null,
									   const QString& _key = QString::null,
									   int _max = 8);
	};
}

#endif /* __OAF_CRECENTFILES_H */
