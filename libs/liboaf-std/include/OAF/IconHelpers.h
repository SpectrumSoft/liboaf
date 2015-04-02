/**
 * @file
 * @brief Классы и функции для загрузки значков MIME-типов
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_ICONHELPERS_H
#define __OAF_ICONHELPERS_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <OAF/OafStdGlobal.h>

namespace OAF
{
	struct MimeTypeInfo;

	/**
	 * @brief Коллекция значков, указанных Пикет при запуске в переменной среды $OAFMIMES
	 */
	class OAFSTD_EXPORT CIconDatabase
	{
		Q_DISABLE_COPY (CIconDatabase)

		/**
		 * @brief Список директорий, в которых ищутся значки
		 */
		QList<QDir> m_icon_dirs;

		/**
		 * @brief Кэш значков
		 */
		QMap<QString, QImage> m_icons;

		/**
		 * @brief Найти значок по заданному имени
		 */
		QImage findImageByName (const QString& _name);

		CIconDatabase ();

	public:
		static CIconDatabase& instance ();

		/**
		 * @brief Возвращает значок для указанного MIME-типа
		 */
		QImage mimeImage (const MimeTypeInfo& _mime_type);

		/**
		 * @brief Возвращает иконку для указанного MIME-типа
		 */
		QIcon mimeIcon (const MimeTypeInfo& _mime_type);
	};

	/**
	 * @brief Преобразование изображения в оттенки серого
	 */
	OAFSTD_EXPORT void convertToGrayscale (QImage& _img);
}

#endif /* __ICONHELPERS_H */
