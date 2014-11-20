/**
 * @file
 * @brief Реализация фабрики объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CTextDocument.h"
#include "CTextEdit.h"
#include "CFactory.h"

using namespace OAF::TXT;

CFactory* OAF::TXT::CFactory::m_instance = NULL;

CFactory::CFactory () : CUnknown (factory_cid)
{}

CFactory::~CFactory ()
{
	m_instance = NULL;
}

OAF::URef<OAF::IUnknown>
CFactory::createObject (const QString& _cid)
{
	if (_cid == text_document_cid)
		return new CTextDocument (this);
	if (_cid == text_edit_cid)
		return new CTextEdit (this);
	return NULL;
}

void
CFactory::add (const QString& _name, OAF::IUnknown* _file)
{
	m_opened_files.insert (_name, _file);
}

void
CFactory::remove (const QString& _name)
{
	m_opened_files.remove (_name);
}

OAF::IUnknown*
CFactory::get (const QString& _name)
{
	return m_opened_files.value (_name, NULL);
}

OAF::IUnknown*
CFactory::instance ()
{
	if (!m_instance)
		m_instance = new CFactory ();
	return m_instance;
}
