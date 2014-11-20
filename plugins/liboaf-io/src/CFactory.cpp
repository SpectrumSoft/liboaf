/**
 * @file
 * @brief Реализация фабрики объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CDeviceFile.h"
#include "CDeviceBuffer.h"
#include "CDeviceGIT.h"
#include "CMonikerIO.h"
#include "CFactory.h"

using namespace OAF::IO;

CFactory* OAF::IO::CFactory::m_instance = NULL;

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
		return new CMonikerIO (this);
	if (_cid == device_file_cid)
		return new CDeviceFile (this);
	if (_cid == device_buffer_cid)
		return new CDeviceBuffer (this);
	if (_cid == device_git_cid)
		return new CDeviceGIT (this);

	Q_ASSERT_X (0, Q_FUNC_INFO, "No object with specified CID was found");
	return NULL;
}
