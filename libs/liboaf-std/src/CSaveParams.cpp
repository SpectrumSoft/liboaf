/**
 * @file
 * @brief Реализация класса для отслеживания прогресса OAF::IOperation
 * @author Michael Kurbatov
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QFileInfo>
#include <QDir>

#include <OAF/Utils.h>
#include <OAF/CSaveParams.h>

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif

using namespace OAF;

CSaveParams::CSaveParams () : save_parm (SAVE_PKM),  ndirs(0), external_flag(false)
{}

void
CSaveParams::make_dir (const QString &path)
{
#ifdef WIN32
	QString qpath = path.right(path.length() -8);
#else
	QString qpath = path.right(path.length() -7);
#endif
	QFileInfo f (qpath);
	QDir qdir = f.absoluteDir();
	QString dir = qdir.absolutePath();
	QString fn = f.fileName();
	qdir.mkdir(qpath);
	this->dir = qpath;
	file_name = qpath + '/' + fn;
}

QString
CSaveParams::add_file (const QString &filename)
{
	QString fname = filename.right(filename.length() -8);
	QMap<QString, QString>::iterator it = saved_files.find(fname);
	if (it!= saved_files.end())
		return saved_files[fname];
	QFileInfo f(fname);
	QString fn = f.fileName();
	int ndir = -1;
	for (int i=1; i<= ndirs; i++)
	{
		QString subfn = QString("%1/%2/%3").arg(dir).arg(i).arg(fn);
		QFile f1(subfn);
		if (!f1.exists())
		{
			ndir = i;
			break;
		}
	}
	QString subdir;
	if (ndir == -1)
	{
		ndirs++;
		ndir = ndirs;
		subdir = QString("%1/%2").arg(dir).arg(ndir);
		QDir d(subdir);
		d.mkdir(subdir);
	}
	else
		subdir = QString("%1/%2").arg(dir).arg(ndir);
	QString newfn = subdir + '/' + fn;
//	copyFile(fname, newfn);
//	newfn = OAF::makeRelativePath (dir, newfn);
	saved_files[fname] = newfn;
	return "file:///" + newfn;
}

QString
CSaveParams::save_file (const QString &fname)
{
	QString newfn;
	QMap<QString, QString>::iterator it = saved_files.find(fname);
	if (it!= saved_files.end())
	{
		newfn = saved_files[fname];
		if (external_flag)
			newfn = "../" +newfn;
		return newfn;
	}
	QFileInfo f(fname);
	QString fn = f.fileName();
	int ndir = -1;
	for (int i=1; i<= ndirs; i++)
	{
		QString subfn = QString("%1/%2/%3").arg(dir).arg(i).arg(fn);
		QFile f1(subfn);
		if (!f1.exists())
		{
			ndir = i;
			break;
		}
	}
	QString subdir;
	if (ndir == -1)
	{
		ndirs++;
		ndir = ndirs;
		subdir = QString("%1/%2").arg(dir).arg(ndir);
		QDir d(subdir);
		d.mkdir(subdir);
	}
	else
		subdir = QString("%1/%2").arg(dir).arg(ndir);
	newfn = subdir + '/' + fn;
	copyFile(fname, newfn);
	newfn = QString("%1/%2").arg(ndir).arg(fn);
	saved_files[fname] = newfn;
	if (external_flag)
		newfn = "../" +newfn;
	return  newfn;
}

void
CSaveParams::copyFile (const QString &_from, const QString &_to)
{
	QFile from(_from);
	if (!from.open(QFile::ReadOnly))
		return;
	QFile to(_to);
	if (!to.open(QFile::WriteOnly | QFile::Truncate))
		return;
	to.write(from.readAll());
	from.close();
	to.close();
}
