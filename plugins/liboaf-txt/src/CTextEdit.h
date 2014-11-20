/**
 * @file
 * @brief Интерфейс текстового редактора
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CTEXTEDIT_H
#define __OAF_TXT_CTEXTEDIT_H

#include <QAction>
#include <QWidgetAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFontComboBox>
#include <QTextDocument>
#include <QTextEdit>
#include <QSignalMapper>

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
		class CTextEdit : public QTextEdit,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IUnknownExtender,
			virtual public OAF::IUIComponent,
			//
			// Внутренние интерфейсы
			//
			virtual public OAF::INotifyListener,
			//
			// Импортируемые реализации
			//
			virtual public CUnknown,
			virtual public CCommon
		{
			Q_OBJECT

			QString m_all_images_filter;

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
			 * @name Команды для форматирования текста
			 */
			/** @{*/
			QAction* m_bold_action;
			QAction* m_underline_action;
			QAction* m_italic_action;
			QAction* m_color_action;
			/** @}*/

			/**
			 * @name Команды для задания выравнивания текста
			 */
			/** @{*/
			QActionGroup* m_alignment;
			QAction*      m_align_left_action;
			QAction*      m_align_center_action;
			QAction*      m_align_right_action;
			QAction*      m_align_justify_action;
			/** @}*/

			/**
			 * @name Команды для задания вложенных списков
			 */
			/** @{*/
			QAction* m_inc_indent_action;
			QAction* m_dec_indent_action;
			/** @}*/

			/**
			 * @name Команды для вставки и редактирования таблицы
			 */
			/** @{*/
			QAction* m_insert_table_action;
			QAction* m_remove_row_action;
			QAction* m_remove_col_action;

			QSignalMapper* m_sig_mapper_row;
			QAction* m_add_row_action_1;
			QAction* m_add_row_action_2;
			QAction* m_add_row_action_3;
			QAction* m_add_row_action_4;
			QAction* m_add_row_action_n;

			QSignalMapper* m_sig_mapper_col;
			QAction* m_add_col_action_1;
			QAction* m_add_col_action_2;
			QAction* m_add_col_action_3;
			QAction* m_add_col_action_4;
			QAction* m_add_col_action_n;

			QAction* m_merge_adjacent_cells_action;
			QAction* m_split_cell_action;
			QAction* m_table_appreance_action;
			QAction* m_change_rows_height_action;
			QAction* m_change_cols_width_action;
			/** @}*/

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
			QAction* m_image_action;
			QAction* m_resize_action;
			/** @}*/

			/**
			 * @brief Команда экспорта rich-текста в формат PDF
			 */
			QAction* m_export_pdf_action;

			/**
			 * @brief Виджет для отображения списка шрифтов
			 */
			QFontComboBox* m_font_list_widget;

			/**
			 * @brief Экшен для включения виджета отображения списка шрифтов в тулбар
			 */
			QWidgetAction* m_font_list;

			/**
			 * @brief Виджет для отображения списка размеров шрифта
			 */
			QComboBox* m_size_list_widget;

			/**
			 * @brief Экшен для включения виджета отображения списка размеров шрифта в тулбар
			 */
			QWidgetAction* m_size_list;

			/**
			 * @brief Виджет для отображения списка стилей
			 */
			QComboBox* m_list_style_widget;

			/**
			 * @brief Экшен для включения виджета отображения списка стилей в тулбар
			 */
			QWidgetAction* m_list_style;

			/**
			 * @name Набор контекстных меню редактора
			 */
			/** @{*/
			QMenu* m_context_menu;
			QMenu* m_add_rows_submenu;
			QMenu* m_add_cols_submenu;
			/** @}*/

			bool clipboardHasData ();

			void mergeFormatOnWordOrSelection (const QTextCharFormat&);
			void dropImage (const QImage&);
			void dropTextFile (const QUrl&);

			void fontChanged (const QFont&);
			void colorChanged (const QColor&);
			void alignmentChanged (Qt::Alignment);
			void listStyleChanged (QTextListFormat::Style);

			void createFileActions ();
			void createEditActions ();
			void createTextActions ();
			void createListActions ();
			void createTableActions ();
			void createFontComboAction ();
			void createSizeComboAction ();
			void createListComboAction ();

			void createContextMenu ();
			void setupActions ();

			void enableTableActions (bool bEnable = true);

			/**
			* @name Реализация интерфейса OAF::INotifiListener
			*/
			/** @{*/
			void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin);
			/** @}*/

		private slots:
			void aboutFilePrintPdf ();

			void aboutTextBold ();
			void aboutTextUnderline ();
			void aboutTextItalic ();
			void aboutTextFamily (const QString&);
			void aboutTextSize (const QString&);
			void aboutListStyle (int _styleIndex);
			void aboutTextColorChanged ();
			void aboutTextAlign (QAction*);

			void aboutIndentListLess ();
			void aboutIndentListMore ();
			void aboutRemoveFromList (QTextCursor cursor = QTextCursor());

			void aboutInsertTable ();
			void aboutRemoveRow ();
			void aboutRemoveCol ();
			void aboutAddRow (int _count);
			void aboutAddCol (int _count);
			void aboutMergeCells ();
			void aboutSplitCells ();
			void aboutTableAppreance ();
			void aboutRowsHeight ();
			void aboutColsWidth ();

			void aboutInsertImage ();
			void aboutResizeImage ();

			void aboutCurrentCharFormatChanged (const QTextCharFormat&);
			void aboutCursorPositionChanged ();
			void aboutClipboardDataChanged ();

		protected:
			bool canInsertFromMimeData (const QMimeData* source) const;
			void insertFromMimeData (const QMimeData* source);
			void contextMenuEvent (QContextMenuEvent*);

		public:
			CTextEdit (CFactory*);
			~CTextEdit ();

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

#endif /* __OAF_TXT_CTEXTEDIT_H */
