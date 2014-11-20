/**
 * @file
 * @brief Интерфейс I/O компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CFactory.h"

using namespace OAF::IO;

extern "C"
{
	__declspec(dllexport) OAF::IUnknown*
	createObject (const QString& /*_cid*/)
	{
		return CFactory::instance ();
	}

	__declspec(dllexport) bool
	dllCanUnloadNow (void)
	{
		return (CCommon::objects () == 0);
	}

	__declspec(dllexport) const char*
	dllVersion (void)
	{
		return BUILD_VER;
	}
}
