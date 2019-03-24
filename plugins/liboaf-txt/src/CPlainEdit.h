/**
 * @file
 * @brief Интерфейс текстового редактора
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CPLAINEDIT_H
#define __OAF_TXT_CPLAINEDIT_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#include <QtPrintSupport>
#endif

#include <idl/IUnknown.h>
#include <idl/INotifySource.h>
#include <idl/IUIComponent.h>

#include <OAF/CUnknown.h>

#include "CCommon.h"
#include "CFactory.h"

namespace OAF
{
	namespace TXT
	{
		/**
		 * @brief Текстовый редактор
		 */
		class CPlainEdit : public QTextEdit,
			//
			// Импортируемые реализации
			//
			public CUnknown,
			public CCommon,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IUnknownExtender,
			virtual public OAF::IUIComponent,
			//
			// Внутренние интерфейсы
			//
			virtual public OAF::INotifyListener
		{
			Q_OBJECT

			/**
			 * @brief Фабрика компонента
			 */
			OAF::URef<CFactory> m_factory;

			/**
			 * @brief Текстовый документ для редактирования
			 */
			OAF::URef<QTextDocument> m_document;

			/**
			 * @brief Метка для добавления редактора в ui:main
			 */
			QString m_main_label;

			/**
			 * @brief Контейнер
			 */
			OAF::IUIContainer* m_uic;

			/**
			 * @brief Идентификатор постоянного интерфейса (т.е. самого текстового редактора)
			 */
			QUuid m_id;

			/**
			 * @brief Идентификатор динамического интерфейса (т.е. команд для тулбара и меню)
			 */
			QUuid m_id_active;

			/**
			 * @name Общие команды редактирования
			 */
			/** @{*/
			QAction* m_undo_action;
			QAction* m_redo_action;
			QAction* m_cut_action;
			QAction* m_copy_action;
			QAction* m_paste_action;
			QAction* m_delete_action;
			QAction* m_select_all_action;
			/** @}*/

			/**
			 * @name Набор контекстных меню редактора
			 */
			/** @{*/
			QMenu* m_context_menu;
			/** @}*/

			/**
			 * @brief Проверка наличия в буфере обмена данных для вставки
			 */
			bool clipboardHasData ();

			/**
			 * @brief Создать команды для редактирования текста
			 */
			void createEditActions ();

			/**
			 * @brief Создать контекстное меню
			 */
			void createContextMenu ();

			/**
			 * @brief Настройка команд
			 */
			void setupActions ();

			/**
			 * @brief Вставить заданный текстовый файл в позицию курсора
			 */
			void dropTextFile (const QUrl& _url);

			/**
			 * @name Реализация интерфейса OAF::INotifiListener
			 *
			 * Для отслеживания изменений в редактируемом документе, сделанных
			 * вне данного редактора.
			 */
			/** @{*/
			void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin);
			/** @}*/

		private slots:
			/**
			 * @brief Реакция на изменение положения курсора
			 */
			void aboutCursorPositionChanged ();

			/**
			 * @brief Реакция на изменение буфера обмена
			 */
			void aboutClipboardDataChanged ();

		protected:
			/**
			 * @brief Проверка возможности вставки заданных данных
			 */
			bool canInsertFromMimeData (const QMimeData* _source) const;

			/**
			 * @brief Вставить заданные данные в позицию курсора
			 */
			void insertFromMimeData (const QMimeData* _source);

			/**
			 * @brief Обработать запрос контекстного меню
			 */
			void contextMenuEvent (QContextMenuEvent* _event);

		public:
			CPlainEdit (CFactory* _factory);
			~CPlainEdit ();

			/**
			 * @name Реализация интерфейса OAF::IUnknownExtender
			 */
			/** @{*/
			OAF::URef<OAF::IUnknown> setExtendedObject (OAF::IUnknown* _object);
			/** @}*/

			/**
			 * @name Реализация интерфейса IUComponent
			 */
			/** @{*/
			void setUILabel (const QString& _bag, const QString& _label);
			void setUIContainer (OAF::IUIContainer* _uic);
			OAF::IUIContainer* getUIContainer ();
			void activate (bool _activate);
			QObject* getUIItem (const QString& _id);
			/** @}*/
		};
	}
}

#endif /* __OAF_TXT_CPLAINEDIT_H */
