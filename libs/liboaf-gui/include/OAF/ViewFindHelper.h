/**
 * @file
 * @brief Интерфейс помощника, для поиска по тексту в представлении
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __VIEWFINDHELPER_H
#define __VIEWFINDHELPER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	namespace Helpers
	{
		/**
		 * @brief Класс для организации поиска по представлению
		 */
		class OAFGUI_EXPORT ViewFindHelper : public QObject
		{
			Q_OBJECT

			/**
			 * @brief Представление, в котором выполняется поиск
			 */
			QAbstractItemView* m_view;

			/**
			 * @brief Строка поиска
			 */
			QString m_pattern;

			/**
			 * @brief Текущее множество элементов, подходящих под строку поиска
			 */
			QModelIndexList m_matched_nodes;

			/**
			 * @brief Текущий выбранный узел из найденных
			 */
			int m_matched_node;

			/**
			 * @brief Столбец, по которому будет осуществляться поиск; по умолчанию - первый
			 */
			int m_column_idx;

        public slots:
			/**
			 * @brief Найти совпадения
			 */
			void findMatched (const QString& _pattern);

			/**
			 * @brief Найти совпадения без перехода на первое найденное по заданному шаблону
			 */
			void findMatched ();

			/**
			 * @brief Найти предыдущее совпадение
			 */
			void findPrev ();

			/**
			 * @brief Найти следующее совпадение
			 */
			void findNext ();

		public:
			ViewFindHelper (QAbstractItemView* _view, QObject* _parent = NULL);

			/**
			 * @brief Возвращает представление, в котором выполняется поиск
			 */
			QAbstractItemView* view ();

			/**
			 * @brief Устанавливает индекс столбца, по которому будет осуществляться поиск
			 */
			void setSearchColumn (int _idx);
		};
	}
}

#endif /* __VIEWFINDHELPER_H */
