/**
 * @file
 * @brief Реализация помощника, для поиска по тексту в представлении
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/ViewFindHelper.h>

using namespace OAF::Helpers;

ViewFindHelper::ViewFindHelper (QAbstractItemView* _view, QObject* _parent) :
	QObject (_parent), m_view (_view), m_matched_node (-1), m_column_idx (0)
{
	Q_ASSERT (_view);
}

void
ViewFindHelper::findMatched (const QString &_pattern)
{
	//
	// Задаём шаблон поиска
	//
	m_pattern = _pattern;

	//
	// Перестраиваем список подходящих по шаблон поиска узлов
	//
	findMatched ();

	//
	// Если узлы найдены, то переходим на первый из них
	//
	if (m_matched_node >= 0)
		m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
}

void
ViewFindHelper::findMatched ()
{
	m_matched_nodes.clear ();
	m_matched_node = -1;

	if (!m_pattern.isEmpty ())
	{
		m_matched_nodes = m_view->model ()->match (
					m_view->model ()->index (0, m_column_idx),
					Qt::DisplayRole,
					QVariant (m_pattern),
					-1,
					Qt::MatchRecursive|Qt::MatchExactly|
					Qt::MatchFixedString|Qt::MatchWrap|
					Qt::MatchStartsWith
					);

		if (!m_matched_nodes.isEmpty ())
			m_matched_node = 0;
	}
}

void
ViewFindHelper::findPrev ()
{
	if (m_matched_node >= 0)
	{
		if (m_matched_node > 0)
			--m_matched_node;
		else
			m_matched_node = m_matched_nodes.count () - 1;

		m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
	}
}

void
ViewFindHelper::findNext ()
{
	if (m_matched_node >= 0)
	{
		if (m_matched_node < (m_matched_nodes.count () - 1))
			++m_matched_node;
		else
			m_matched_node = 0;

		m_view->setCurrentIndex (m_matched_nodes.at (m_matched_node));
	}
}

void
ViewFindHelper::setSearchColumn (int _idx)
{
	if ((_idx >= 0) && (_idx < m_view->model ()->columnCount ()))
		m_column_idx = _idx;
	else
		Q_ASSERT (0);
}
