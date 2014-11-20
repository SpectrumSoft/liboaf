/**
 * @file
 * @brief Реализация главного окна приложения
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QCloseEvent>
#include <QFileDialog>

#include <OAF/Utils.h>
#include <OAF/Helpers.h>

#include "CMainWindow.h"

QMessageBox::StandardButton
CMainWindow::check4save ()
{
	if (m_document)
	{
		return QMessageBox::question (this, tr ("Warning" ),
						tr ("Unsaved data will be lost..."),
						QMessageBox::Discard|QMessageBox::Cancel|QMessageBox::Save,
						QMessageBox::Save);
	}

	return QMessageBox::Discard;
}

void
CMainWindow::setDocument (const QString& _path, OAF::URef<OAF::IUIComponent> _document)
{
    Q_UNUSED(_path);

	//
	// Отключаем существующий документ
	//
	if (m_document)
	{
		m_document->activate (false);
		m_document->setUIContainer (NULL);
	}

	//
	// Устанавливаем новый документ
	//
	m_document = _document;

	//
	// Подключаем новый документ
	//
	if (m_document)
	{
		m_document->setUIContainer (this);
		m_document->activate (true);
	}
}

void
CMainWindow::newDocument (const QStringList& _mime_types)
{
	QMessageBox::StandardButton b = check4save ();
	if (b == QMessageBox::Cancel)
	{
		return;
	}
	else if (b == QMessageBox::Save)
	{
		aboutSave ();
	}

	//
	// Пытаемся создать пустой документ заданного типа
	//
	if (OAF::URef<OAF::IUIComponent> document = OAF::createFromMIME<OAF::IUIComponent> (_mime_types))
		setDocument (QString (), document);
	else
		QMessageBox::warning (this, "Not supported format", "Can't create file with type " + _mime_types[0]);
}

void
CMainWindow::aboutNewWindow ()
{
	CMainWindow* w = new CMainWindow ();
	w->show ();
}

void
CMainWindow::aboutNewText ()
{
	QStringList mime_types;
	mime_types.append ("text/plain");
	//
	// Создаём пустой текстовый документ
	//
	newDocument (mime_types);
}

void
CMainWindow::aboutNewHTML ()
{
	QStringList mime_types;
	mime_types.append ("text/html");
	mime_types.append ("text/plain");

	//
	// Создаём пустой HTML документ. Если не удалось, то создаём пустой текстовый документ
	//
	newDocument (mime_types);
}

void
CMainWindow::aboutNewLyX ()
{
	QStringList mime_types;
	mime_types.append ("application/x-lyx");
	mime_types.append ("text/html");
	mime_types.append ("text/plain");

	//
	// Создаём пустой LyX документ. Если не удалось, то создаём пустой HTML документ, а если
	// и это не удалось, то пустой текстовый документ
	//
	newDocument (mime_types);
}

void
CMainWindow::aboutOpen ()
{
	QMessageBox::StandardButton b = check4save ();
	if (b == QMessageBox::Cancel)
	{
		return;
	}
	else if (b == QMessageBox::Save)
	{
		aboutSave ();
	}

	QFileDialog open (this, tr ("Open file"));
	open.setAcceptMode (QFileDialog::AcceptOpen);
	open.setFileMode (QFileDialog::ExistingFile);

	if (open.exec ())
	{
		QStringList files = open.selectedFiles ();
		if (files.count () == 1)
		{
			//
			// Пытаемся загрузить указанный файл
			//
			if (OAF::URef<OAF::IUIComponent> document = OAF::createFromName<OAF::IUIComponent> (OAF::fromLocalFile (files[0]).toString ()))
				setDocument (OAF::fromLocalFile (files[0]).toString (), document);
			else
				QMessageBox::warning (this, "Not supported format", "Can't open given file");
		}
	}
}

void
CMainWindow::aboutSave ()
{
	if (!m_document.queryInterface<OAF::IPropertyBag> ()->isDefined ("path"))
		aboutSaveAs ();
	else
	{
		OAF::URef<OAF::IIODevice> d = OAF::createFromName<OAF::IIODevice> (m_document.queryInterface<OAF::IPropertyBag> ()->getValue ("path").toString ());
		if (d->device ()->open (QIODevice::WriteOnly))
		{
			//
			// Если документ поддерживает текстовый формат, то пишем в него
			//
			if (OAF::ITXTPersist* txt = m_document.queryInterface<OAF::ITXTPersist> ())
			{
				QTextStream os (d->device ());
				txt->save (os, d->getInfo (OAF::IIODevice::CONTENT_TYPE).toStringList ());
			}
		}
	}
}

void
CMainWindow::aboutSaveAs ()
{
	QFileDialog open (this, tr ("Save as file"));
	open.setAcceptMode (QFileDialog::AcceptSave);
	open.setFileMode (QFileDialog::AnyFile);

	if (open.exec ())
	{
		QStringList files = open.selectedFiles ();
		if (files.count () == 1)
		{
			m_document.queryInterface<OAF::IPropertyBag> ()->setValue ("path", OAF::fromLocalFile (files[0]).toString ());
			aboutSave ();
		}
	}
}

void
CMainWindow::aboutAboutDialog ()
{
	QMessageBox::about (this, tr ("About"), tr ("QTextEdit 0.5.0"));
}

void
CMainWindow::closeEvent (QCloseEvent* _e)
{
	QMessageBox::StandardButton b = check4save ();
	if (b == QMessageBox::Cancel)
	{
		_e->ignore ();
		return;
	}
	else if (b == QMessageBox::Save)
	{
		aboutSave ();
	}

	QMainWindow::closeEvent (_e);
}

CMainWindow::CMainWindow ()
{
	//
	// Устанавливаем удаление всех объектов окна при его закрытии
	//
	setAttribute (Qt::WA_DeleteOnClose);

	//
	// Устанавливаем размер по умолчанию
	//
	resize (1024, 768);

	//
	// Главное меню
	//
	m_menubar = menuBar ();
	m_menubar->setObjectName ("ui:menubar");
	m_manager.install ("ui:menubar", m_menubar);

	//
	// Тулбар (в качестве места для размещения тулбаров выступает само главное окно, так как
	// тулбары представляют собой по сути те же плавающие окна)
	//
	m_manager.install ("ui:toolbar", this);

	//
	// Центральный виджет (стек виджетов)
	//
	m_main = new QStackedWidget (this);
	m_main->setObjectName ("ui:main");
	setCentralWidget (m_main);
	m_manager.install ("ui:main", m_main);

	//
	// Строка статуса
	//
	m_statusbar = statusBar ();
	m_statusbar->setObjectName ("ui:statusbar");
	m_manager.install ("ui:statusbar", m_statusbar);

	//
	// Элементы интерфейса главного окна
	//
	// FIXME: иконка по умолчанию
	m_new_window = new QAction (tr ("&New window"), this);
	m_new_window->setObjectName ("ui:shell:new_window");
	m_new_window->setIcon (QIcon::fromTheme ("window-new"));
	connect (m_new_window, SIGNAL (triggered(bool)), this, SLOT (aboutNewWindow ()));

	// FIXME: иконка по умолчанию
	m_new_text = new QAction (tr ("&New text"), this);
	m_new_text->setObjectName ("ui:shell:new_text");
	m_new_text->setIcon (QIcon::fromTheme ("text-x-generic"));
	connect (m_new_text, SIGNAL (triggered(bool)), this, SLOT (aboutNewText ()));

	// FIXME: иконка по умолчанию
	m_new_html = new QAction (tr ("&New HTML"), this);
	m_new_html->setObjectName ("ui:shell:new_html");
	m_new_html->setIcon (QIcon::fromTheme ("text-html"));
	connect (m_new_html, SIGNAL (triggered(bool)), this, SLOT (aboutNewHTML ()));

	// FIXME: иконка по умолчанию
	m_new_lyx = new QAction (tr ("&New LyX"), this);
	m_new_lyx->setObjectName ("ui:shell:new_lyx");
	m_new_lyx->setIcon (QIcon::fromTheme ("text-lyx"));
	connect (m_new_lyx, SIGNAL (triggered(bool)), this, SLOT (aboutNewLyX ()));

	// FIXME: иконка по умолчанию
	m_open = new QAction (tr ("&Open ..."), this);
	m_open->setObjectName ("ui:shell:open");
	m_open->setIcon (QIcon::fromTheme ("document-open"));
	connect (m_open, SIGNAL (triggered(bool)), this, SLOT (aboutOpen ()));

	// FIXME: иконка по умолчанию
	m_save = new QAction (tr ("&Save"), this);
	m_save->setObjectName ("ui:shell:save");
	m_save->setIcon (QIcon::fromTheme ("document-save"));
	connect (m_save, SIGNAL (triggered(bool)), this, SLOT (aboutSave ()));

	// FIXME: иконка по умолчанию
	m_save_as = new QAction (tr ("Save &As ..."), this);
	m_save_as->setObjectName ("ui:shell:save_as");
	m_save_as->setIcon (QIcon::fromTheme ("document-save-as"));
	connect (m_save_as, SIGNAL (triggered(bool)), this, SLOT (aboutSaveAs ()));

	// FIXME: иконка по умолчанию
	m_close = new QAction (tr ("&Close"), this);
	m_close->setObjectName ("ui:shell:close");
	m_close->setIcon (QIcon::fromTheme ("window-close"));
	connect (m_close, SIGNAL (triggered(bool)), this, SLOT (close ()));

	// FIXME: иконка по умолчанию
	m_quit = new QAction (tr ("&Exit"), this);
	m_quit->setObjectName ("ui:shell:quit");
	m_quit->setIcon (QIcon::fromTheme ("application-exit"));
	connect (m_quit, SIGNAL (triggered(bool)), qApp, SLOT (closeAllWindows ()));

	m_about = new QAction (tr ("&About ..."), this);
	m_about->setObjectName ("ui:shell:about");
	connect (m_about, SIGNAL (triggered(bool)), this, SLOT (aboutAboutDialog ()));

	//
	// Регистрируем собственный интерфейс пользователя
	//
	setUIContainer (this);

	//
	// Создаём новый пустой текстовый документ
	//
	aboutNewLyX ();
}

CMainWindow::~CMainWindow ()
{
	//
	// Отключаем редактируемый документ
	//
	setDocument (QString (), OAF::URef<OAF::IUIComponent> ());

	//
	// Отключаем собственный интерфейс
	//
	setUIContainer (NULL);
}

QUuid
CMainWindow::addUI (const QString& _uidef, OAF::IUIComponent* _uic)
{
	return m_manager.addUI (_uidef, _uic);
}

void
CMainWindow::removeUI (const QUuid& _ui)
{
	m_manager.removeUI (_ui);
}

void
CMainWindow::setUILabel (const QString& _bag, const QString& _label)
{
	Q_UNUSED (_bag);
	Q_UNUSED (_label);

	//
	// Механизм переопределения меток главным окном не поддерживается
	//
}

void
CMainWindow::setUIContainer (OAF::IUIContainer* _uic)
{
	//
	// Стандартный набор папок и действий
	//
	static const char uidef[] =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:menubar\">"
				//
				// В самой левой позиции главного меню помещаем подменю "Файл"
				//
				"<folder id=\"ui:file\" label=\"&amp;File\" priority=\"-1000\">"
					//
					// Пункты меню "Новое окно"/"Новый текст"/"Новый HTML" помещаем на самый верх
					//
					"<item id=\"ui:shell:new_window\" priority=\"-1000\"/>"
					"<item id=\"ui:shell:new_text\" priority=\"-1000\"/>"
					"<item id=\"ui:shell:new_html\" priority=\"-1000\"/>"
					"<item id=\"ui:shell:new_lyx\" priority=\"-1000\"/>"
					//
					// Пункты меню "Открыть"/"Сохранить"/"Сохранить как" помещаем после разделителя
					//
					"<separator priority=\"-1000\"/>"
					"<item id=\"ui:shell:open\" priority=\"-1000\"/>"
					"<item id=\"ui:shell:save\" priority=\"-1000\"/>"
					"<item id=\"ui:shell:save_as\" priority=\"-1000\"/>"

					//
					// Пункты меню "Закрыть окно" и "Выход" помещаем в самый низ и отделяем
					// от остальных пунктов меню разделителем
					//
					"<separator priority=\"1000\"/>"
					"<item id=\"ui:shell:close\" priority=\"1000\"/>"
					"<item id=\"ui:shell:quit\" priority=\"1000\"/>"
				"</folder>"

				//
				// Следом за подменю "Файл" помещаем подменю "Правка"
				//
				"<folder id=\"ui:edit\" label=\"&amp;Edit\" priority=\"-999\"/>"

				//
				// Подменю "Помощь" выносим в самый конец главного меню
				//
				"<folder id=\"ui:help\" label=\"&amp;Help\" priority=\"1000\">"
					//
					// Пункты меню "О программе" помещаем в самый низ и отделяем от остальных
					// пунктов меню разделителем
					//
					"<separator priority=\"1000\"/>"
					"<item id=\"ui:shell:about\" priority=\"1000\"/>"
				"</folder>"
			"</bag>"

			"<bag id=\"ui:toolbar\">"
				"<folder id=\"ui:file\" label=\"File\">"
					"<item id=\"ui:shell:new_text\"/>"
					"<item id=\"ui:shell:new_html\"/>"
					"<item id=\"ui:shell:open\"/>"
					"<item id=\"ui:shell:save\"/>"
				"</folder>"
			"</bag>"
		"</uidef>";

	if (_uic)
		m_ui = m_manager.addUI (tr (uidef), this);
	else
		m_manager.removeUI (m_ui);
}

OAF::IUIContainer*
CMainWindow::getUIContainer ()
{
	return NULL;
}

void
CMainWindow::activate (bool /*_activate*/)
{}

QObject*
CMainWindow::getUIItem (const QString& _id)
{
	if (_id == m_new_window->objectName ())
	  return m_new_window;

	if (_id == m_new_text->objectName ())
	  return m_new_text;

	if (_id == m_new_html->objectName ())
	  return m_new_html;

	if (_id == m_new_lyx->objectName ())
	  return m_new_lyx;

	if (_id == m_open->objectName ())
	  return m_open;

	if (_id == m_save->objectName ())
	  return m_save;

	if (_id == m_save_as->objectName ())
	  return m_save_as;

	if (_id == m_close->objectName ())
	  return m_close;

	if (_id == m_quit->objectName ())
	  return m_quit;

	if (_id == m_about->objectName ())
	  return m_about;

	return NULL;
}
