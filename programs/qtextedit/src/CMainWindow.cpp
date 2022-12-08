/**
 * @file
 * @brief Реализация главного окна приложения
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CNotifySource.h>
#include <OAF/StreamUtils.h>
#include <OAF/Helpers.h>

#include "CMainWindow.h"

QMessageBox::StandardButton
CMainWindow::check4save ()
{
	if (m_document && m_is_modified)
	{
		return QMessageBox::question (this, tr ("Warning" ),
						tr ("Unsaved data will be lost..."),
						QMessageBox::Discard|QMessageBox::Cancel|QMessageBox::Save,
						QMessageBox::Save);
	}

	return QMessageBox::Discard;
}

void
CMainWindow::setDocument (OAF::ITextDocument* _d)
{
	//
	// Отключаемся от текущего документа
	//
	if (m_document)
	{
		OAF::unsubscribe (m_document, this);

		//
		// Отключаем редактор документа
		//
		if (m_edit)
		{
			m_edit->activate (false);
			m_edit->setUIContainer (NULL);
		}

		m_edit = NULL;
	}

	//
	// Устанавливаем новый документ
	//
	m_document = _d;

	//
	// Подключаемся к новому документу
	//
	if (m_document)
	{
		OAF::subscribe (m_document, this);

		//
		// Создаём редактор для документа
		//
		m_edit = OAF::createFromObject<OAF::IUIComponent> (m_document.queryInterface<OAF::IUnknown> ());

		//
		// Подключаем новый редактор документа
		//
		if (m_edit)
		{
			m_edit->setUIContainer (this);
			m_edit->activate (true);
		}
	}

	m_is_modified = false;
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
	if (OAF::URef<OAF::ITextDocument> d = OAF::createFromMIME<OAF::ITextDocument> (_mime_types))
		setDocument (d);
	else
		QMessageBox::warning (this, "Not supported format", "Can't create file with type " + _mime_types[0]);
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

void
CMainWindow::notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_source);
	Q_UNUSED (_origin);

	//
	// Если это событие от текстового документа, значит он изменился
	//
	if (OAF::queryInterface<OAF::ITextDocumentNotify> (_event))
		m_is_modified = true;
}

void
CMainWindow::aboutNewWindow ()
{
	CMainWindow* w = new CMainWindow ();
	w->show ();
}

void
CMainWindow::aboutNewPlain ()
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
	//
	// Создаём пустой HTML документ. Если не удалось, то создаём пустой текстовый документ
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

	if (open.exec () == QFileDialog::Accepted)
	{
		QStringList files = open.selectedFiles ();
		if (files.count () == 1)
		{
			//
			// Пытаемся загрузить указанный файл
			//
			if (OAF::URef<OAF::ITextDocument> d = OAF::createFromName<OAF::ITextDocument> (OAF::fromLocalFile (files[0]).toString ()))
				setDocument (d);
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

	m_is_modified = false;
}

void
CMainWindow::aboutSaveAs ()
{
	QFileDialog open (this, tr ("Save as file"));
	open.setAcceptMode (QFileDialog::AcceptSave);
	open.setFileMode (QFileDialog::AnyFile);

	if (open.exec () == QFileDialog::Accepted)
	{
		QStringList files = open.selectedFiles ();
		if (files.count () == 1)
		{
			m_edit.queryInterface<OAF::IPropertyBag> ()->setValue ("path", OAF::fromLocalFile (files[0]).toString ());
			aboutSave ();
		}
	}
}

void
CMainWindow::aboutQuit ()
{
	QTimer::singleShot (0, qApp, SLOT (closeAllWindows ()));
}

void
CMainWindow::aboutAboutDialog ()
{
	QMessageBox::about (this, tr ("About"), tr ("QTextEdit 0.5.0"));
}

CMainWindow::CMainWindow () : m_is_modified (false)
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
	m_new_window->setObjectName ("ui:new_window");
	m_new_window->setIcon (QIcon::fromTheme ("window-new"));
	connect (m_new_window, SIGNAL (triggered(bool)), this, SLOT (aboutNewWindow ()));

	// FIXME: иконка по умолчанию
	m_new_plain = new QAction (tr ("&New text"), this);
	m_new_plain->setObjectName ("ui:new_plain");
	m_new_plain->setIcon (QIcon::fromTheme ("text-x-generic"));
	connect (m_new_plain, SIGNAL (triggered(bool)), this, SLOT (aboutNewPlain ()));

	// FIXME: иконка по умолчанию
	m_new_html = new QAction (tr ("&New HTML"), this);
	m_new_html->setObjectName ("ui:new_html");
	m_new_html->setIcon (QIcon::fromTheme ("text-html"));
	connect (m_new_html, SIGNAL (triggered(bool)), this, SLOT (aboutNewHTML ()));

	// FIXME: иконка по умолчанию
	m_open = new QAction (tr ("&Open ..."), this);
	m_open->setObjectName ("ui:open");
	m_open->setIcon (QIcon::fromTheme ("document-open"));
	connect (m_open, SIGNAL (triggered(bool)), this, SLOT (aboutOpen ()));

	// FIXME: иконка по умолчанию
	m_save = new QAction (tr ("&Save"), this);
	m_save->setObjectName ("ui:save");
	m_save->setIcon (QIcon::fromTheme ("document-save"));
	connect (m_save, SIGNAL (triggered(bool)), this, SLOT (aboutSave ()));

	// FIXME: иконка по умолчанию
	m_save_as = new QAction (tr ("Save &As ..."), this);
	m_save_as->setObjectName ("ui:save_as");
	m_save_as->setIcon (QIcon::fromTheme ("document-save-as"));
	connect (m_save_as, SIGNAL (triggered(bool)), this, SLOT (aboutSaveAs ()));

	// FIXME: иконка по умолчанию
	m_close = new QAction (tr ("&Close"), this);
	m_close->setObjectName ("ui:close");
	m_close->setIcon (QIcon::fromTheme ("window-close"));
	connect (m_close, SIGNAL (triggered(bool)), this, SLOT (close ()));

	// FIXME: иконка по умолчанию
	m_quit = new QAction (tr ("&Exit"), this);
	m_quit->setObjectName ("ui:quit");
	m_quit->setIcon (QIcon::fromTheme ("application-exit"));
	connect (m_quit, SIGNAL (triggered(bool)), this, SLOT (aboutQuit ()));

	m_about = new QAction (tr ("&About ..."), this);
	m_about->setObjectName ("ui:about");
	connect (m_about, SIGNAL (triggered(bool)), this, SLOT (aboutAboutDialog ()));

	//
	// Регистрируем собственный интерфейс пользователя
	//
	setUIContainer (this);

	//
	// Создаём новый пустой текстовый документ
	//
	aboutNewPlain ();
}

CMainWindow::~CMainWindow ()
{
	//
	// Отключаем редактируемый документ
	//
	setDocument (NULL);

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

QUuid
CMainWindow::removeUI (const QUuid& _ui)
{
	return m_manager.removeUI (_ui);
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
				"<folder id=\"ui:file\" label=\"%1\" priority=\"-1000\">"
					//
					// Пункты меню "Новое окно"/"Новый текст"/"Новый HTML" помещаем на самый верх
					//
					"<item id=\"ui:new_window\" priority=\"-1000\"/>"

					"<separator priority=\"-1000\"/>"
					"<item id=\"ui:new_plain\" priority=\"-1000\"/>"
					"<item id=\"ui:new_html\" priority=\"-1000\"/>"

					//
					// Пункты меню "Открыть"/"Сохранить"/"Сохранить как" помещаем после разделителя
					//
					"<separator priority=\"-1000\"/>"
					"<item id=\"ui:open\" priority=\"-1000\"/>"
					"<item id=\"ui:save\" priority=\"-1000\"/>"
					"<item id=\"ui:save_as\" priority=\"-1000\"/>"

					//
					// Пункты меню "Закрыть окно" и "Выход" помещаем в самый низ и отделяем
					// от остальных пунктов меню разделителем
					//
					"<separator priority=\"1000\"/>"
					"<item id=\"ui:close\" priority=\"1000\"/>"
					"<item id=\"ui:quit\" priority=\"1000\"/>"
				"</folder>"

				//
				// Подменю "Помощь" выносим в самый конец главного меню
				//
				"<folder id=\"ui:help\" label=\"%2\" priority=\"1000\">"
					//
					// Пункты меню "О программе" помещаем в самый низ и отделяем от остальных
					// пунктов меню разделителем
					//
					"<separator priority=\"1000\"/>"
					"<item id=\"ui:about\" priority=\"1000\"/>"
				"</folder>"
			"</bag>"

			"<bag id=\"ui:toolbar\">"
				"<folder id=\"ui:file\" label=\"%3\">"
					"<item id=\"ui:new_plain\"/>"
					"<item id=\"ui:new_html\"/>"
					"<item id=\"ui:open\"/>"
					"<item id=\"ui:save\"/>"
				"</folder>"
			"</bag>"
		"</uidef>";

	if (_uic)
		m_ui = m_manager.addUI (QString (uidef)
									.arg (tr ("&amp;File"))
									.arg (tr ("&amp;Help"))
									.arg (tr ("File")), this);
	else
		m_ui = m_manager.removeUI (m_ui);
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

	if (_id == m_new_plain->objectName ())
	  return m_new_plain;

	if (_id == m_new_html->objectName ())
	  return m_new_html;

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
