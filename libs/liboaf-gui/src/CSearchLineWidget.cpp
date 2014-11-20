/**
 * @file
 * @brief Реализация строки поиска объектов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QKeyEvent>

#include <OAF/CSearchLineWidget.h>
#include "ui_CSearchLineWidget.h"

using namespace OAF;

/**
 * @brief Макро для подключения поисковика к интерфейсу пользователя
 *
 * Модель может отсутствовать
 */
#define SET_VFHELPER(_find_helper, _search, _model, _w) \
{ \
	QObject::connect ((_search)->find_LE, SIGNAL (textChanged (const QString&)), (_find_helper), SLOT (findMatched (const QString&))); \
	QObject::connect (_w, SIGNAL (activated (const QModelIndex&)), (_find_helper->view ()), SIGNAL (activated (const QModelIndex&))); \
	QObject::connect ((_search)->find_prev_PB, SIGNAL (clicked ()), (_find_helper), SLOT (findPrev ())); \
	QObject::connect ((_search)->find_next_PB, SIGNAL (clicked ()), (_find_helper), SLOT (findNext ())); \
	if ((_model)) \
	{ \
		QObject::connect ((_model), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), (_find_helper), SLOT (findMatched ())); \
		QObject::connect ((_model), SIGNAL (rowsInserted (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
		QObject::connect ((_model), SIGNAL (rowsRemoved (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
	} \
}

/**
 * @brief Макро для отключения поисковика от интерфейса пользователя
 *
 * Модель может отсутствовать, так как была отключена ранее
 */
#define RESET_VFHELPER(_find_helper, _search, _model, _w) \
{ \
	QObject::disconnect ((_search)->find_LE, SIGNAL (textChanged (const QString&)), (_find_helper), SLOT (findMatched (const QString&))); \
	QObject::disconnect (_w, SIGNAL (activated (const QModelIndex&)), (_find_helper->view ()), SIGNAL (activated (const QModelIndex&))); \
	QObject::disconnect ((_search)->find_prev_PB, SIGNAL (clicked ()), (_find_helper), SLOT (findPrev ())); \
	QObject::disconnect ((_search)->find_next_PB, SIGNAL (clicked ()), (_find_helper), SLOT (findNext ())); \
	if ((_model)) \
	{ \
		QObject::disconnect ((_model), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), (_find_helper), SLOT (findMatched ())); \
		QObject::disconnect ((_model), SIGNAL (rowsInserted (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
		QObject::disconnect ((_model), SIGNAL (rowsRemoved (const QModelIndex&, int, int)), (_find_helper), SLOT (findMatched ())); \
	} \
}

bool
CSearchLineWidget::eventFilter (QObject* _o, QEvent* _ev)
{
	//
	// Обрабатываем нажатие клавиши "Вверх", "Вниз" в текстовом редакторе строки для поиска
	//
	if (_o == m_ui->find_LE)
	{
		if (_ev->type () == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast <QKeyEvent *> (_ev);
			if (ke->key () == Qt::Key_Up)
			{
				//
				// Выбираем предыдущий найденный элемент в представлении
				//
				if (m_finder.data ())
					m_finder->findPrev ();

				return true;
			}
			else if (ke->key () == Qt::Key_Down)
			{
				//
				// Выбираем следующий найденный элемент в представлении
				//
				if (m_finder.data ())
					m_finder->findPrev ();

				return true;
			}
			else if (ke->key () == Qt::Key_Return)
			{
				//
				// Открываем редактор для текущего выделенного элемента в дереве
				//
				if (m_finder.data () && m_finder->view ())
				{
					QModelIndexList mil = m_finder->view ()->selectionModel ()->selectedIndexes ();
					if (! mil.empty ())
					{
						emit activated (mil.first ());

						//
						// Возвращаем фокус ввода редактору строки для поиска,
						// чтобы пользователь мог выбрать другое требование
						//
						m_ui->find_LE->setFocus ();

						return true;
					}
				}
			}
		}
	}
	else
	{
		//
		// Передаем неинтересное нам событие обработчику базового класса
		//
		return QWidget::eventFilter (_o, _ev);
	}

	return false;
}

CSearchLineWidget::CSearchLineWidget (QWidget* _parent) :
	QWidget (_parent), m_ui (new Ui::CSearchLineWidget)
{
	m_ui->setupUi (this);

	//
	// Подключаемся к событиям редактора для отслеживания нажатия некоторых клавиш
	//
	m_ui->find_LE->installEventFilter (this);
}

CSearchLineWidget::~CSearchLineWidget ()
{
	setView (NULL);

	//
	// Отключаемся от событий редактора
	//
	m_ui->find_LE->removeEventFilter (this);
}

void
CSearchLineWidget::setView (QAbstractItemView* _view)
{
	//
	// Если поиск ранее уже был настроен
	//
	if (m_finder.data ())
	{
		//
		// Отключаем поисковик
		//
		RESET_VFHELPER (m_finder.data (), m_ui, m_finder->view ()->model (), this);
		//
		// И удаляем его
		//
		m_finder.reset (NULL);
	}

	//
	// Если задано новое представление для посика
	//
	if (_view)
	{
		//
		// Создадим новый поисковик
		//
		m_finder.reset (new OAF::Helpers::ViewFindHelper (_view));
		//
		// И подключим его к поиску
		//
		SET_VFHELPER (m_finder.data (), m_ui, _view->model (), this);
	}
}

void
CSearchLineWidget::setSearchColumn (int _idx)
{
	if (m_finder)
		m_finder->setSearchColumn (_idx);
}
