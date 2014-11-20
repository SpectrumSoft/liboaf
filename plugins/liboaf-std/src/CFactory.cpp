/**
 * @file
 * @brief Реализация фабрики объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CMonikerSTD.h"
#include "CFactory.h"

using namespace OAF::STD;

CFactory* OAF::STD::CFactory::m_instance = NULL;

CFactory::CFactory () : CUnknown (factory_cid)
{}

CFactory::~CFactory ()
{
	m_instance = NULL;
}

OAF::IUnknown*
CFactory::instance ()
{
	if (!m_instance)
		m_instance = new CFactory ();
	return m_instance;
}

OAF::URef<OAF::IUnknown>
CFactory::createObject (const QString& _cid)
{
	if (_cid == moniker_cid)
		return new CMonikerSTD (this);
	return NULL;
}
