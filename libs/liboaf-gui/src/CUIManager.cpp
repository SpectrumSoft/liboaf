/**
 * @file
 * @brief Реализация управления динамическим интерфейсом пользователя
 * @author Sergey N. Yatskevich <syatskevih@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CUIManager.h>

/**
 * @brief Определение приоритета
 */
static int
getPriority (const QXmlStreamAttributes& _attrs)
{
	int p = 0;

	if (_attrs.hasAttribute ("oaf:priority"))
		p = _attrs.value ("oaf:priority").toString ().toInt ();

	return p;
}

/**
 * @brief Найти позицию для вставки действия по заданному приоритету
 */
static QAction*
positionByPriority (QWidget* _to, int _p)
{
	QList<QAction*> actions = _to->actions ();
	for (QList<QAction*>::iterator a = actions.begin (); a != actions.end (); ++a)
	{
		if (_p < (*a)->property (_to->objectName ().toUtf8 ()).toInt ())
			return *a;
	}

	return NULL;
}

/**
 * @brief Найти позицию для вставки тулбара по заданному приоритету
 */
static QToolBar*
positionByPriority (QMainWindow* _to, int _p)
{
	QList<QToolBar*> toolbars = _to->findChildren<QToolBar*> ();
	for (QList<QToolBar*>::iterator t = toolbars.begin (); t != toolbars.end (); ++t)
	{
		if (_p < (*t)->property (_to->objectName ().toUtf8 ()).toInt ())
			return *t;
	}

	return NULL;
}

/**
 * @brief Поместить действие в соответствии с заданной позицией
 *
 * @param[in] _a дейстаие для добавления
 */
static void
insertAction (QWidget* _to, QAction* _a, int _p)
{
	_a->setProperty (_to->objectName ().toUtf8 (), _p);

	_to->insertAction (positionByPriority (_to, _p), _a);
}

/**
 * @brief Добавить тулбар в соответствии с заданным приоритетом
 *
 * При равных приоритетах тулбары располагаются в порядке добавления
 */
static void
insertToolBar (QMainWindow* _to, QToolBar* _t, int _p)
{
	_t->setProperty (_to->objectName ().toUtf8 (), _p);

	if (QToolBar* before = positionByPriority (_to, _p))
		_to->insertToolBar (before, _t);
	else
		_to->addToolBar (_t);

	if (_t->property ("break").toBool ())
		_to->insertToolBarBreak (_t);
}

static void
removeAction (QWidget* _from, QAction* _a)
{
	_from->removeAction (_a);

	_a->setProperty (_from->objectName ().toUtf8 (), QVariant ());
}

OAF::CUIManager::UI::UI () : m_uic (NULL)
{}

OAF::CUIManager::UI::UI (OAF::IUIComponent* _uic) : m_uic (_uic)
{}

OAF::CUIManager::UI::UI (const UI& _ui) : m_desc (_ui.m_desc), m_uic (_ui.m_uic), m_items (_ui.m_items)
{}

void
OAF::CUIManager::UI::add (QObject* _object)
{
	if (_object)
		m_items.insert (_object);
}

void
OAF::CUIManager::UI::remove (QObject* _object)
{
	if (_object)
		m_items.remove (_object);
}

void
OAF::CUIManager::UI::set (const QString& _desc)
{
	m_desc = _desc;
}

const QString&
OAF::CUIManager::UI::get () const
{
	return m_desc;
}

OAF::IUIComponent*
OAF::CUIManager::UI::uic () const
{
	return m_uic;
}

bool
OAF::CUIManager::UI::ownerOf (QObject* _object) const
{
	return m_items.contains (_object);
}

OAF::CUIManager::CUIManager ()
{}

OAF::CUIManager::~CUIManager ()
{}

void
OAF::CUIManager::addFolder (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldef.attributes ();

	//
	// Если описание папки не имеет идентификатора или не имеет метки, то мы с ней
	// сделать ничего не можем
	//
	Q_ASSERT (attrs.hasAttribute ("id"));
	Q_ASSERT (attrs.hasAttribute ("label"));

	//
	// Определяем приоритет
	//
	int p = getPriority (attrs);

	//
	// Папка для добавления элементов
	//
	QObject* folder = NULL;

	//
	// Если папка добавляется в меню или подменю
	//
	if (QMenu* menu = qobject_cast<QMenu*> (_to))
	{
		//
		// Ищем подменю с заданным идентификатором
		//
		QMenu* submenu = menu->findChild<QMenu*> (attrs.value ("id").toString ());

		//
		// Если такого подменю нет, то создаём его
		//
		if (!submenu)
		{
			//
			// @todo Обработка тэга icon
			//
			submenu = new QMenu (attrs.value ("label").toString (), menu);
			submenu->setObjectName (attrs.value ("id").toString ());
			submenu->setProperty ("oaf:refs", (int)0);

			//
			// Размещаем меню в соответствии с заданным приоритетом
			//
			insertAction (menu, submenu->menuAction (), p);
		}

		//
		// Данное подменю и есть папка
		//
		folder = submenu;
	}
	//
	// Иначе если папка добавляется в меню главного окна
	//
	else if (QMenuBar* menu = qobject_cast<QMenuBar*> (_to))
	{
		//
		// Ищем подменю с заданным идентификатором
		//
		QMenu* submenu = menu->findChild<QMenu*> (attrs.value ("id").toString ());

		//
		// Если такого подменю нет, то создаём его
		//
		if (!submenu)
		{
			//
			// @todo Обработка тэга icon
			//
			submenu = new QMenu (attrs.value ("label").toString (), menu);
			submenu->setObjectName (attrs.value ("id").toString ());
			submenu->setProperty ("oaf:refs", (int)0);

			//
			// Размещаем меню в соответствии с заданным приоритетом
			//
			insertAction (menu, submenu->menuAction (), p);
		}

		//
		// Данное подменю и есть папка
		//
		folder = submenu;
	}
	//
	// Иначе если папка добавляется в главное окно (панель инструментов)
	//
	else if (QMainWindow* mw = qobject_cast<QMainWindow*> (_to))
	{
		//
		// Ищем тулбар с заданным идентификатором
		//
		QToolBar* toolbar = mw->findChild<QToolBar*> (attrs.value ("id").toString ());

		//
		// Если такого тулбара нет, то создаём его
		//
		if (!toolbar)
		{
			//
			// @todo Обработка тэга icon
			//
			toolbar = new QToolBar (attrs.value ("label").toString (), mw);
			toolbar->setObjectName (attrs.value ("id").toString ());
			toolbar->setProperty ("oaf:refs", (int)0);
			//
			// Тулбар идёт на новой строке
			//
			toolbar->setProperty("break", attrs.value ("break").toString () == "true");

			//
			// Размещаем тулбар в соответствии с заданным приоритетом
			//
			insertToolBar (mw, toolbar, p);
		}

		//
		// Данный тулбар и есть папка
		//
		folder = toolbar;
	}

	//
	// Иначе если папка добавляется в диалог (панель инструментов)
	//
	else if (QDialog* dg = qobject_cast<QDialog*> (_to))
	{
		//
		// Ищем тулбар с заданным идентификатором
		//
		QToolBar* toolbar = dg->findChild<QToolBar*> (attrs.value ("id").toString ());

		//
		// Если такого тулбара нет, то создаём его
		//
		if (!toolbar)
		{
			//
			// @todo Обработка тэга icon
			//
			toolbar = new QToolBar (attrs.value ("label").toString (), dg);
			toolbar->setObjectName (attrs.value ("id").toString ());
			toolbar->setProperty ("oaf:refs", (int)0);
		}

		//
		// Данный тулбар и есть папка
		//
		folder = toolbar;
	}

	//
	// Если папка найдена или добавлена
	//
	if (folder)
	{
		//
		// Запоминаем папку как элемент данной части интерфейса пользователя
		//
		_ui.add (folder);

		//
		// Увеличиваем счётчик ссылок на папку из частей интерфейса пользователя
		//
		folder->setProperty ("oaf:refs", folder->property ("oaf:refs").toInt () + 1);

		//
		// Запоминаем указатель на объект папки в описании данной части интерфейса
		//
		_xmldesc.writeStartElement ("folder");
		_xmldesc.writeAttribute ("p", QString::number ((qulonglong)folder, 16));

		//
		// Обработка элементов папки
		//
		while (_xmldef.readNextStartElement ())
		{
			if (_xmldef.name () == "folder")
				addFolder (_xmldef, _xmldesc, folder, _ui);

			else if (_xmldef.name () == "item")
				addItem (_xmldef, _xmldesc, folder, _ui);

			else if (_xmldef.name () == "separator")
				addSeparator (_xmldef, _xmldesc, folder, _ui);

			//
			// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
			//
			if (!_xmldef.isEndElement ())
				_xmldef.skipCurrentElement ();
		}

		_xmldesc.writeEndElement ();
	}
}

void
OAF::CUIManager::removeFolder (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldesc.attributes ();

	//
	// Если описание папки не имеет указателя, то мы с ней сделать ничего не можем
	//
	if (!attrs.hasAttribute ("p"))
		return;

	//
	// Папка для удаления
	//
	QObject* folder = reinterpret_cast<QObject*> (attrs.value ("p").toString ().toULongLong (NULL, 16));

	//
	// Проверяем, что эта папка добавлялась в составе данной части интерфейса пользователя
	//
	if (_ui.ownerOf (folder))
	{
		//
		// Обработка элементов папки
		//
		while (_xmldesc.readNextStartElement ())
		{
			if (_xmldesc.name () == "folder")
				removeFolder (_xmldesc, folder, _ui);

			else if (_xmldesc.name () == "item")
				removeItem (_xmldesc, folder, _ui);

			else if (_xmldesc.name () == "separator")
				removeSeparator (_xmldesc, folder, _ui);

			//
			// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
			//
			if (!_xmldesc.isEndElement ())
				_xmldesc.skipCurrentElement ();
		}

		//
		// Уменьшаем счётчик ссылок на папку
		//
		folder->setProperty ("oaf:refs", folder->property ("oaf:refs").toInt () - 1);

		//
		// Если это последняя ссылка на папку то удаляем её, надеясь, что соответствующий
		// объект сам вычистит себя откуда надо
		//
		if (folder->property ("oaf:refs").toInt () <= 0)
		{
			//
			// Если это тулбар главного окна и для него задан перенос, то удаляем
			// перенос
			//
			if (QMainWindow* mw = qobject_cast<QMainWindow*> (_from))
			{
				if (folder->property ("break").toBool ())
					mw->removeToolBarBreak (qobject_cast<QToolBar*> (folder));
			}

			delete folder;
		}
	}
}

void
OAF::CUIManager::addItem (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldef.attributes ();

	//
	// Если описание элемента не имеет идентификатора, то мы с ним сделать ничего не можем
	//
	Q_ASSERT (attrs.hasAttribute ("id"));

	//
	// Определяем приоритет
	//
	int p = getPriority (attrs);

	//
	// Запрашиваем у компонента заданный элемент интерфейса
	//
	if (QObject* o = _ui.uic ()->getUIItem (attrs.value ("id").toString ()))
	{
		//
		// Запоминаем элемент интерфейса как часть данной части интерфейса пользователя
		//
		_ui.add (o);

		//
		// Запоминаем указатель на элемент интерфейса в описании части интерфейса
		//
		_xmldesc.writeStartElement ("item");
		_xmldesc.writeAttribute ("p", QString::number ((qulonglong)o, 16));
		_xmldesc.writeEndElement ();

		//
		// В зависимости от того, что это за элемент интерфейса
		//
		if (QAction* a = qobject_cast<QAction*> (o))
		{
			//
			// В зависимости от типа контейнера/папки
			//
			if (QMenu* m = qobject_cast<QMenu*> (_to))
				insertAction (m, a, p);
			else if (QMenuBar* m = qobject_cast<QMenuBar*> (_to))
				insertAction (m, a, p);
			else if (QActionGroup* ag = qobject_cast<QActionGroup*> (_to))
				ag->addAction (a);
			else if (QToolBar* t = qobject_cast<QToolBar*> (_to))
				insertAction (t, a, p);

			//
			// QAction больше никуда добавлен быть не может
			//
		}
		else if (QWidget* w = qobject_cast<QWidget*> (o))
		{
			//
			// В зависимости от типа контейнера/папки
			//
			if (QTabWidget* tw = qobject_cast<QTabWidget*> (_to))
			{
				//
				// Если описание элемента не имеет метки, то мы с ним сделать ничего не можем
				//
				Q_ASSERT (attrs.hasAttribute ("label"));

				//
				// @todo Порядок добавления
				// @todo Обработка тэга icon
				//
				tw->addTab (w, attrs.value ("label").toString ());
			}
			else if (QToolBox* bw = qobject_cast<QToolBox*> (_to))
			{
				//
				// Если описание элемента не имеет метки, то мы с ним сделать ничего не можем
				//
				Q_ASSERT (attrs.hasAttribute ("label"));

				//
				// @todo Порядок добавления
				// @todo Обработка тэга icon
				//
				bw->addItem (w, attrs.value ("label").toString ());
			}
			else if (QToolBar* tb = qobject_cast<QToolBar*> (_to))
			{
				//
				// Создаём проксирующий виджет (он необходим, так как при удалении QAction, к
				// которому он будет привязан, он так же будет удалён и это никак изменить не
				// получается)
				//
				QWidget* wp = new QWidget ();
				//
				// Создаём для него упаковщик, чтобы наш виджет был нормально размещён на
				// проксирующем виджете
				//
				QVBoxLayout* wplayout = new QVBoxLayout (wp);
				//
				// Устанавливаем этот упаковщик для проксирующего виджета
				//
				wp->setLayout (wplayout);

				//
				// Добавляем виджет для отображения к проксирующему виджету
				//
				wplayout->addWidget (w);

				//
				// Создаём действие для проксирующего виджета
				//
				// @todo Порядок добавления
				//
				QAction* a = tb->addWidget (wp);

				//
				// Запоминаем связанное с виджетом действие в самом виджете
				//
				w->setProperty ("action", (qulonglong)a);
			}
			else if (QStackedWidget* sw = qobject_cast<QStackedWidget*> (_to))
			{
				//
				// @todo Порядок добавления
				//
				sw->addWidget (w);
			}
			else if (QStatusBar* sw = qobject_cast<QStatusBar*> (_to))
			{
				//
				// @todo Порядок добавления
				//
				sw->addWidget (w, p);

				//
				// После удаления из строки состояния виджет становится невидимым
				// поэтому после повторного добавления нужно его отобразить
				//
				w->show ();
			}

			//
			// QWidget больше никуда не добавляем
			//
		}

		//
		// Другие типы элементов интерфейса не обрабатываем
		//
	}
}

void
OAF::CUIManager::removeItem (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldesc.attributes ();

	//
	// Если описание элемента не имеет указателя, то мы с ним сделать ничего не можем
	//
	Q_ASSERT (attrs.hasAttribute ("p"));

	//
	// Элемент для удаления
	//
	QObject* o = reinterpret_cast<QObject*> (attrs.value ("p").toString ().toULongLong (NULL, 16));

	//
	// Проверяем, что этот элемент добавлялся в составе данной части интерфейса пользователя
	//
	if (_ui.ownerOf (o))
	{
		//
		// В зависимости от того, что это за элемент интерфейса
		//
		if (QAction* a = qobject_cast<QAction*> (o))
		{
			//
			// В зависимости от типа контейнера/папки
			//
			if (QMenu* m = qobject_cast<QMenu*> (_from))
				removeAction (m, a);
			else if (QMenuBar* m = qobject_cast<QMenuBar*> (_from))
				removeAction (m, a);
			else if (QActionGroup* ag = qobject_cast<QActionGroup*> (_from))
				ag->removeAction (a);
			else if (QToolBar* t = qobject_cast<QToolBar*> (_from))
				removeAction (t, a);

			//
			// QAction больше никуда не добавляется
			//
		}
		else if (QWidget* w = qobject_cast<QWidget*> (o))
		{
			//
			// В зависимости от типа контейнера/папки
			//
			if (QTabWidget* tw = qobject_cast<QTabWidget*> (_from))
				tw->removeTab (tw->indexOf (w));
			else if (QToolBox* bw = qobject_cast<QToolBox*> (_from))
				bw->removeItem (bw->indexOf (w));
			else if (QToolBar* tb = qobject_cast<QToolBar*> (_from))
				tb->removeAction ((QAction*)w->property ("action").toULongLong ());
			else if (QStackedWidget* sw = qobject_cast<QStackedWidget*> (_from))
				sw->removeWidget (w);
			else if (QStatusBar* sw = qobject_cast<QStatusBar*> (_from))
				sw->removeWidget (w);

			//
			// Сбрасываем значение родительского виджета, чтобы контейнеры не
			// уносили за собой дочерние виджеты
			//
			w->setParent (NULL);

			//
			// QWidget больше никуда не добавляли
			//
		}

		//
		// Другие типы элементов интерфейса не добавляли
		//
	}
}

void
OAF::CUIManager::addSeparator (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldef.attributes ();

	//
	// Определяем приоритет
	//
	int p = getPriority (attrs);

	//
	// Разделитель
	//
	QAction* s = NULL;

	//
	// В зависимости от типа контейнера/папки
	//
	if (QMenu* m = qobject_cast<QMenu*> (_to))
	{
		s = new QAction (_to);
		s->setSeparator (true);
		insertAction (m, s, p);
	}
	else if (QMenuBar* m = qobject_cast<QMenuBar*> (_to))
	{
		s = new QAction (_to);
		s->setSeparator (true);
		insertAction (m, s, p);
	}
	else if (QToolBar* t = qobject_cast<QToolBar*> (_to))
	{
		s = new QAction (_to);
		s->setSeparator (true);
		insertAction (t, s, p);
	}

	//
	// Разделитель больше никуда добавлен быть не может
	//

	//
	// Если разделитель был добавлен
	//
	if (s)
	{
		//
		// Запоминаем его как часть данной части интерфейса пользователя
		//
		_ui.add (s);

		//
		// Запоминаем указатель на разделитель в описании части интерфейса
		//
		_xmldesc.writeStartElement ("separator");
		_xmldesc.writeAttribute ("p", QString::number ((qulonglong)s, 16));
		_xmldesc.writeEndElement ();
	}
}

void
OAF::CUIManager::removeSeparator (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui)
{
	QXmlStreamAttributes attrs = _xmldesc.attributes ();

	//
	// Если описание разделителя не имеет указателя, то мы с ним сделать ничего не можем
	//
	Q_ASSERT (attrs.hasAttribute ("p"));

	//
	// Разделитель для удаления
	//
	QObject* o = reinterpret_cast<QObject*> (attrs.value ("p").toString ().toULongLong (NULL, 16));

	//
	// Проверяем, что этот разделитель добавлялся в составе данной части интерфейса пользователя
	//
	if (_ui.ownerOf (o))
	{
		//
		// Разделитель - это всегда QAction
		//
		if (QAction* a = qobject_cast<QAction*> (o))
		{
			//
			// В зависимости от типа контейнера/папки
			//
			if (QMenu* m = qobject_cast<QMenu*> (_from))
				removeAction (m, a);
			else if (QMenuBar* m = qobject_cast<QMenuBar*> (_from))
				removeAction (m, a);
			else if (QToolBar* t = qobject_cast<QToolBar*> (_from))
				removeAction (t, a);

			//
			// Разделитель больше никуда не добавляется
			//
		}
	}
}

void
OAF::CUIManager::copyElement (QXmlStreamReader& _from, QXmlStreamWriter& _to)
{
	QXmlStreamAttributes attrs = _from.attributes ();

	_to.writeStartElement (_from.name ().toString ());
	_to.writeAttributes (attrs);
	while (_from.readNextStartElement ())
		copyElement (_from, _to);
	_to.writeEndElement ();
}

void
OAF::CUIManager::install (const QString& _name, QObject* _c)
{
	m_containers[_name] = _c;
}

bool
OAF::CUIManager::isInstalled (const QString& _name) const
{
	return m_containers.contains (_name);
}

bool
OAF::CUIManager::isInstalled (QObject* _c) const
{
	for (auto i = m_containers.constBegin (); i != m_containers.constEnd (); ++i)
	{
		if (i.value () == _c)
			return true;
	}

	return false;
}

void
OAF::CUIManager::remove (const QString& _name)
{
	//
	// Удалить можно только если данный контейнерный виджет нигде не используется
	//
	if (ownerOf (m_containers.value (_name, NULL)) == NULL)
		m_containers.remove (_name);
}

void
OAF::CUIManager::clear ()
{
	//
	// Зачищаем все части интерфейсов пользователя
	//
	while (!m_uidescs.isEmpty ())
		removeUI (m_uidescs.begin ().key ());

	m_containers.clear ();
}

QUuid
OAF::CUIManager::addUI (const QString& _uidef, OAF::IUIComponent* _uic)
{
	//
	// Создаём место для описания заданной части интерфейса пользователя. Место создаём
	// в любом случае, даже если ничего добавлено не будет. Идентификатор нам понадобится
	// для связи всех частей интерфейса воедино
	//
	QUuid id = QUuid::createUuid ();
	UI& ui = m_uidescs[id];
	ui = UI (_uic);

	//
	// Описание созданной части интерфейса пользователя
	//
	QString uidesc;
	QXmlStreamWriter xmldesc (&uidesc);
	xmldesc.setAutoFormatting (false);
	xmldesc.writeStartDocument ("1.0");
	xmldesc.writeStartElement ("ui");

	//
	// Обработка описания интерфейса и построения заданной его части
	//
	QXmlStreamReader xmldef (_uidef);
	if (xmldef.readNextStartElement ())
	{
		while (xmldef.readNextStartElement ())
		{
			//
			// Обработка тэга "bag"
			//
			if (xmldef.name () == "bag")
			{
				//
				// Контейнерный виджет для вставки элемента интерфейса
				//
				if (QObject* to = m_containers.value (xmldef.attributes ().value ("id").toString (), NULL))
				{
					//
					// Запоминаем указатель на контейнерный виджет в описании части интерфейса
					//
					xmldesc.writeStartElement ("bag");
					xmldesc.writeAttribute ("p", QString::number ((qulonglong)to, 16));

					//
					// Читаем описание части интерфейса пользователя для этого контейнерного виджета
					//
					while (xmldef.readNextStartElement ())
					{
						if (xmldef.name () == "folder")
							addFolder (xmldef, xmldesc, to, ui);

						else if (xmldef.name () == "item")
							addItem (xmldef, xmldesc, to, ui);

						else if (xmldef.name () == "separator")
							addSeparator (xmldef, xmldesc, to, ui);

						//
						// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
						//
						if (!xmldef.isEndElement ())
							xmldef.skipCurrentElement ();
					}

					xmldesc.writeEndElement ();
				}
			}

			//
			// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
			//
			if (!xmldef.isEndElement ())
				xmldef.skipCurrentElement ();
		}
	}

	//
	// Запоминаем описание созданной части интерфейса пользователя
	//
	xmldesc.writeEndElement ();
	xmldesc.writeEndDocument ();
	ui.set (uidesc);

	return id;
}

QUuid
OAF::CUIManager::removeUI (const QUuid& _id)
{
	//
	// Удаляемый интерфейс обязан присутствовать
	//
	if (m_uidescs.contains (_id))
	{
		UI& ui = m_uidescs[_id];

		//
		// Обработка описания части интерфейса
		//
		QXmlStreamReader xmldesc (ui.get ());
		if (xmldesc.readNextStartElement () && (xmldesc.name () == "ui"))
		{
			while (xmldesc.readNextStartElement ())
			{
				//
				// Обработка тэга "bag"
				//
				if (xmldesc.name () == "bag")
				{
					//
					// Контейнерный виджет элементов интерфейса из которого будут удаляться
					// элементы
					//
					QObject* from = reinterpret_cast<QObject*> (xmldesc.attributes ().value ("p").toString ().toULongLong (NULL, 16));

					//
					// Проверяем, что этот контейнерный виджет установлен в менеджер
					//
					if (isInstalled (from))
					{
						//
						// Читаем описание части интерфейса пользователя для этого контейнерного
						// виджета
						//
						while (xmldesc.readNextStartElement ())
						{
							if (xmldesc.name () == "folder")
								removeFolder (xmldesc, from, ui);

							else if (xmldesc.name () == "item")
								removeItem (xmldesc, from, ui);

							else if (xmldesc.name () == "separator")
								removeSeparator (xmldesc, from, ui);

							//
							// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
							//
							if (!xmldesc.isEndElement ())
								xmldesc.skipCurrentElement ();
						}
					}
				}

				//
				// Если ещё не находимся в конце элемента, то пропускаем всё до его конца
				//
				if (!xmldesc.isEndElement ())
					xmldesc.skipCurrentElement ();
			}
		}

		m_uidescs.remove (_id);
	}

	//
	// Возвращаем признак успешного удаления интерфейса
	//
	return QUuid ();
}

QString
OAF::CUIManager::purgeUI (const QString& _uidef)
{
	//
	// Описание созданной части интерфейса пользователя
	//
	QString uidef_to;
	QXmlStreamWriter xmldef_to (&uidef_to);
	xmldef_to.setAutoFormatting (false);
	xmldef_to.writeStartDocument ("1.0");
	xmldef_to.writeStartElement ("uidef");

	//
	// Обработка описания интерфейса и очистка заданной части
	//
	QXmlStreamReader xmldef_from (_uidef);
	if (xmldef_from.readNextStartElement () && (xmldef_from.name () == "uidef"))
	{
		while (xmldef_from.readNextStartElement ())
		{
			//
			// Обработка тэгов "bag"
			//
			if  (xmldef_from.name () == "bag")
			{
				//
				// Если контейнерный виджет для вставки элемента интерфейса известен
				//
				if (m_containers.contains (xmldef_from.attributes ().value ("id").toString ()))
				{
					//
					// То пропускаем элемент
					//
					if (!xmldef_from.isEndElement ())
						xmldef_from.skipCurrentElement ();
				}
				else
				{
					//
					// Иначе копируем элемент
					//
					copyElement (xmldef_from, xmldef_to);
				}
			}
			else
			{
				//
				// Иначе копируем элемент
				//
				copyElement (xmldef_from, xmldef_to);
			}
		}
	}

	//
	// Возвращаем очищенное описание интерфейса
	//
	xmldef_to.writeEndElement ();
	xmldef_to.writeEndDocument ();
	return uidef_to;
}

OAF::IUIComponent*
OAF::CUIManager::ownerOf (QObject* _object) const
{
	for (auto i = m_uidescs.constBegin (); i != m_uidescs.constEnd (); ++i)
	{
		if (i->ownerOf (_object))
			return i->uic ();
	}

	return NULL;
}
