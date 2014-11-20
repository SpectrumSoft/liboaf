/**
 * @file
 * @brief Реализация вспомогательных функций для создания объектов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/Helpers.h>

QStringList
OAF::mimeForObject (OAF::IUnknown* _o)
{
	QStringList mime_types;

	//
	// Если это прокси-объект, то определяем его MIME-тип по проксируемому объекту
	//
	if (URef<OAF::IUnknownProxy> op = OAF::queryInterface<OAF::IUnknownProxy> (_o))
		return OAF::mimeForObject (op->getExtendedObject ());

	if (_o)
	{
		if (const OAF::IPropertyBag* cinfo = OAF::getClassInfo (_o->cid ()))
		{
			if (cinfo->isDefined ("mime_types"))
				mime_types = cinfo->getValue ("mime_types").toStringList ();
		}
	}

	return mime_types;
}

QString
OAF::mimeAsString (const QStringList& _list)
{
	QString res;

	for (QStringList::const_iterator s = _list.begin (); s != _list.end (); ++s)
	{
		if (s != _list.begin ())
			res += ';';
		res += *s;
	}

	return res;
}

QStringList
OAF::mimeAsStringList (const QString& _s)
{
	return _s.split (';', QString::SkipEmptyParts);
}
