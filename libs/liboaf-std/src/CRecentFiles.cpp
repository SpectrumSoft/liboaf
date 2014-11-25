/**
 * @file
 * @brief Реализация списка последних открытых файлов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QSettings>
#include <QStringList>
#include <QFileInfo>
#include <QTimer>

#include <OAF/StreamUtils.h>
#include <OAF/Helpers.h>
#include <OAF/CRecentFiles.h>

using namespace OAF;

QScopedPointer<CRecentFiles> OAF::CRecentFiles::m_recent_helper;

bool
CRecentFiles::check () const
{
	QStack<QPair<QString, QString> > new_stack;

	//
	// Проходим по всему списку файлов и выкидываем те из них,
	// которые не существуют или недоступны для чтения
	//
	QPair<QString, QString> s;
	foreach (s, m_stack)
	{
		QUrl u (s.first);
		if (u.scheme () == "file")
		{
			QFileInfo f (OAF::toLocalFile (u));
			if (f.isReadable ())
				new_stack.push_back (s);
		}
	}

	if (new_stack != m_stack)
	{
		m_stack = new_stack;
		return true;
	}

	return false;
}

/**
 * @name Разделители между названием файла и путём к нему и между элементами
 *
 * Уникальные строки (~99.(9)%), которые точно не будут использоваться в путях
 * и названиях
 */
/** @{*/
static const QString NAME_AND_PATH_SEPARATOR = "-----------------------------4934773896482885481650897724";
static const QString ITEMS_SEPARATOR         = "-----------------------------108248967318451809571498147106";
/** @} */

static QString
stackToString (const QStack<QPair<QString, QString> >& _stack)
{
	QString result;

	QPair<QString, QString> f;
	foreach (f, _stack)
	{
		if (!result.isEmpty())
			result += ITEMS_SEPARATOR;

		result += QString("%1%3%2").arg(f.second, f.first, NAME_AND_PATH_SEPARATOR);
	}

	return result;
}

static QStack<QPair<QString, QString> >
stringToStack (const QString& _string)
{
	QStack< QPair<QString, QString> > result;

	foreach (const QString& item, _string.split (ITEMS_SEPARATOR, QString::SkipEmptyParts))
	{
		QStringList file_data = item.split (NAME_AND_PATH_SEPARATOR);
		result.push (QPair<QString, QString> (file_data.last (), file_data.first ()));
	}

	return result;
}

void
CRecentFiles::save () const
{
	QSettings settings (m_org, m_app);
	settings.setValue (m_key, ::stackToString (m_stack));

	QTimer::singleShot (0, const_cast<CRecentFiles*> (this), SLOT (aboutStateChanged ()));
}

CRecentFiles::CRecentFiles (const QString& _org, const QString& _app, const QString& _key, int _max) :
	QObject (NULL), m_org (_org), m_app (_app), m_key (_key), m_max (_max)
{
	QSettings settings (m_org, m_app);

	//
	// Читаем список файлов из конфигурации
	//
	QStack<QPair<QString, QString> > files = ::stringToStack (settings.value (m_key).toString ());

	int cnt = 0;
	QPair<QString, QString> s;
	foreach (s, files)
	{
		QUrl u (s.first);
		if (u.scheme () == "file")
		{
			QFileInfo f (OAF::toLocalFile (u));
			if (f.isReadable ())
			{
				m_stack.push_back (s);

				if (++cnt == m_max)
					break;
			}
		}
	}
}

void
CRecentFiles::aboutStateChanged ()
{
	emit stateChanged ();
}

void
CRecentFiles::push (const QString& _path, const QString& _name)
{
	bool need_update = check ();

	//
	// Работаем только с локальными и существующими файлами
	//
	QUrl u (_path);
	if (u.scheme () == "file")
	{
		QFileInfo f (OAF::toLocalFile (u));
		if (f.isReadable ())
		{
			//
			// Если имя не задано используем имя файла (с расширением)
			//
			QString name = _name;
			if (name.isEmpty ())
				name = f.fileName ();

			//
			// Сформируем элемент
			//
			QPair<QString, QString> recent_file (_path, name);

			//
			// Индекс данного файла в стеке (при сравнении элементов используем только _path,
			// так как имена могут произвольно меняться)
			//
			int idx = -1; int ii = 0;
			for (QStack<QPair<QString, QString> >::const_iterator i = m_stack.begin (); i != m_stack.end (); ++i)
			{
				if (i->first == recent_file.first)
				{
					idx = ii;
					break;
				}

				++ii;
			}

			//
			// Если файл находится наверху стека
			//
			if (idx == 0)
			{
				//
				// Если наименования отличаются
				//
				if (m_stack[idx].second != recent_file.second)
				{
					//
					// Обновляем наименование файла
					//
					m_stack[idx].second = recent_file.second;

					//
					// Выставляем признак необходимости обновления конфигурации
					//
					need_update = true;
				}
			}
			//
			// Если файла нет на стеке, либо он не наверху
			//
			else
			{
				//
				// Если в стеке уже находится данный файл, то удаляем его. Он будет перенесён
				// наверх стека
				//
				if (idx > 0)
					m_stack.remove (idx);

				//
				// Заносим файл наверх стека
				//
				m_stack.push_front (recent_file);

				//
				// Если размеры стека превысили заданную величину, то выбрасываем "старые" файлы
				//
				if (m_stack.size () > m_max)
					m_stack.resize (m_max);

				//
				// Выставляем признак необходимости обновления конфигурации
				//
				need_update = true;
			}
		}
	}

	//
	// Если необходимо, то сохраняем конфигурацию
	//
	if (need_update)
		save ();
}

void
CRecentFiles::clear ()
{
	if (m_stack.size () > 0)
	{
		m_stack.clear ();
		save ();
	}
}

const QStack<QPair<QString, QString> >&
CRecentFiles::stack () const
{
	if (check ())
		save ();

	return m_stack;
}

OAF::CRecentFiles*
CRecentFiles::instance (const QString& _org, const QString& _app, const QString& _key, int _max)
{
	if (!m_recent_helper)
		m_recent_helper.reset (new CRecentFiles (_org, _app, _key, _max));
	return m_recent_helper.data ();
}
