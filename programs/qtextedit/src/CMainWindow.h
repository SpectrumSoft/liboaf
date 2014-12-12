/**
 * @file
 * @brief Интерфейс главного окна приложения
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CMAIN_WINDOW_H
#define __CMAIN_WINDOW_H

#include <QList>
#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QStackedWidget>
#include <QStatusBar>
#include <QMessageBox>

#include <idl/IUIComponent.h>
#include <OAF/OAF.h>
#include <OAF/CUIManager.h>

/**
 * @brief Главное окно приложения
 */
class CMainWindow : public QMainWindow,
	//
	// Экспортируемые интерфейсы
	//
	virtual public OAF::IUIContainer,
	virtual public OAF::IUIComponent
{
	Q_OBJECT
	/**
	 * @brief Текстовый редактор
	 */
	OAF::URef<OAF::IUIComponent> m_document;

	/**
	 * @brief Главное меню
	 */
	QMenuBar* m_menubar;

	/**
	 * @brief Главный виджет окна
	 */
	QStackedWidget* m_main;

	/**
	 * @brief Строка статуса
	 */
	QStatusBar* m_statusbar;

	/**
	 * @name Стандартные действия
	 */
	/** @{*/
	QAction* m_new_window;
	QAction* m_new_text;
	QAction* m_new_html;
	QAction* m_new_lyx;
	QAction* m_open;
	QAction* m_save;
	QAction* m_save_as;
	QAction* m_close;
	QAction* m_quit;
	QAction* m_about;
	/** @}*/

	/**
	 * @brief Менеджер интерфейса
	 */
	OAF::CUIManager m_manager;

	/**
	 * @brief Собственный интерфейс
	 */
	QUuid m_ui;

	/**
	 * @brief Проверка несохранённого документа
	 */
	QMessageBox::StandardButton check4save ();

	/**
	 * @brief Задать новый документ
	 */
	void setDocument (const QString& _path, OAF::URef<OAF::IUIComponent> _document);

	/**
	 * @brief Создать новый документ заданного типа
	 */
	void newDocument (const QStringList& _mime_types);

	/**
	 * @brief Реакция на команду закрытия окна
	 */
	void closeEvent (QCloseEvent* _e);

private slots:
	/**
	 * @brief Открыть новое окно
	 */
	void aboutNewWindow ();

	/**
	 * @brief Создать новый текстовый документ
	 */
	void aboutNewText ();

	/**
	 * @brief Создать новый HTML документ
	 */
	void aboutNewHTML ();

	/**
	 * @brief Создать новый LyX документ
	 */
	void aboutNewLyX ();

	/**
	 * @brief Открыть документ
	 */
	void aboutOpen ();

	/**
	 * @brief Сохранить документ
	 */
	void aboutSave ();

	/**
	 * @brief Сохранить документ под новым именем
	 */
	void aboutSaveAs ();

	/**
	 * @brief Открыть диалог "О программе"
	 */
	void aboutAboutDialog ();

public:
	CMainWindow ();
	~CMainWindow ();

	/**
	 * @name Реализация методов интерфейса OAF::IUIContainer
	 */
	/** @{*/
	QUuid addUI (const QString& _uidef, OAF::IUIComponent* _uic);
	QUuid removeUI (const QUuid& _ui);
	/** @}*/

	/**
	 * @name Реализация интерфейса OAF::IUIComponent
	 */
	/** @{*/
	void setUILabel (const QString& _bag, const QString& _label);
	void setUIContainer (OAF::IUIContainer* _uic);
	OAF::IUIContainer* getUIContainer ();
	void activate (bool _activate);
	QObject* getUIItem (const QString& _id);
	/** @}*/
};

#endif /* __CMAIN_WINDOW_H */
