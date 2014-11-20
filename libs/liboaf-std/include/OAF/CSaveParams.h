/**
 * @file
 * @brief Интерфейс класса для отслеживания прогресса OAF::IOperation
 * @author Michael Kurbatov
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CSAVEPARAMS_H
#define __CSAVEPARAMS_H

#include <OAF/OafStdGlobal.h>

#include <QString>
#include <QList>
#include <QPair>
#include <QMap>

namespace OAF
{
	/**
	 * FIXME: Необходимо убедиться, что данный функционал невозможно
	 *        реализовать более "стандартным" способом, ну и привести
	 *        к более аккуратному виду
	 */
	struct OAFSTD_EXPORT CSaveParams
	{
		enum SaveParm
		{
			SAVE_PKM,
			SAVE_DIR,
			SAVE_ARC
		};

		CSaveParams ();
		SaveParm save_parm;
		QMap<QString, QString> saved_files;
		QString dir;
		QString file_name;
		int ndirs;
		void make_dir (const QString& path);
		QString add_file (const QString & fname);
		QString save_file (const QString & fname);
		void copyFile (const QString& _from, const QString& _to);
		bool external_flag;
	};
}

#endif /* __CSAVEPARAMS_H */
