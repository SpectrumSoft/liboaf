/**
 * @file
 * @brief Реализация менеджера интерфейса пользователя
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CUIMANAGER_H
#define __CUIMANAGER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <idl/IUIComponent.h>

#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	/**
	 * @brief Менеджер интерфейса пользователя
	 *
	 * Весь интерфейс пользователя состоит из контейнерных виджетов, в которые добавлены элементы
	 * интерфейса отдельных компонентов. С точки зрения управления весь интерфейс состоит из набора
	 * частей, которые добавляются и удаляются как единое целое в соответствии с текущим состоянием
	 * компонентов.
	 *
	 * Папки могут быть добавлены в:
	 *    - QMenuBar - произвольной вложенности;
	 *    - QMenu - произвольной вложенности;
	 *    - QMainWindow - один уровень вложенности (для заданной панели инструментов).
	 *
	 * QAction могут быть добавлены в:
	 *    - QMenuBar;
	 *    - QToolBar;
	 *    - QMenu;
	 *    - QActionGroup.
	 *
	 * С помощью обёртки QWidgetAction в меню могут быть добавлены произвольные виджеты, но этот
	 * случай явно не обрабатывается из-за его относительной редкости.
	 *
	 * QWidget могут быть добавлены в:
	 *    - QTabWidget;
	 *    - QToolBox;
	 *    - QToolBar;
	 *    - QStackedWidget;
	 *    - QStatusBar.
	 *
	 * При добавлении виджетов в QStatusBar необходимо иметь в виду, что он захватывает владение
	 * виджетами, поэтому при их удалении из него выполняется setParent (NULL) и в компоненте
	 * необходимо выполнять явное удаление таких виджетов не надеясь на внутренние механизмы Qt
	 * (а лучше это делать для всех виджетов).
	 */
	class OAFGUI_EXPORT CUIManager
	{
		/**
		 * @brief Описание созданной части интерфейса пользователя
		 */
		class UI
		{
			/**
			 * @brief XML-описание созданного интерфейса
			 *
			 * Данное XML-описание повторяет по структуре XML-описание интерфейса, переданное
			 * при вставке интерфейса, только для каждого XML-элемента задан единственный атрибут
			 * "p", который содержит адрес (в шестнадцатиричном виде) соответствующего объекта
			 * в памяти.
			 *
			 * Главным элементом данного описания является "ui". Остальные элементы имеют те же
			 * имена, что и для "uidef", только для каждого определён единственный атрибут "p"
			 */
			QString m_desc;

			/**
			 * @brief Компонент, для которого создано описание
			 */
			OAF::IUIComponent* m_uic;

			/**
			 * @brief Множество использованных в интерфейсе элементов компонента
			 *
			 * Используется для проверки принадлежности элемента интерфейса компоненту
			 */
			QSet<QObject*> m_items;

		public:
			UI ();
			UI (OAF::IUIComponent* _uic);
			UI (const UI& _ui);

			/**
			 * @brief Добавить элемент интерфейса пользователя к списку
			 */
			void add (QObject* _object);

			/**
			 * @brief Удалить элемент интерфейса пользователя
			 */
			void remove (QObject* _object);

			/**
			 * @brief Задать описание добавленного интерфейса
			 */
			void set (const QString& _desc);

			/**
			 * @brief Получить описание добавленного интерфейса
			 */
			const QString& get () const;

			/**
			 * @brief Получить компонент, для которого создана данная часть инетрфейса пользователя
			 */
			OAF::IUIComponent* uic () const;

			/**
			 * @brief Проверить принадлежность элемента интерфейса
			 */
			bool ownerOf (QObject* _object) const;
		};

		/**
		 * @brief Контейнеры элементов интерфейса пользователя, с которыми работает этот менежер
		 */
		QMap<QString, QObject*> m_containers;

		/**
		 * @brief Набор созданных частей интерфейса пользователя
		 */
		QMap<QUuid, UI> m_uidescs;

		static void addFolder (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui);
		static void removeFolder (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui);
		static void addItem (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui);
		static void removeItem (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui);
		static void addSeparator (QXmlStreamReader& _xmldef, QXmlStreamWriter& _xmldesc, QObject* _to, UI& _ui);
		static void removeSeparator (QXmlStreamReader& _xmldesc, QObject* _from, UI& _ui);
		static void copyElement (QXmlStreamReader& _from, QXmlStreamWriter& _to);

	public:
		CUIManager ();
		~CUIManager ();

		/**
		 * @brief Добавить контейнерный виджет
		 */
		void install (const QString& _name, QObject* _c);

		/**
		 * @brief Проверить, что заданный контейнерный виджет установлен
		 */
		bool isInstalled (const QString& _name) const;

		/**
		 * @brief Проверить, что заданный контейнерный виджет установлен
		 */
		bool isInstalled (QObject* _c) const;

		/**
		 * @brief Удалить контейнерный виджет
		 */
		void remove (const QString& _name);

		/**
		 * @brief Очистить менеджер
		 *
		 * В момент очистки менеджера все контейнеры интерфейса в нём должны быть пустыми
		 */
		void clear ();

		/**
		 * @brief Добавить интерфейс
		 *
		 * Все элементы интерфейса компонента @a _uic, описанные в @a _uidef и относящиеся к
		 * контейнерным виджетам, которые привязаны к данному менеджеру, добавляется в них
		 */
		QUuid addUI (const QString& _uidef, OAF::IUIComponent* _uic);

		/**
		 * @brief Удалить интерфейс
		 *
		 * Все элементы интерфейса, связанные с идентификатором @a _id удаляются из контейнерных
		 * виджетов
		 */
		QUuid removeUI (const QUuid& _id);

		/**
		 * @brief Удалить из описания интерфейса обработанные элементы
		 *
		 * Из заданного описания интерфейса удаляются элементы, которые относятся к контейнерным
		 * виджетам, привязанным к данному менеджеру
		 */
		QString purgeUI (const QString& _uidef);

		/**
		 * @brief Получить указатель на компонент для заданного элемента интерфейса
		 */
		OAF::IUIComponent* ownerOf (QObject* _object) const;
	};
}

#endif /* __CUIMANAGER_H */
