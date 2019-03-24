/**
 * @file
 * @brief Интерфейс HTML редактора
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CHTMLEDIT_H
#define __OAF_TXT_CHTMLEDIT_H

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
		 * @brief HTML редактор
		 */
		class CHTMLEdit : public QTextEdit,
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

			/**
			 * @brief Фильтр для выбора всех изображений в диалоге вставки изображения
			 */
			QString m_all_images_filter;

			/**
			 * @brief Проверка наличия в буфере обмена данных для вставки
			 */
			bool clipboardHasData ();

			/**
			 * @brief Применить заданный формат к слову/выделенному фрагменту
			 */
			void mergeFormatOnWordOrSelection (const QTextCharFormat& _tcf);

			/**
			 * @brief Вставить заданное изображение в позицию курсора
			 */
			void dropImage (const QImage& _image);

			/**
			 * @brief Вставить заданный текстовый файл в позицию курсора
			 */
			void dropTextFile (const QUrl& _url);

			/**
			 * @brief Изменить интерфейс под заданный фонт
			 */
			void fontChanged (const QFont& _f);

			/**
			 * @brief Изменить интерфейс под заданный цвет
			 */
			void colorChanged (const QColor& _c);

			/**
			 * @brief Изменить интерфейс под заданное выравнивание
			 */
			void alignmentChanged (Qt::Alignment _a);

			/**
			 * @brief Изменить интерфейс под заданный тип списка
			 *
			 * Если курсор находится на списке, то выбираем его стиль в
			 * соответствующем комбобоксе. Иначе выбираем стандартный, т.е.
			 * отсутствие списка
			 */
			void listStyleChanged (QTextListFormat::Style _style);

			/**
			 * @brief Создать действия редактирования
			 */
			void createEditActions ();

			/**
			 * @brief Создать дейтсвия форматирования текста
			 */
			void createTextActions ();

			/**
			 * @brief Создать действия управления списками
			 */
			void createListActions ();

			/**
			 * @brief Создать действия редактирования таблиц
			 */
			void createTableActions ();

			/**
			 * @brief Создать список шрифтов
			 */
			void createFontComboAction ();

			/**
			 * @brief Создать список размеров шрифта
			 */
			void createSizeComboAction ();

			/**
			 * @brief Создать список списков
			 */
			void createListComboAction ();

			/**
			 * @brief Создать контектное меню и его субменю
			 */
			void createContextMenu ();

			/**
			 * @brief Настроить действия
			 */
			void setupActions ();

			/**
			 * @brief Настройка действий по редактированию таблиц
			 */
			void enableTableActions (bool _enable = true);

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
			 * @brief Выключить/выключить жирный текст для слова/выделенного фрагмента
			 */
			void aboutTextBold ();

			/**
			 * @brief Включить/выключить подчёркивание для слова/выделенного фрагмента
			 */
			void aboutTextUnderline ();

			/**
			 * @brief Включить/выключить наклон для слова/выделенного фрагмента
			 */
			void aboutTextItalic ();

			/**
			 * @brief Установить заданный шрифт для слова/выделенного фрагмента
			 */
			void aboutTextFamily (const QString& _f);

			/**
			 * @brief Установить заданный размер шрифта для слова/выделенного фрагмента
			 */
			void aboutTextSize (const QString&);

			/**
			 * @brief Установить заданный тип списка для блока
			 */
			void aboutListStyle (int _style_index);

			/**
			 * @brief Установить выбранный цвет для слова/выделенного фрагмента
			 */
			void aboutTextColorChanged ();

			/**
			 * @brief Установить выравнивание для блока
			 */
			void aboutTextAlign (QAction* _a);

			/**
			 * @brief Уменьшить отступ для блока
			 */
			void aboutIndentListLess ();

			/**
			 * @brief Увеличить отступ для блока
			 */
			void aboutIndentListMore ();

			/**
			 * @brief Удалить заданный блок из списка
			 */
			void aboutRemoveFromList (QTextCursor _cursor = QTextCursor());

			/**
			 * @brief Вставить таблицу в позиции курсора
			 */
			void aboutInsertTable ();

			/**
			 * @brief Удалить строку в позиции курсора
			 */
			void aboutRemoveRow ();

			/**
			 * @brief Удалить столбец в позции курсора
			 */
			void aboutRemoveCol ();

			/**
			 * @brief Добавить заданное число строк
			 */
			void aboutAddRow (int _count);

			/**
			 * @brief Добавить заданное число столбцов в позиции курсора
			 */
			void aboutAddCol (int _count);

			/**
			 * @brief Объединить выбранные ячейки
			 */
			void aboutMergeCells ();

			/**
			 * @brief Разделить ячейку в позиции курсора
			 */
			void aboutSplitCells ();

			/**
			 * @brief Настроить внешний вид таблицы в позиции курсора
			 */
			void aboutTableAppreance ();

			/**
			 * @brief Установить высоту выбранных строк
			 */
			void aboutRowsHeight ();

			/**
			 * @brief Установить ширину выбранных столбцов
			 */
			void aboutColsWidth ();

			/**
			 * @brief Вставить изображение в позицию курсора
			 */
			void aboutInsertImage ();

			/**
			 * @brief Изменить размер выбранного изображения
			 */
			void aboutResizeImage ();

			/**
			 * @brief Изменить интерфейс под заданный формат
			 */
			void aboutCurrentCharFormatChanged (const QTextCharFormat& _tcf);

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
			CHTMLEdit (CFactory* _factory);
			~CHTMLEdit ();

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

#endif /* __OAF_TXT_CHTMLEDIT_H */
