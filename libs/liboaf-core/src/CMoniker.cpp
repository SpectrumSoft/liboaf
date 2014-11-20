/**
 * @file
 * @brief Базовая реализация моникера
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CMoniker.h>

OAF::CMoniker::CMoniker ()
{}

OAF::CMoniker::CMoniker (const CMoniker& _moniker) : OAF::IInterface (_moniker), OAF::IMoniker (_moniker),
	m_left (_moniker.m_left), m_prefix (_moniker.m_prefix), m_suffix (_moniker.m_suffix)
{}

OAF::CMoniker::~CMoniker ()
{}

void
OAF::CMoniker::set (OAF::IMoniker* _left, const QString& _prefix, const QString& _suffix)
{
	m_left   = _left;
	m_prefix = _prefix;
	m_suffix = _suffix;
}

OAF::IMoniker*
OAF::CMoniker::left ()
{
	return m_left;
}

const QString&
OAF::CMoniker::prefix ()
{
	return m_prefix;
}

const QString&
OAF::CMoniker::suffix ()
{
	return m_suffix;
}
