/**
 * @file
 * @brief Реализация текстового редактора
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QApplication>
#include <QPrinter>
#include <QClipboard>
#include <QMimeData>
#include <QImageReader>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>

#include <QAction>
#include <QWidgetAction>
#include <QTextList>
#include <QTextTable>
#include <QMenu>
#include <QContextMenuEvent>

#include <OAF/OAF.h>
#include <OAF/StreamUtils.h>
#include <OAF/Helpers.h>
#include <OAF/MimeHelpers.h>
#include <OAF/CNotifySource.h>

#include "CTextEdit.h"

#include "ui_CResizeDialog.h"
#include "ui_CTableDialog.h"
#include "ui_CInputDialog.h"
#include "ui_CTableAppreance.h"

using namespace OAF::TXT;

bool
CTextEdit::clipboardHasData ()
{
	if (const QMimeData* mime = QApplication::clipboard ()->mimeData ())
		return mime->hasText () || mime->hasHtml ();

	return false;
}

void
CTextEdit::mergeFormatOnWordOrSelection (const QTextCharFormat& _format)
{
	QTextCursor cursor = textCursor ();
	if (!cursor.hasSelection ())
		cursor.select (QTextCursor::WordUnderCursor);
	cursor.mergeCharFormat (_format);
	mergeCurrentCharFormat (_format);
}

void
CTextEdit::fontChanged (const QFont& _f)
{
	m_font_list_widget->setCurrentFont (_f);
	m_size_list_widget->setCurrentIndex (m_size_list_widget->findText (QString::number (_f.pointSize ())));

	m_bold_action->setChecked (_f.bold ());
	m_italic_action->setChecked (_f.italic ());
	m_underline_action->setChecked (_f.underline ());
}

void
CTextEdit::colorChanged (const QColor& _c)
{
	QPixmap pix (16, 16);
	pix.fill (_c);
	m_color_action->setIcon (pix);
}

void
CTextEdit::alignmentChanged (Qt::Alignment _a)
{
	if (_a & Qt::AlignLeft)
		m_align_left_action->setChecked (true);
	else if (_a & Qt::AlignHCenter)
		m_align_center_action->setChecked (true);
	else if (_a & Qt::AlignRight)
		m_align_right_action->setChecked (true);
	else if (_a & Qt::AlignJustify)
		m_align_justify_action->setChecked (true);
	else Q_ASSERT(0);
}

//
// Если курсор находится на списке, то выбираем его стиль в соответствующем комбобоксе;
// иначе выбираем стандартный, т.е. отсутствие списка
//
void
CTextEdit::listStyleChanged (QTextListFormat::Style _style)
{
	int style_index = 0;
	switch (_style)
	{
		case QTextListFormat::ListDisc:
			style_index = 1;
			break;
		case QTextListFormat::ListCircle:
			style_index = 2;
			break;
		case QTextListFormat::ListSquare:
			style_index = 3;
			break;
		case QTextListFormat::ListDecimal:
			style_index = 4;
			break;
		case  QTextListFormat::ListLowerAlpha:
			style_index = 5;
			break;
		case QTextListFormat::ListUpperAlpha:
			style_index = 6;
			break;
		case QTextListFormat::ListLowerRoman:
			style_index = 7;
			break;
		case QTextListFormat::ListUpperRoman:
			style_index = 8;
			break;
		default:
			break;
	}

	m_list_style_widget->setCurrentIndex (style_index);
}

void
CTextEdit::createFileActions ()
{
	m_export_pdf_action = new QAction (tr ("&Export PDF ..."), this);
	m_export_pdf_action->setIcon (QIcon::fromTheme ("export-pdf", QIcon (":/liboaf-txt/icons/exportpdf.png")));
	m_export_pdf_action->setPriority (QAction::LowPriority);
	m_export_pdf_action->setShortcut (Qt::CTRL + Qt::Key_D);
	m_export_pdf_action->setObjectName ("ui:textedit:exportpdf");

	connect (m_export_pdf_action, SIGNAL (triggered ()), this, SLOT (aboutFilePrintPdf ()));
}

void
CTextEdit::createEditActions ()
{
	m_undo_action = new QAction (tr ("&Undo"), this);
	m_undo_action->setIcon (QIcon::fromTheme ("edit-undo", QIcon (":/liboaf-txt/icons/editundo.png")));
	m_undo_action->setPriority (QAction::LowPriority);
	m_undo_action->setShortcut (QKeySequence::Undo);
	m_undo_action->setObjectName ("ui:textedit:undo");

	m_redo_action = new QAction (tr ("&Redo"), this);
	m_redo_action->setIcon (QIcon::fromTheme ("edit-redo", QIcon (":/liboaf-txt/icons/editredo.png")));
	m_redo_action->setPriority (QAction::LowPriority);
	m_redo_action->setShortcut (QKeySequence::Redo);
	m_redo_action->setObjectName ("ui:textedit:redo");

	m_cut_action = new QAction (tr ("Cu&t"), this);
	m_cut_action->setIcon (QIcon::fromTheme ("edit-cut", QIcon (":/liboaf-txt/icons/editcut.png")));
	m_cut_action->setPriority (QAction::LowPriority);
	m_cut_action->setShortcut (QKeySequence::Cut);
	m_cut_action->setObjectName ("ui:textedit:cut");

	m_copy_action = new QAction (tr ("&Copy"), this);
	m_copy_action->setIcon (QIcon::fromTheme ("edit-copy", QIcon (":/liboaf-txt/icons/editcopy.png")));
	m_copy_action->setPriority (QAction::LowPriority);
	m_copy_action->setShortcut (QKeySequence::Copy);
	m_copy_action->setObjectName ("ui:textedit:copy");

	m_paste_action = new QAction (tr ("&Paste"), this);
	m_paste_action->setIcon (QIcon::fromTheme ("edit-paste", QIcon (":/liboaf-txt/icons/editpaste.png")));
	m_paste_action->setPriority (QAction::LowPriority);
	m_paste_action->setShortcut (QKeySequence::Paste);
	m_paste_action->setObjectName ("ui:textedit:paste");
	m_paste_action->setEnabled (clipboardHasData ());

	m_delete_action = new QAction (tr ("Delete"), this);
	m_delete_action->setPriority (QAction::LowPriority);
	m_delete_action->setObjectName ("ui:textedit:delete");
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());

	m_select_all_action = new QAction (tr ("Select &All"), this);
	m_select_all_action->setPriority (QAction::LowPriority);
	m_select_all_action->setObjectName ("ui:textedit:select_all");
	m_select_all_action->setShortcut (QKeySequence::SelectAll);
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());

	m_image_action = new QAction (tr("&Insert image"), this);
	m_image_action->setIcon (QIcon::fromTheme ("insert-image", QIcon (":/liboaf-txt/icons/image_add.png")));
	m_image_action->setPriority (QAction::LowPriority);
	//TODO: m_image_action->setShortcut(Qt::CTRL + Qt::Key_I);
	m_image_action->setObjectName ("ui:textedit:insert_image");

	m_resize_action = new QAction (tr("Resize image"), this);
	m_resize_action->setIcon (QIcon::fromTheme ("resize-image", QIcon (":/liboaf-txt/icons/image_resize.png")));
	m_resize_action->setPriority (QAction::LowPriority);
	//TODO: m_resize_action->setShortcut (Qt::CTRL + Qt::Key_R);
	m_resize_action->setObjectName ("ui:textedit:resize_image");
}

void
CTextEdit::createTextActions ()
{
	m_bold_action = new QAction (tr ("&Bold"), this);
	m_bold_action->setIcon (QIcon::fromTheme ("format-text-bold", QIcon (":/liboaf-txt/icons/textbold.png")));
	m_bold_action->setShortcut (Qt::CTRL + Qt::Key_B);
	m_bold_action->setPriority (QAction::LowPriority);
	m_bold_action->setObjectName ("ui:textedit:bold");
	QFont bold_font;
	bold_font.setBold(true);
	m_bold_action->setFont (bold_font);
	m_bold_action->setCheckable (true);
	connect (m_bold_action, SIGNAL (triggered ()), this, SLOT (aboutTextBold ()));

	m_italic_action = new QAction (tr ("&Italic"), this);
	m_italic_action->setIcon (QIcon::fromTheme ("format-text-italic", QIcon (":/liboaf-txt/icons/textitalic.png")));
	m_italic_action->setPriority (QAction::LowPriority);
	m_italic_action->setShortcut (Qt::CTRL + Qt::Key_I);
	m_italic_action->setObjectName ("ui:textedit:italic");
	QFont italic_font;
	italic_font.setItalic(true);
	m_italic_action->setFont (italic_font);
	m_italic_action->setCheckable(true);
	connect (m_italic_action, SIGNAL (triggered ()), this, SLOT (aboutTextItalic ()));

	m_underline_action = new QAction (tr ("&Underline"), this);
	m_underline_action->setIcon (QIcon::fromTheme ("format-text-underline", QIcon (":/liboaf-txt/icons/textunder.png")));
	m_underline_action->setShortcut (Qt::CTRL + Qt::Key_U);
	m_underline_action->setPriority (QAction::LowPriority);
	m_underline_action->setObjectName ("ui:textedit:underline");
	QFont underline_font;
	underline_font.setUnderline (true);
	m_underline_action->setFont (underline_font);
	m_underline_action->setCheckable(true);
	connect (m_underline_action, SIGNAL (triggered ()), this, SLOT (aboutTextUnderline ()));

	m_alignment = new QActionGroup (this);
	m_alignment->setObjectName ("ui:textedit:alignment");
	connect (m_alignment, SIGNAL (triggered (QAction*)), this, SLOT (aboutTextAlign (QAction*)));

	m_align_left_action = new QAction (tr ("&Left"), m_alignment);
	m_align_left_action->setIcon (QIcon::fromTheme ("format-justify-left", QIcon (":/liboaf-txt/icons/textleft.png")));
	m_align_left_action->setShortcut (Qt::CTRL + Qt::Key_L);
	m_align_left_action->setCheckable (true);
	m_align_left_action->setPriority (QAction::LowPriority);
	m_align_left_action->setObjectName ("ui:textedit:alignment:left");

	m_align_center_action = new QAction (tr ("C&enter"), m_alignment);
	m_align_center_action->setIcon (QIcon::fromTheme ("format-justify-center", QIcon (":/liboaf-txt/icons/textcenter.png")));
	m_align_center_action->setShortcut (Qt::CTRL + Qt::Key_E);
	m_align_center_action->setCheckable (true);
	m_align_center_action->setPriority (QAction::LowPriority);
	m_align_center_action->setObjectName ("ui:textedit:alignment:center");

	m_align_right_action = new QAction (tr ("&Right"), m_alignment);
	m_align_right_action->setIcon (QIcon::fromTheme ("format-justify-right", QIcon (":/liboaf-txt/icons/textright.png")));
	m_align_right_action->setShortcut (Qt::CTRL + Qt::Key_R);
	m_align_right_action->setCheckable (true);
	m_align_right_action->setPriority (QAction::LowPriority);
	m_align_right_action->setObjectName ("ui:textedit:alignment:right");

	m_align_justify_action = new QAction (tr ("&Justify"), m_alignment);
	m_align_justify_action->setIcon (QIcon::fromTheme("format-justify-fill", QIcon(":/liboaf-txt/icons/textjustify.png")));
	m_align_justify_action->setShortcut (Qt::CTRL + Qt::Key_J);
	m_align_justify_action->setCheckable (true);
	m_align_justify_action->setPriority (QAction::LowPriority);
	m_align_justify_action->setObjectName ("ui:textedit:alignment:justify");

	QPixmap pix (16, 16);
	pix.fill (Qt::black);
	m_color_action = new QAction (pix, tr ("&Color ..."), this);
	m_color_action->setObjectName ("ui:textedit:color");
	connect (m_color_action, SIGNAL (triggered ()), this, SLOT (aboutTextColorChanged ()));
}

void
CTextEdit::createListActions ()
{
	m_inc_indent_action = new QAction (tr ("Increase Indent"), this);
	m_inc_indent_action->setIcon (QIcon::fromTheme ("format-indent-more", QIcon (":/liboaf-txt/icons/inc_indent.png")));
	m_inc_indent_action->setShortcut (Qt::ALT + Qt::Key_Right);
	m_inc_indent_action->setPriority (QAction::LowPriority);
	m_inc_indent_action->setObjectName ("ui:textedit:increase_indent");
	connect (m_inc_indent_action, SIGNAL (triggered ()), this, SLOT (aboutIndentListMore ()));

	m_dec_indent_action = new QAction (tr ("Decrease Indent"), this);
	m_dec_indent_action->setIcon (QIcon::fromTheme ("format-indent-less", QIcon (":/liboaf-txt/icons/dec_indent.png")));
	m_dec_indent_action->setShortcut (Qt::ALT + Qt::Key_Left);
	m_dec_indent_action->setPriority (QAction::LowPriority);
	m_dec_indent_action->setObjectName ("ui:textedit:decrease_indent");
	connect (m_dec_indent_action, SIGNAL (triggered ()), this, SLOT (aboutIndentListLess ()));
}

void
CTextEdit::createTableActions ()
{
	m_insert_table_action = new QAction (tr ("Insert Table"), this);
	m_insert_table_action->setIcon (QIcon::fromTheme ("format-insert-table", QIcon (":/liboaf-txt/icons/insert_table.png")));
	//m_insert_table_action->setShortcut (Qt::ALT + Qt::Key_Right);
	m_insert_table_action->setPriority (QAction::LowPriority);
	m_insert_table_action->setObjectName ("ui:textedit:insert_table");
	connect (m_insert_table_action, SIGNAL (triggered ()), this, SLOT (aboutInsertTable ()));

	m_remove_row_action = new QAction (tr ("Remove current row"), this);
	m_remove_row_action->setObjectName ("ui:textedit:remove_row");
	m_remove_row_action->setEnabled (false);
	connect (m_remove_row_action, SIGNAL (triggered ()), this, SLOT (aboutRemoveRow ()));

	m_remove_col_action = new QAction (tr ("Remove current column"), this);
	m_remove_col_action->setObjectName ("ui:textedit:remove_col");
	m_remove_col_action->setEnabled (false);
	connect (m_remove_col_action, SIGNAL (triggered ()), this, SLOT (aboutRemoveCol ()));

	m_sig_mapper_row = new QSignalMapper (this);
	connect (m_sig_mapper_row, SIGNAL (mapped (int)), this, SLOT (aboutAddRow (int)));

	m_add_row_action_1 = new QAction ("1", this);
	m_add_row_action_1->setObjectName ("ui:textedit:add_1_row");
	m_add_row_action_1->setEnabled (false);
	m_sig_mapper_row->setMapping (m_add_row_action_1, 1);
	connect (m_add_row_action_1, SIGNAL (triggered ()), m_sig_mapper_row, SLOT (map ()));

	m_add_row_action_2 = new QAction ("2", this);
	m_add_row_action_2->setObjectName ("ui:textedit:add_2_row");
	m_add_row_action_2->setEnabled (false);
	m_sig_mapper_row->setMapping (m_add_row_action_2, 2);
	connect (m_add_row_action_2, SIGNAL (triggered ()), m_sig_mapper_row, SLOT (map ()));

	m_add_row_action_3 = new QAction ("3", this);
	m_add_row_action_3->setObjectName ("ui:textedit:add_3_row");
	m_add_row_action_3->setEnabled (false);
	m_sig_mapper_row->setMapping (m_add_row_action_3, 3);
	connect (m_add_row_action_3, SIGNAL (triggered ()), m_sig_mapper_row, SLOT (map ()));

	m_add_row_action_4 = new QAction ("4", this);
	m_add_row_action_4->setObjectName ("ui:textedit:add_4_row");
	m_add_row_action_4->setEnabled (false);
	m_sig_mapper_row->setMapping (m_add_row_action_4, 4);
	connect (m_add_row_action_4, SIGNAL (triggered ()), m_sig_mapper_row, SLOT (map ()));

	m_add_row_action_n = new QAction (tr ("Custom ..."), this);
	m_add_row_action_n->setObjectName ("ui:textedit:add_n_row");
	m_add_row_action_n->setEnabled (false);
	m_sig_mapper_row->setMapping (m_add_row_action_n, -1);
	connect (m_add_row_action_n, SIGNAL (triggered ()), m_sig_mapper_row, SLOT (map ()));

	m_sig_mapper_col = new QSignalMapper (this);
	connect (m_sig_mapper_col, SIGNAL (mapped (int)), this, SLOT (aboutAddCol (int)));

	m_add_col_action_1 = new QAction ("1", this);
	m_add_col_action_1->setObjectName ("ui:textedit:add_1_col");
	m_add_col_action_1->setEnabled (false);
	m_sig_mapper_col->setMapping (m_add_col_action_1, 1);
	connect (m_add_col_action_1, SIGNAL (triggered ()), m_sig_mapper_col, SLOT (map ()));

	m_add_col_action_2 = new QAction ("2", this);
	m_add_col_action_2->setObjectName ("ui:textedit:add_2_col");
	m_add_col_action_2->setEnabled (false);
	m_sig_mapper_col->setMapping (m_add_col_action_2, 2);
	connect (m_add_col_action_2, SIGNAL (triggered ()), m_sig_mapper_col, SLOT (map ()));

	m_add_col_action_3 = new QAction ("3", this);
	m_add_col_action_3->setObjectName ("ui:textedit:add_3_col");
	m_add_col_action_3->setEnabled (false);
	m_sig_mapper_col->setMapping (m_add_col_action_3, 3);
	connect (m_add_col_action_3, SIGNAL (triggered ()), m_sig_mapper_col, SLOT (map ()));

	m_add_col_action_4 = new QAction ("4", this);
	m_add_col_action_4->setObjectName ("ui:textedit:add_4_col");
	m_add_col_action_4->setEnabled (false);
	m_sig_mapper_col->setMapping (m_add_col_action_4, 4);
	connect (m_add_col_action_4, SIGNAL (triggered ()), m_sig_mapper_col, SLOT (map ()));

	m_add_col_action_n = new QAction (tr ("Custom ..."), this);
	m_add_col_action_n->setObjectName ("ui:textedit:add_n_col");
	m_add_col_action_n->setEnabled (false);
	m_sig_mapper_col->setMapping (m_add_col_action_n, -1);
	connect (m_add_col_action_n, SIGNAL (triggered ()), m_sig_mapper_col, SLOT (map ()));

	m_merge_adjacent_cells_action = new QAction (tr ("Merge two adjacent cells"), this);
	m_merge_adjacent_cells_action->setObjectName ("ui:textedit:merge_cells");
	m_merge_adjacent_cells_action->setEnabled (false);
	connect (m_merge_adjacent_cells_action, SIGNAL (triggered ()), this, SLOT (aboutMergeCells ()));

	m_split_cell_action = new QAction (tr ("Split the cell into two ones"), this);
	m_split_cell_action->setObjectName ("ui:textedit:split_cells");
	m_split_cell_action->setEnabled (false);
	connect (m_split_cell_action, SIGNAL(triggered ()), this, SLOT (aboutSplitCells ()));

	m_table_appreance_action = new QAction (tr ("Setup table appreance ..."), this);
	m_table_appreance_action->setObjectName ("ui:textedit:table_appreance");
	m_table_appreance_action->setEnabled (false);
	connect (m_table_appreance_action, SIGNAL(triggered ()), this, SLOT (aboutTableAppreance()));

	m_change_rows_height_action = new QAction (tr ("Change rows height ..."), this);
	m_change_rows_height_action->setObjectName ("ui:textedit:rows_height");
	m_change_rows_height_action->setEnabled (false);
	connect (m_change_rows_height_action, SIGNAL(triggered ()), this, SLOT (aboutRowsHeight ()));

	m_change_cols_width_action = new QAction (tr ("Change columns width ..."), this);
	m_change_cols_width_action->setObjectName ("ui:textedit:cols_width");
	m_change_cols_width_action->setEnabled (false);
	connect (m_change_cols_width_action, SIGNAL(triggered ()), this, SLOT (aboutColsWidth ()));
}

void
CTextEdit::createFontComboAction ()
{
	//
	// Создаём список шрифтов
	//
	m_font_list_widget = new QFontComboBox ();
	m_font_list_widget->setToolTip (tr ("Font of selected text"));

	QSizePolicy sp = m_font_list_widget->sizePolicy ();
	sp.setHorizontalPolicy (QSizePolicy::Maximum);
	m_font_list_widget->setSizePolicy (sp);
	m_font_list_widget->setSizeAdjustPolicy (QComboBox::AdjustToMinimumContentsLengthWithIcon);
	connect (m_font_list_widget, SIGNAL (activated (QString)), this, SLOT (aboutTextFamily (QString)));

	//
	// Создаём действие для добавления списка фонтов в панель инструментов.
	// Действие будет владеть виджетом и удалит его при своём удалении
	//
	m_font_list = new QWidgetAction (this);
	m_font_list->setDefaultWidget (m_font_list_widget);
	m_font_list->setObjectName ("ui:textedit:fontlist");
}

void
CTextEdit::createSizeComboAction ()
{
	//
	// Создаём список размеров
	//
	m_size_list_widget = new QComboBox ();
	m_size_list_widget->setToolTip (tr ("Size of selected text"));

	m_size_list_widget->setEditable (true);
	m_size_list_widget->setSizeAdjustPolicy (QComboBox::AdjustToContentsOnFirstShow);

	//
	// Добавляем все доступные в данной системе размеры шрифтов в соответствующий список.
	//
	QFontDatabase db;
	foreach (int size, db.standardSizes ())
		m_size_list_widget->addItem (QString::number(size));

	connect (m_size_list_widget, SIGNAL (activated (QString)), this, SLOT (aboutTextSize (QString)));

	//
	// Устанавливаем размер шрифта по умолчанию (запрашиваем у объекта приложения).
	//
	int default_pt_size = QApplication::font().pointSize();
	m_size_list_widget->setCurrentIndex (m_size_list_widget->findText (QString::number (default_pt_size)));

	//
	// Создаём экшен для добавления списка размеров в тулбар. Экшен будет владеть виджетом
	// и удалит его при своём удалении
	//
	m_size_list = new QWidgetAction (this);
	m_size_list->setDefaultWidget (m_size_list_widget);
	m_size_list->setObjectName ("ui:textedit:fontsize");
}

void
CTextEdit::createListComboAction ()
{
	//
	// Создаём список размеров
	//
	m_list_style_widget = new QComboBox ();
	m_list_style_widget->setToolTip (tr ("List style of selected text"));

	m_list_style_widget->addItem (tr ("Standard"));
	m_list_style_widget->addItem (tr ("Bullet List (Disc)"));
	m_list_style_widget->addItem (tr ("Bullet List (Circle)"));
	m_list_style_widget->addItem (tr ("Bullet List (Square)"));
	m_list_style_widget->addItem (tr ("Ordered List (Decimal)"));
	m_list_style_widget->addItem (tr ("Ordered List (Alpha lower)"));
	m_list_style_widget->addItem (tr ("Ordered List (Alpha upper)"));
	m_list_style_widget->addItem (tr ("Ordered List (Roman lower)"));
	m_list_style_widget->addItem (tr ("Ordered List (Roman upper)"));

	QSizePolicy sp = m_list_style_widget->sizePolicy ();
	sp.setHorizontalPolicy (QSizePolicy::Maximum);
	m_list_style_widget->setSizePolicy (sp);
	m_list_style_widget->setSizeAdjustPolicy (QComboBox::AdjustToContentsOnFirstShow);

	connect (m_list_style_widget, SIGNAL (activated (int)), this, SLOT (aboutListStyle (int)));

	//
	// Создаём экшен для добавления списка в тулбар. Экшен будет владеть виджетом
	// и удалит его при своём удалении
	//
	m_list_style = new QWidgetAction (this);
	m_list_style->setDefaultWidget (m_list_style_widget);
	m_list_style->setObjectName ("ui:textedit:liststyle");
}

void
CTextEdit::createContextMenu ()
{
	m_context_menu = new QMenu (this);

	m_context_menu->addAction (m_undo_action);
	m_context_menu->addAction (m_redo_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_cut_action);
	m_context_menu->addAction (m_copy_action);
	m_context_menu->addAction (m_paste_action);
	m_context_menu->addAction (m_delete_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_select_all_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_remove_row_action);
	m_context_menu->addAction (m_remove_col_action);

	m_add_rows_submenu = m_context_menu->addMenu (tr ("Add rows before selected"));
	m_add_rows_submenu->addAction (m_add_row_action_1);
	m_add_rows_submenu->addAction (m_add_row_action_2);
	m_add_rows_submenu->addAction (m_add_row_action_3);
	m_add_rows_submenu->addAction (m_add_row_action_4);
	m_add_rows_submenu->addAction (m_add_row_action_n);

	m_add_cols_submenu = m_context_menu->addMenu (tr ("Add columns before selected"));
	m_add_cols_submenu->addAction (m_add_col_action_1);
	m_add_cols_submenu->addAction (m_add_col_action_2);
	m_add_cols_submenu->addAction (m_add_col_action_3);
	m_add_cols_submenu->addAction (m_add_col_action_4);
	m_add_cols_submenu->addAction (m_add_col_action_n);

	m_context_menu->addAction (m_merge_adjacent_cells_action);
	m_context_menu->addAction (m_split_cell_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_table_appreance_action);
//	m_context_menu->addAction (m_change_rows_height_action);
	m_context_menu->addAction (m_change_cols_width_action);
}

void
CTextEdit::setupActions ()
{
	//
	// Ловим undo/redo от документа
	//
	connect (document (), SIGNAL (modificationChanged (bool)), this, SLOT (setWindowModified (bool)));
	connect (this, SIGNAL (undoAvailable (bool)), m_undo_action, SLOT (setEnabled (bool)));
	connect (this, SIGNAL (redoAvailable (bool)), m_redo_action, SLOT (setEnabled (bool)));

	m_undo_action->setEnabled (document ()->isUndoAvailable ());
	m_redo_action->setEnabled (document ()->isRedoAvailable ());

	connect (m_undo_action, SIGNAL (triggered ()), this, SLOT (undo ()));
	connect (m_redo_action, SIGNAL (triggered ()), this, SLOT (redo ()));

	//
	// Настраиваем действия вырезания/копирования/вставки
	//
	m_cut_action->setEnabled (false);
	m_copy_action->setEnabled (false);

	connect (m_cut_action, SIGNAL(triggered ()), this, SLOT (cut()));
	connect (m_copy_action, SIGNAL(triggered ()), this, SLOT (copy()));
	connect (m_paste_action, SIGNAL(triggered ()), this, SLOT (paste()));
	connect (m_delete_action, SIGNAL(triggered ()), this, SLOT (clear ()));
	connect (m_select_all_action, SIGNAL(triggered ()), this, SLOT (selectAll ()));

	connect (this, SIGNAL (copyAvailable (bool)), m_cut_action, SLOT (setEnabled (bool)));
	connect (this, SIGNAL (copyAvailable (bool)), m_copy_action, SLOT (setEnabled (bool)));

	//
	// Ловим события об изменении буфера обмена
	//
	connect (QApplication::clipboard (), SIGNAL(dataChanged ()), this, SLOT(aboutClipboardDataChanged ()));

	//
	// Настраиваем вставку изображения и другие операции с ним
	//
	connect (m_image_action, SIGNAL (triggered ()), this, SLOT (aboutInsertImage ()));
	connect (m_resize_action, SIGNAL (triggered ()), this, SLOT (aboutResizeImage ()));
}

void
CTextEdit::enableTableActions (bool _enable)
{
	m_remove_row_action->setEnabled (_enable);
	m_remove_col_action->setEnabled (_enable);

	m_add_row_action_1->setEnabled (_enable);
	m_add_row_action_2->setEnabled (_enable);
	m_add_row_action_3->setEnabled (_enable);
	m_add_row_action_4->setEnabled (_enable);
	m_add_row_action_n->setEnabled (_enable);

	m_add_col_action_1->setEnabled (_enable);
	m_add_col_action_2->setEnabled (_enable);
	m_add_col_action_3->setEnabled (_enable);
	m_add_col_action_4->setEnabled (_enable);
	m_add_col_action_n->setEnabled (_enable);

	m_merge_adjacent_cells_action->setEnabled (_enable);
	m_split_cell_action->setEnabled (_enable);
	m_table_appreance_action->setEnabled (_enable);
	m_change_rows_height_action->setEnabled (_enable);
	m_change_cols_width_action->setEnabled (_enable);
}

void
CTextEdit::notify ( OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_event);
	Q_UNUSED (_source);
	Q_UNUSED (_origin);

	//
	// Если документ пуст, то блокируем delete и select_all
	//
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());
}

void
CTextEdit::aboutFilePrintPdf ()
{
	QString file_name = QFileDialog::getSaveFileName (this, tr ("Export PDF"), QString (),
													  OAF::CMimeDatabase::instance ().filterFromMime ("application/pdf"));
	if (!file_name.isEmpty ())
	{
		if (QFileInfo (file_name).suffix().isEmpty())
			file_name.append (".pdf");
		QPrinter printer (QPrinter::HighResolution);
		printer.setOutputFormat (QPrinter::PdfFormat);
		printer.setOutputFileName (file_name);
		document()->print (&printer);
	}
}

void
CTextEdit::aboutTextBold ()
{
	QTextCharFormat fmt;
	fmt.setFontWeight (m_bold_action->isChecked() ? QFont::Bold : QFont::Normal);
	mergeFormatOnWordOrSelection (fmt);
}

void
CTextEdit::aboutTextItalic ()
{
	QTextCharFormat fmt;
	fmt.setFontItalic (m_italic_action->isChecked());
	mergeFormatOnWordOrSelection (fmt);
}

void
CTextEdit::aboutTextUnderline ()
{
	QTextCharFormat fmt;
	fmt.setFontUnderline (m_underline_action->isChecked());
	mergeFormatOnWordOrSelection (fmt);
}

void
CTextEdit::aboutTextFamily (const QString& _f)
{
	QTextCharFormat fmt;
	fmt.setFontFamily (_f);
	mergeFormatOnWordOrSelection (fmt);
}

void
CTextEdit::aboutTextSize (const QString& _p)
{
	qreal pointSize = _p.toFloat();
	if (_p.toFloat() > 0)
	{
		QTextCharFormat fmt;
		fmt.setFontPointSize (pointSize);
		mergeFormatOnWordOrSelection (fmt);
	}
}

void
CTextEdit::aboutListStyle (int _style_index)
{
	QTextCursor cursor = textCursor ();

	if (_style_index != 0)
	{
		QTextListFormat::Style style = QTextListFormat::ListDisc;

		switch (_style_index)
		{
			case 1:
				style = QTextListFormat::ListDisc;
				break;
			case 2:
				style = QTextListFormat::ListCircle;
				break;
			case 3:
				style = QTextListFormat::ListSquare;
				break;
			case 4:
				style = QTextListFormat::ListDecimal;
				break;
			case 5:
				style = QTextListFormat::ListLowerAlpha;
				break;
			case 6:
				style = QTextListFormat::ListUpperAlpha;
				break;
			case 7:
				style = QTextListFormat::ListLowerRoman;
				break;
			case 8:
				style = QTextListFormat::ListUpperRoman;
				break;
			default:
				break;
		}

		cursor.beginEditBlock ();

		QTextBlockFormat block_fmt = cursor.blockFormat ();
		QTextListFormat list_fmt;
		if (cursor.currentList ())
		{
			list_fmt = cursor.currentList ()->format ();
		}
		else
		{
			list_fmt.setIndent(block_fmt.indent () + 1);
			block_fmt.setIndent (0);
			cursor.setBlockFormat (block_fmt);
		}
		list_fmt.setStyle (style);
		cursor.createList (list_fmt);
		cursor.endEditBlock ();
	}
	else
	{
		//
		// Если находимся на списке, то превращаем его обратно в обычный текст;
		// если текст уже обычный, то ничего и не делаем
		//
		QTextList* list = cursor.currentList();
		if (list)
		{
			QTextBlockFormat block_def_format;
			block_def_format.setIndent (0);

			//
			// Либо берем все выделенные элементы списка,
			// либо текущий - на котором стоит курсор
			//
			if (cursor.hasSelection())
			{
				int sel_start = cursor.selectionStart();
				int sel_end = cursor.selectionEnd();

				QTextBlock sel_start_block = document()->findBlock (sel_start);
				QTextBlock sel_end_block = document()->findBlock (sel_end);
				Q_ASSERT_X (sel_start_block.isValid(), Q_FUNC_INFO, "Invalid text block");
				Q_ASSERT_X (sel_end_block.isValid(), Q_FUNC_INFO, "Invalid text block");

				do
				{
					//
					// Удаляем очередной элемент из списка
					//
					int item_index = list->itemNumber (sel_start_block);
					if (item_index != -1)
						list->removeItem (item_index);

					//
					// Восстанавливаем отступ по умолчанию
					//
					cursor.mergeBlockFormat (block_def_format);

					sel_start_block = sel_start_block.next();
					cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor);
				}
				while (sel_start_block.isValid() && (sel_start_block != sel_end_block.next()));
			}
			else
			{
				//
				// Получаем текущий элемент списка
				//
				QTextBlock item_block = cursor.block();
				Q_ASSERT(item_block.isValid());
				int item_index = list->itemNumber (item_block);
				if (item_index != -1)
					list->removeItem (item_index);

				//
				// Восстанавливаем отступ по умолчанию
				//
				cursor.mergeBlockFormat (block_def_format);
			}
		}
	}
}

void
CTextEdit::aboutTextColorChanged ()
{
	QColor col = QColorDialog::getColor (textColor (), this);
	if (col.isValid ())
	{
		QTextCharFormat fmt;
		fmt.setForeground (col);
		mergeFormatOnWordOrSelection (fmt);
		colorChanged (col);
	}
}

void
CTextEdit::aboutTextAlign (QAction* _a)
{
	if (_a == m_align_left_action)
		setAlignment (Qt::AlignLeft | Qt::AlignAbsolute);
	else if (_a == m_align_center_action)
		setAlignment (Qt::AlignHCenter);
	else if (_a == m_align_right_action)
		setAlignment (Qt::AlignRight | Qt::AlignAbsolute);
	else if (_a == m_align_justify_action)
		setAlignment (Qt::AlignJustify);
}

void
CTextEdit::aboutIndentListLess ()
{
	QTextCursor cursor = textCursor ();
	QTextCursor workingCursor (cursor);
	workingCursor.setPosition (cursor.selectionStart ());
	workingCursor.beginEditBlock ();

	QTextCursor endCursor (cursor);
	endCursor.setPosition (cursor.selectionEnd ());

	bool moveSucceeded;
	do
	{
		QTextList* textList = workingCursor.currentList ();
		if (textList)
		{
			QTextListFormat listFormat = textList->format ();

			if (listFormat.indent() == 1)
			{
				if (! cursor.hasSelection ())
					aboutRemoveFromList (workingCursor);
			}
			else
			{
				listFormat.setIndent (listFormat.indent () - 1);

				QTextListFormat::Style listStyle;
				switch (listFormat.indent() % 3)
				{
					case 1:
						listStyle = QTextListFormat::ListDisc;
						break;
					case 2:
						listStyle = QTextListFormat::ListSquare;
						break;
					default:  // case 0:
						listStyle = QTextListFormat::ListCircle;
						break;
				}

				listFormat.setStyle (listStyle);
				workingCursor.createList (listFormat);
			}
		}

		moveSucceeded = workingCursor.movePosition (QTextCursor::NextBlock);
	}
	while (workingCursor.position() <= endCursor.block().position() && moveSucceeded);

	workingCursor.endEditBlock();
}

void
CTextEdit::aboutIndentListMore ()
{
	QTextCursor cursor = textCursor();
	QTextCursor workingCursor(cursor);
	workingCursor.setPosition(cursor.selectionStart());
	workingCursor.beginEditBlock();

	QTextCursor endCursor(cursor);
	endCursor.setPosition(cursor.selectionEnd());

	bool moveSucceeded;
	do
	{
		QTextList* textList = workingCursor.currentList ();
		QTextListFormat listFormat;

		if (textList)
		{
			listFormat = textList->format ();
			listFormat.setIndent (listFormat.indent () + 1);
		}
		else
			listFormat.setIndent (1);

		QTextListFormat::Style listStyle;
		switch (listFormat.indent () % 3)
		{
			case 1:
				listStyle = QTextListFormat::ListDisc;
				break;
			case 2:
				listStyle = QTextListFormat::ListSquare;
				break;
			default:  // case 0:
				listStyle = QTextListFormat::ListCircle;
				break;
		}

		listFormat.setStyle (listStyle);
		workingCursor.createList (listFormat);
		moveSucceeded = workingCursor.movePosition (QTextCursor::NextBlock);
	}
	while (workingCursor.position () <= endCursor.block ().position () && moveSucceeded);

	workingCursor.endEditBlock ();
}

void
CTextEdit::aboutRemoveFromList (QTextCursor cursor)
{
	if (cursor.isNull())
		cursor = textCursor ();

	QTextList* text_list = cursor.currentList();
	cursor.beginEditBlock ();
		QTextBlockFormat removeIndentFormat;
		removeIndentFormat.setIndent(0);
		text_list->remove (cursor.block());
		cursor.mergeBlockFormat (removeIndentFormat);
	cursor.endEditBlock ();
}

void
CTextEdit::aboutInsertTable ()
{
	QDialog dlg (this);
	Ui::CTableDialog insert_dlg;
	insert_dlg.setupUi (&dlg);
	dlg.setWindowTitle (tr ("Insert table"));

	if (dlg.exec() == QDialog::Accepted)
	{
		bool bOk = false;
		int rows_cnt = insert_dlg.row_count->text().toInt (&bOk);
		if (! bOk)
			return;
		int columns_cnt = insert_dlg.column_count->text().toInt (&bOk);
		if (! bOk)
			return;

		//
		// Делаем небольшой отступ между текстом в ячейке и границей (просто для красоты)
		//
		QTextCursor cursor = textCursor ();
		QTextTableFormat table_format;
		table_format.setCellPadding (5);
	//        table_format.setHeight (50);

		//
		// Растягиваем таблицу на ширину документа, с одинаковой шириной для каждого столбца
		//
		QVector <QTextLength> col_widths;
		for (int i = 0; i < columns_cnt; ++i)
			col_widths << QTextLength (QTextLength::PercentageLength, 100.0 / columns_cnt);
		table_format.setColumnWidthConstraints (col_widths);

		//
		// Вставляем таблицу с указанным числом строк и столбцов
		//
		cursor.insertTable (rows_cnt, columns_cnt, table_format);
	}
}

namespace
{
	//
	// Возвращает текущую ячейку в таблице (т.е. на которой стоит курсор)
	//
	QTextTableCell
	_currentTableCell (const QTextCursor& cursor, QTextTable** tbl)
	{
		(*tbl) = cursor.currentTable ();
		Q_ASSERT (tbl);

		QTextTableCell cell = (*tbl)->cellAt (cursor);
		Q_ASSERT (cell.isValid ());
		return cell;
	}

	//
	// Запрашивает число у пользователя с помощью диалога
	//
	int
	_askUserForNumber (QWidget* _parent, const QString& _title, const QString& _label, int _def_value = 1)
	{
		//
		// Запрашиваем конкретное число строк с помощью диалога
		//
		QDialog dlg (_parent);
		Ui::CInputDialog input_dlg;
		input_dlg.setupUi (&dlg);
		dlg.setWindowTitle (_title);
		input_dlg.input_label->setText (_label);

		if (dlg.exec() != QDialog::Accepted)
			return (-1);

		bool bOk = false;
		int res = input_dlg.data->text().toInt (&bOk);
		if (! bOk)
			res = _def_value;

		return res;
	}
}

void
CTextEdit::aboutRemoveRow ()
{
	QTextTable* tbl = NULL;
	int row_idx = _currentTableCell (textCursor (), &tbl).row ();

	//
	// Удаляем строку, на которой стоит курсор
	//
	tbl->removeRows (row_idx, 1);
}

void
CTextEdit::aboutRemoveCol ()
{
	QTextTable* tbl = NULL;
	int col_idx = _currentTableCell (textCursor (), &tbl).column ();

	//
	// Удаляем строку, на которой стоит курсор
	//
	tbl->removeColumns (col_idx, 1);
}

void
CTextEdit::aboutAddRow (int _count)
{
	if (_count > 4)
	{
		_count = _askUserForNumber (this, tr ("Enter required number of rows"), tr ("Row count: "));
		if (_count == -1)
			return;
	}

	//
	// Вставляем указанное число строк перед той, на которой стоит курсор
	//
	QTextTable* tbl = NULL;
	int row_idx = _currentTableCell (textCursor (), &tbl).row ();
	tbl->insertRows (row_idx, _count);
}

void
CTextEdit::aboutAddCol (int _count)
{
	if (_count > 4)
	{
		_count = _askUserForNumber (this, tr ("Enter required number of columns"), tr ("Column count: "));
		if (_count == -1)
			return;
	}

	//
	// Вставляем указанное число столбцов перед тем, на котором стоит курсор
	//
	QTextTable* tbl = NULL;
	int col_idx = _currentTableCell (textCursor (), &tbl).column ();
	tbl->insertColumns (col_idx, _count);
}

void
CTextEdit::aboutMergeCells ()
{
	//
	// Объединяем текущую ячейку и ее соседа справа
	//
	QTextTable* tbl = NULL;
	int row_idx = _currentTableCell (textCursor (), &tbl).row ();
	int col_idx = _currentTableCell (textCursor (), &tbl).column ();
	tbl->mergeCells (row_idx, col_idx, 1, 2);
}

void
CTextEdit::aboutSplitCells ()
{
	//
	// Разбиваем на две ячейку, на которой стоит курсор;
	// NOTE: разбивать можно только уже объединенные ячейки
	//
	QTextTable* tbl = NULL;
	int row_idx = _currentTableCell (textCursor (), &tbl).row ();
	int col_idx = _currentTableCell (textCursor (), &tbl).column ();
	tbl->splitCell (row_idx, col_idx, 1, 1);
}

void
CTextEdit::aboutTableAppreance ()
{
	QTextTable* tbl = textCursor ().currentTable ();
	Q_ASSERT (tbl);

	//
	// Показываем диалог с настройками внешнего вида таблицы (границы и т.п.)
	//
	QDialog dlg (this);
	Ui::CTableAppreance appr_dlg;
	appr_dlg.setupUi (&dlg);
	dlg.setWindowTitle (tr ("Table appreance"));
	appr_dlg.border_width->setValue (tbl->format ().border ());
	appr_dlg.border_style->setCurrentIndex (tbl->format ().borderStyle ());

	if (dlg.exec() != QDialog::Accepted)
		return;

	//
	// NOTE: стили границы счастливым образом идут подряд в enum'е и он нуля
	//
	qreal border_width = appr_dlg.border_width->value ();
	int bs = appr_dlg.border_style->currentIndex ();
	Q_ASSERT ((bs >= 0) && (bs <= QTextFrameFormat::BorderStyle_Outset));
	QTextFrameFormat::BorderStyle border_style = QTextFrameFormat::BorderStyle (bs);

	QTextTableFormat fmt = tbl->format ();
	fmt.setBorder (border_width);
	fmt.setBorderStyle (border_style);

	tbl->setFormat (fmt);
}

void
CTextEdit::aboutRowsHeight ()
{
	// TODO: стандартными средствами QTextTable, QTextTableFormat невозможно изменить
	// высоту строк
}

void
CTextEdit::aboutColsWidth ()
{
	//
	// Показываем диалог для настройки ширины столбцов
	// TODO: пока одной для всех, а не индивидуально, т.к. сложноват интерфейс диалога получится
	//
	int new_col_width = _askUserForNumber (this, tr ("Enter required width of columns"),
										   tr ("Width: "));
	if (new_col_width == -1)
		return;

	QTextTable* tbl = textCursor ().currentTable ();
	Q_ASSERT (tbl);

	QTextTableFormat fmt = tbl->format ();
	QVector <QTextLength> col_widths;
	for (int i = 0; i < tbl->columns (); ++i)
		col_widths << QTextLength (QTextLength::FixedLength, new_col_width);
	fmt.setColumnWidthConstraints (col_widths);

	tbl->setFormat (fmt);
}

bool
CTextEdit::canInsertFromMimeData (const QMimeData* _source) const
{
	return _source->hasImage () || _source->hasUrls () || _source->hasText () || _source->hasHtml ();
}

void
CTextEdit::insertFromMimeData (const QMimeData* source)
{
	if (source->hasImage())
	{
		dropImage (qvariant_cast<QImage>(source->imageData()));
	}
	else if (source->hasUrls())
	{
		foreach (QUrl url, source->urls())
		{
			QFileInfo info (OAF::toLocalFile(url));
			if (QImageReader::supportedImageFormats ().contains (info.suffix ().toLower ().toLatin1 ()))
				dropImage (QImage (info.filePath ()));
			else
				dropTextFile (url);
		}
	}
	else
		QTextEdit::insertFromMimeData (source);
}

//
// Переопределяем, чтобы показать custom-меню для встроенной в текст таблицы
//
// TODO: вообще-то логичнее работать с выделенными ячейками/строками/столбцами,
// да только нужная для этого функция работать не хочет. Гугл на эту тему молчит, StackOverflow - тоже.
//int startRow, startCol, numRows, numCols;
//cursor.selectedTableCells(&startRow, &numRows, &startCol, &numCols);
void
CTextEdit::contextMenuEvent (QContextMenuEvent* event)
{
	QTextCursor cursor = cursorForPosition (event->pos ());
	Q_ASSERT (! cursor.isNull ());

	//
	// Проверяем, не кликнул ли пользователь внутри таблицы;
	// если да, то включаем соответствующие пункты меню
	//
	QTextTable* tbl = cursor.currentTable ();
	enableTableActions (tbl != NULL);

	//
	// Показываем модифицированное меню и ждем,
	// пока пользователь выберет какой-то его пункт (т.е. синхронно)
	//
	m_context_menu->exec (event->globalPos ());
}

void
CTextEdit::dropImage (const QImage& image)
{
	// Максимальный размер в байтах изображения, которое можно вставить в редактор
	// в виде raw-data без получения тормозов
	// NOTE: это размер уже распакованных данных в памяти!
	// за счет сжатия, включенного например для png,
	// размер такого файла на диске будет заметно меньше - вплоть до двух раз и выше
	const int MAX_EMBED_SIZE = 300*1024;    // 300 КiB

	if (! image.isNull ())
	{
		//
		// Предлагаем пользователю вставить изображение-переросток в качестве ссылки на файл,
		// для чего картинку придется предварительно сохранить на диск
		//
		if (image.byteCount() > MAX_EMBED_SIZE)
		{
			const QString msg = tr("The image in Clipboard have too large size to be embedded in document directly.\n"
								   "Do you want to save this image as file and embed it as link?");
			if (QMessageBox::question (this, tr("Warning"), msg, QMessageBox::Save, QMessageBox::Cancel) == QMessageBox::Save)
			{
				QString file = QFileDialog::getSaveFileName (this, tr("Select an image to insert"), ".",
															 m_all_images_filter);
				if (! file.isEmpty())
				{
					if (image.save (file))
						textCursor ().insertImage (OAF::fromLocalFile (file).toString ());
					else
						QMessageBox::warning (this, tr("Warning"),
											  tr("Could not save image in specified location \"") + file + "\"");
				}
			}
		}
		else
		{
			//
			// Создаём описатель raw данных
			//
			if (URef<OAF::IIODevice> d = OAF::createFromName<OAF::IIODevice> ("raw:"))
			{
				//
				// Открываем его на запись
				//
				if (d->device ()->open (QIODevice::WriteOnly))
				{
					//
					// Сохраняем в заданном описателе в формате PNG (данный формат сохраняет прозрачность)
					//
					if (image.save (d->device (), "PNG"))
					{
						//
						// Закрываем, чтобы гарантировать, что все данные записаны
						//
						d->device ()->close ();

						//
						// Сохраняем изображение в документе
						//
						textCursor ().insertImage (d->getInfo (OAF::IIODevice::PATH).toString ());
					}
				}
			}
		}
	}
}

void
CTextEdit::dropTextFile (const QUrl& url)
{
	QFile file (OAF::toLocalFile (url));
	if (file.open (QIODevice::ReadOnly | QIODevice::Text))
		textCursor ().insertText (file.readAll ());
}

void
CTextEdit::aboutInsertImage ()
{
	QString file = QFileDialog::getOpenFileName (this, tr("Select an image to insert"), ".",
												 m_all_images_filter);
	if (! file.isEmpty())
		textCursor ().insertImage (OAF::fromLocalFile (file).toString ());
}

void
CTextEdit::aboutResizeImage ()
{
	QTextCursor cursor = textCursor ();
	if (cursor.hasSelection())
	{
		int sel_start = cursor.selectionStart();
		int sel_end = cursor.selectionEnd();

		QTextBlock sel_start_block = document()->findBlock (sel_start);
		QTextBlock sel_end_block = document()->findBlock (sel_end);
		Q_ASSERT_X (sel_start_block.isValid(), Q_FUNC_INFO, "Invalid text block");
		Q_ASSERT_X (sel_end_block.isValid(), Q_FUNC_INFO, "Invalid text block");

		//
		// Сначала проверяем, что в выделенном блоке вообще есть изображения и
		// выясняем размеры первого из выделенных изображений. Эти размеры будут
		// размерами по умолчанию для диалога
		//
		int width  = 0;
		int height = 0;
		for (QTextBlock block = sel_start_block; block.isValid () && (block != sel_end_block.next ()); block = block.next ())
		{
			for (QTextBlock::iterator it = block.begin (); !it.atEnd (); ++it)
			{
				QTextFragment fragment = it.fragment ();
				if (fragment.isValid ())
				{
					if (fragment.charFormat ().isImageFormat ())
					{
						int img_pos = fragment.position();
						if ((img_pos >= sel_start) && (img_pos <= sel_end))
						{
							QTextImageFormat img_format = fragment.charFormat ().toImageFormat ();

							width  = img_format.width ();
							height = img_format.height ();

							//
							// Если ширина и высота явно не заданы, то выбираем их из самой картинки
							//
							if ((width == 0) || (height == 0))
							{
								//
								// Открываем устройство для чтения изображения
								//
								if (URef<OAF::IIODevice> d = OAF::createFromName<OAF::IIODevice> (img_format.name ()))
								{
									//
									// Если это устройство успешно открыто для чтения
									//
									if (d->device ()->open (QIODevice::ReadOnly))
									{
										//
										// Загружаем изображение
										//
										QImage img = QImageReader (d->device ()).read ();
										if (!img.isNull ())
										{
											if (width == 0)
												width = img.width ();
											if (height == 0)
												height = img.height ();
										}
									}
								}
							}

							break;
						}
					}
				}
			}

			//
			// Если изображение найдено, то прерываем цикл
			//
			if ((width > 0) && (height > 0))
				break;
		}

		//
		// Если изображений нет, то выдаём предупреждение и выходим
		//
		if ((width < 1) || (height < 1))
		{
			QMessageBox::warning (this, tr("Warning"), tr("No images found!"));
			return;
		}

		//
		// Запрашиваем у пользователя новые размеры изображения с помощью диалога
		//
		QDialog dlg (this);
		Ui::CResizeDialog resize_dlg;
		resize_dlg.setupUi (&dlg);
		dlg.setWindowTitle (tr("Resize image"));
		resize_dlg.width->setText (QString::number (width));
		resize_dlg.height->setText (QString::number (height));

		if (dlg.exec() == QDialog::Accepted)
		{
			bool ok = false;

			width  = resize_dlg.width ->text().toDouble (&ok);
			height = resize_dlg.height->text().toDouble (&ok);

			//
			// То, что пользователь ввёл какой-то бред, не повод абортить программу на ASSERT'е
			//
			if (!ok)
			{
				QMessageBox::warning (this, tr("Warning"), tr("Can't set specified width or height!"));
				return;
			}

			if ((width < 1) || (height < 1))
			{
				QMessageBox::warning (this, tr("Warning"), tr("Minimal allowed size of image is 1x1 pixels!"));
				return;
			}

			//
			// Пробегаем по всем выделенным блокам текста и ищем в них изображения
			//
			do
			{
				for (QTextBlock::iterator it = sel_start_block.begin(); !it.atEnd(); ++it)
				{
					QTextFragment fragment = it.fragment();
					if (fragment.isValid ())
					{
						if (fragment.charFormat ().isImageFormat ())
						{
							QTextImageFormat img_format = fragment.charFormat ().toImageFormat ();

							//
							// Устанавливаем новый размер изображения, указанный пользователем
							//
							if (img_format.isValid () && ((width != img_format.width ()) || (height != img_format.height ())))
							{
								img_format.setWidth  (width);
								img_format.setHeight (height);

								int img_pos = fragment.position();
								if ((img_pos >= sel_start) && (img_pos <= sel_end))
								{
									cursor.setPosition (img_pos);
									cursor.setPosition (img_pos + fragment.length(),
														QTextCursor::KeepAnchor);
									cursor.setCharFormat (img_format);
								}
							}
						}
					}
				}

				sel_start_block = sel_start_block.next ();
				cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor);
			}
			while (sel_start_block.isValid () && (sel_start_block != sel_end_block.next ()));
		}
	}
}

void
CTextEdit::aboutCurrentCharFormatChanged (const QTextCharFormat& _format)
{
	fontChanged (_format.font ());
	colorChanged (_format.foreground().color ());
}

void
CTextEdit::aboutCursorPositionChanged ()
{
	//
	// Отображаем выравнивание в новой позиции курсора
	//
	alignmentChanged (alignment ());

	//
	// Отображаем текущий стиль списка в новой позиции (если есть)
	//
	QTextCursor cursor = textCursor ();
	QTextListFormat::Style list_style = QTextListFormat::ListStyleUndefined;
	if (cursor.currentList ())
	{
		QTextListFormat list_fmt = cursor.currentList ()->format ();
		list_style = list_fmt.style();
	}
	listStyleChanged (list_style);

	//
	// Если курсор встал внутри таблицы, то активируем соответствующие пункты меню
	//
	enableTableActions (cursor.currentTable () != NULL);
}

void
CTextEdit::aboutClipboardDataChanged ()
{
	m_paste_action->setEnabled (clipboardHasData ());
}

CTextEdit::CTextEdit (CFactory* _factory) : CUnknown (text_edit_cid), m_factory (_factory), m_uic (NULL),
	m_font_list_widget (NULL), m_size_list_widget (NULL), m_list_style_widget (NULL)
{
	setObjectName ("ui:textedit");

	//
	// Создаем и сохраняем в переменной фильтр для выбора изображений в QFileDialog
	//
	QStringList img_mimes;
	img_mimes << "image/bmp";
	img_mimes << "image/tiff";
	img_mimes << "image/jpeg";
	img_mimes << "image/x-tga";
	img_mimes << "image/png";
	img_mimes << "video/x-mng";
	img_mimes << "image/x-xbitmap";
	img_mimes << "image/x-xpixmap";
	img_mimes << "image/x-portable-bitmap";
	img_mimes << "image/x-portable-graymap";
	img_mimes << "image/x-portable-pixmap";
	m_all_images_filter = OAF::CMimeDatabase::instance ().filterFromMime (tr ("All Image Files"), img_mimes);

	//
	// Создаем стандартные объекты действий для WordPad-like текстового редактора
	//
	createFileActions ();
	createEditActions ();
	createTextActions ();
	createListActions ();
	createTableActions ();
	createFontComboAction ();
	createSizeComboAction ();
	createListComboAction ();
	createContextMenu ();

	//
	// Реагируем на изменение формата выделенного текста
	//
	connect (this, SIGNAL (currentCharFormatChanged (const QTextCharFormat&)),
				  this, SLOT (aboutCurrentCharFormatChanged (const QTextCharFormat&)));
	connect (this, SIGNAL (cursorPositionChanged ()),
				  this, SLOT (aboutCursorPositionChanged ()));
	//
	// Инициализируем шрифт, цвет и выравнивание по умолчанию
	//
	fontChanged (font ());
	colorChanged (textColor ());
	alignmentChanged (alignment ());

	//
	// Связываем команды редактирования с документом
	//
	setupActions ();

	//
	// Создаём пустой документ заданного класса для редактирования
	//
	m_document = OAF::createFromName<QTextDocument> ("cid:OAF/TXT/CTextDocument:1.0");
	setDocument (m_document);
	OAF::subscribe (m_document.queryInterface<OAF::IInterface> (), this);
}

CTextEdit::~CTextEdit ()
{
	setDocument (NULL);
	OAF::unsubscribe (m_document.queryInterface<OAF::IInterface> (), this);

	disconnect (QApplication::clipboard (), SIGNAL(dataChanged ()), this, SLOT(aboutClipboardDataChanged ()));
}

OAF::URef<OAF::IUnknown>
CTextEdit::setExtendedObject (OAF::IUnknown* _o)
{
	//
	// Устанавливаем новый документ для редактирования
	//
	if (OAF::URef<QTextDocument> document = OAF::queryInterface<QTextDocument> (_o))
	{
		OAF::unsubscribe (m_document.queryInterface<OAF::IInterface> (), this);
		setDocument (NULL);
		m_document = document;
		setDocument (m_document);
		OAF::subscribe (m_document.queryInterface<OAF::IInterface> (), this);
	}

	//
	// Если документ пуст, то блокируем delete и select_all
	//
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());

	return this;
}

void
CTextEdit::setUILabel (const QString& _bag, const QString& _label)
{
	if (_bag == "ui:main")
		m_main_label = _label;
}

void
CTextEdit::setUIContainer (OAF::IUIContainer* _uic)
{
	//
	// Добавляем в интерфейс объект редактора rich-текста
	//

	static const char uidef[] =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:main\">"
				"<item id=\"ui:textedit\" label=\"%1\"/>"
			"</bag>"
		"</uidef>";

	if (_uic)
	{
		m_uic = _uic;
		m_id  = m_uic->addUI (QString (uidef).arg (m_main_label.isEmpty () ? tr ("Text Edit") : m_main_label), this);
	}
	else
	{
		m_id  = m_uic->removeUI (m_id);
		m_uic = _uic;
	}
}

OAF::IUIContainer*
CTextEdit::getUIContainer ()
{
	return m_uic;
}

//
// Активация/деактивация динамического графического интерфейса пользователя
// NOTE: не забываем выслать сигнал об изменении содержимого редактора при активации!
//
void
CTextEdit::activate (bool _activate)
{
	// TODO: если понадобится печать документа в PDF - раскомментировать.
//	static const QString export_uidef =
//			"<folder id=\"ui:export\"               label=\"%1\" >"
//			"   <item id=\"ui:textedit:exportpdf\"              />"
//			"</folder                                            >";

	static const QString menubar_edit_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<item id=\"ui:textedit:undo\"/>"
				"<item id=\"ui:textedit:redo\"/>"
				"<separator/>"
				"<item id=\"ui:textedit:cut\"/>"
				"<item id=\"ui:textedit:copy\"/>"
				"<item id=\"ui:textedit:paste\"/>"
				"<item id=\"ui:textedit:delete\"/>"
				"<separator/>"
				"<item id=\"ui:textedit:select_all\"/>"
			"</folder>";

	static const QString toolbar_edit_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<item id=\"ui:textedit:undo\"/>"
				"<item id=\"ui:textedit:redo\"/>"
				"<separator/>"
				"<item id=\"ui:textedit:cut\"/>"
				"<item id=\"ui:textedit:copy\"/>"
				"<item id=\"ui:textedit:paste\"/>"
			"</folder>";

	static const QString menubar_formatting_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<separator/>"
				"<item id=\"ui:textedit:bold\"/>"
				"<item id=\"ui:textedit:italic\"/>"
				"<item id=\"ui:textedit:underline\"/>"
				"<separator/>"
				"<item id=\"ui:textedit:color\" label=\"%2\"/>"
			"</folder>";

	static const QString toolbar_formatting_uidef =
			"<folder id=\"ui:formatting\" label=\"%1\">"
				"<item id=\"ui:textedit:liststyle\"/>"
				"<item id=\"ui:textedit:fontlist\"/>"
				"<item id=\"ui:textedit:fontsize\"/>"
			"</folder>";

	static const QString menubar_nestedlist_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<separator/>"
				"<item id=\"ui:textedit:increase_indent\"/>"
				"<item id=\"ui:textedit:decrease_indent\"/>"
			"</folder>";

	static const QString menubar_alignment_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<separator/>"
				"<item id=\"ui:textedit:alignment:left\"/>"
				"<item id=\"ui:textedit:alignment:center\"/>"
				"<item id=\"ui:textedit:alignment:right\"/>"
				"<item id=\"ui:textedit:alignment:justify\"/>"
			"</folder>";

	static const QString toolbar_alignment_uidef =
			"<folder id=\"ui:textedit:alignment\" label=\"%1\">"
				"<item id=\"ui:textedit:alignment:left\"/>"
				"<item id=\"ui:textedit:alignment:center\"/>"
				"<item id=\"ui:textedit:alignment:right\"/>"
				"<item id=\"ui:textedit:alignment:justify\"/>"
			"</folder>";

	static const QString menubar_image_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<separator/>"
				"<item id=\"ui:textedit:insert_image\"/>"
				"<item id=\"ui:textedit:resize_image\"/>"
			"</folder>";

	static const QString menubar_table_uidef =
			"<folder id=\"ui:table\" label=\"%1\">"
				"<separator/>"
				"<item id=\"ui:textedit:insert_table\"/>"
				"<item id=\"ui:textedit:remove_row\" />"
				"<item id=\"ui:textedit:remove_col\" />"
				"<folder id=\"ui:add_row_menu\" label=\"%2\">"
					"<item id=\"ui:textedit:add_1_row\" />"
					"<item id=\"ui:textedit:add_2_row\" />"
					"<item id=\"ui:textedit:add_3_row\" />"
					"<item id=\"ui:textedit:add_4_row\" />"
					"<item id=\"ui:textedit:add_n_row\" />"
				"</folder>"
				"<folder id=\"ui:add_col_menu\" label=\"%3\">"
					"<item id=\"ui:textedit:add_1_col\" />"
					"<item id=\"ui:textedit:add_2_col\" />"
					"<item id=\"ui:textedit:add_3_col\" />"
					"<item id=\"ui:textedit:add_4_col\" />"
					"<item id=\"ui:textedit:add_n_col\" />"
				"</folder>"
				"<item id=\"ui:textedit:merge_cells\" />"
				"<item id=\"ui:textedit:split_cells\" />"
				"<item id=\"ui:textedit:table_appreance\" />"
//				"<item id=\"ui:textedit:rows_height\" />"	// Не реализовано
				"<item id=\"ui:textedit:cols_width\" />"
			"</folder>";

	static const QString uidef =
			"<?xml version=\"1.0\"?>"
			"<uidef>"
				"<bag id=\"ui:menubar\">"
					+ menubar_edit_uidef.arg (tr ("Edit"))
					+ menubar_formatting_uidef.arg (tr ("Edit")).arg (tr ("Font color"))
					+ menubar_nestedlist_uidef.arg (tr ("Indentation"))
					+ menubar_alignment_uidef.arg (tr ("Edit"))
					+ menubar_image_uidef.arg (tr ("Edit"))
					+ menubar_table_uidef.arg (tr ("Table"),
											   tr ("Insert rows before this"),
											   tr ("Insert columns before this")) +
				"</bag>"
				"<bag id=\"ui:toolbar\">"
					+ toolbar_edit_uidef.arg (tr ("Edit"))
					+ toolbar_formatting_uidef.arg (tr ("Format"))
					+ toolbar_alignment_uidef.arg (tr ("Alignment")) +
				"</bag>"
			"</uidef>";

	if (! _activate)
		m_id_active = m_uic->removeUI (m_id_active);

	if (_activate)
		m_id_active = m_uic->addUI (uidef, this);

	emit aboutCurrentCharFormatChanged (currentCharFormat ());
}

QObject*
CTextEdit::getUIItem (const QString& _id)
{
	if (_id == objectName ())
		return this;
	else if (_id == m_font_list->objectName ())
		return m_font_list;
	else if (_id == m_size_list->objectName ())
		return m_size_list;
	else if (_id == m_list_style->objectName ())
		return m_list_style;
	else if (_id == m_bold_action->objectName ())
		return m_bold_action;
	else if (_id == m_italic_action->objectName ())
		return m_italic_action;
	else if (_id == m_underline_action->objectName ())
		return m_underline_action;
	else if (_id == m_color_action->objectName ())
		return m_color_action;
	else if (_id == m_alignment->objectName ())
		return m_alignment;
	else if (_id == m_align_left_action->objectName ())
		return m_align_left_action;
	else if (_id == m_align_center_action->objectName ())
		return m_align_center_action;
	else if (_id == m_align_right_action->objectName ())
		return m_align_right_action;
	else if (_id == m_align_justify_action->objectName ())
		return m_align_justify_action;
	else if (_id == m_undo_action->objectName ())
		return m_undo_action;
	else if (_id == m_redo_action->objectName ())
		return m_redo_action;
	else if (_id == m_cut_action->objectName ())
		return m_cut_action;
	else if (_id == m_copy_action->objectName ())
		return m_copy_action;
	else if (_id == m_paste_action->objectName ())
		return m_paste_action;
	else if (_id == m_delete_action->objectName ())
		return m_delete_action;
	else if (_id == m_select_all_action->objectName ())
		return m_select_all_action;
	else if (_id == m_image_action->objectName ())
		return m_image_action;
	else if (_id == m_resize_action->objectName ())
		return m_resize_action;
	else if (_id == m_export_pdf_action->objectName ())
		return m_export_pdf_action;
	else if (_id == m_inc_indent_action->objectName ())
		return m_inc_indent_action;
	else if (_id == m_dec_indent_action->objectName ())
		return m_dec_indent_action;
	else if (_id == m_insert_table_action->objectName ())
		return m_insert_table_action;
	else if (_id == m_remove_row_action->objectName ())
		return m_remove_row_action;
	else if (_id == m_remove_col_action->objectName ())
		return m_remove_col_action;

	else if (_id == m_add_row_action_1->objectName ())
		return m_add_row_action_1;
	else if (_id == m_add_row_action_2->objectName ())
		return m_add_row_action_2;
	else if (_id == m_add_row_action_3->objectName ())
		return m_add_row_action_3;
	else if (_id == m_add_row_action_4->objectName ())
		return m_add_row_action_4;
	else if (_id == m_add_row_action_n->objectName ())
		return m_add_row_action_n;

	else if (_id == m_add_col_action_1->objectName ())
		return m_add_col_action_1;
	else if (_id == m_add_col_action_2->objectName ())
		return m_add_col_action_2;
	else if (_id == m_add_col_action_3->objectName ())
		return m_add_col_action_3;
	else if (_id == m_add_col_action_4->objectName ())
		return m_add_col_action_4;
	else if (_id == m_add_col_action_n->objectName ())
		return m_add_col_action_n;

	else if (_id == m_merge_adjacent_cells_action->objectName ())
		return m_merge_adjacent_cells_action;
	else if (_id == m_split_cell_action->objectName ())
		return m_split_cell_action;
	else if (_id == m_table_appreance_action->objectName ())
		return m_table_appreance_action;
	else if (_id == m_change_rows_height_action->objectName ())
		return m_change_rows_height_action;
	else if (_id == m_change_cols_width_action->objectName ())
		return m_change_cols_width_action;

	return NULL;
}
