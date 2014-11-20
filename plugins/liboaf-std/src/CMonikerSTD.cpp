/**
 * @file
 * @brief Реализация стандартного моникера
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/IObjectSource.h>
#include <idl/IGenericFactory.h>

#include <OAF/OAF.h>

#include "CMonikerSTD.h"

using namespace OAF::STD;

CMonikerSTD::CMonikerSTD (CFactory* _factory) : CUnknown (moniker_cid), m_factory (_factory)
{}

CMonikerSTD::~CMonikerSTD ()
{}

OAF::URef<OAF::IUnknown>
CMonikerSTD::resolve (const QString& _interface)
{
	//
	// Резольвить заданный интерфейс не нужно, это сделает либо createFromName, либо
	// сам пользователь
	//
	(void)_interface;

	//
	// Создаваемый объект
	//
	OAF::URef<OAF::IUnknown> o;

	//
	// Моникер !
	//
	if (prefix () == "!")
	{
		//
		// Данный моникер в обязательном порядке требует суффикс
		//
		if (!suffix ().isEmpty ())
		{
			//
			// Данный моникер в обязательном порядке требует наличия левого моникера
			// с интерфейсом OAF::IObjectCollection
			//
			if (OAF::URef<OAF::IObjectCollection> oc = OAF::queryInterface<OAF::IObjectCollection> (left ()))
				o = oc->getObject (suffix ());
		}
	}

	//
	// Моникер cid:
	//
	else if (prefix () == "cid:")
	{
		//
		// Данный моникер в обязательном порядке требует суффикс
		//
		if (!suffix ().isEmpty ())
			o = OAF::createFromCID (suffix ());
	}

	//
	// Моникер new:
	//
	else if (prefix () == "new:")
	{
		//
		// Данный моникер в обязательном порядке требует суффикс
		//
		if (!suffix ().isEmpty ())
		{
			//
			// Данный моникер в обязательном порядке требует наличия левого моникера
			// с интерфейсом OAF::IGenericFactory
			//
			if (OAF::URef<OAF::IGenericFactory> of = OAF::queryInterface<OAF::IGenericFactory> (left ()))
				o = of->createObject (suffix ());
		}
	}

	//
	// Увы, но подходящий объект создать не удалось
	//
	return o;
}
