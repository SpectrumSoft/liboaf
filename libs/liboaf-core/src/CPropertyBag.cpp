/**
 * @file
 * @brief Стандартные реализации контейнера атрибутов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QtCore>

#include <OAF/CPropertyBag.h>

bool
OAF::CPropertyBag::isVirtualDefined (const Key& _key) const
{
	Q_UNUSED (_key);

	return false;
}

OAF::CPropertyBag::Value
OAF::CPropertyBag::getVirtualValue (const Key& _key) const
{
	Q_UNUSED (_key);

	return Value ();
}

void
OAF::CPropertyBag::setVirtualValue (const Key& _key, const Value& _value)
{
	Q_UNUSED (_key);
	Q_UNUSED (_value);
}

OAF::CPropertyBag::CPropertyBag ()
{}

OAF::CPropertyBag::CPropertyBag (const PropertySet& _values) : m_values (_values)
{}

OAF::CPropertyBag::CPropertyBag (const CPropertyBag& _bag) : OAF::IInterface (_bag), OAF::IPropertyBag (_bag),
	m_values (_bag.m_values)
{}

std::size_t
OAF::CPropertyBag::enumKeys (KeyList& _out) const
{
	_out = m_values.keys ();

	return _out.size ();
}

OAF::IPropertyBag::PropertyFlags
OAF::CPropertyBag::getFlags (const Key& /*_key*/) const
{
	return PROPERTY_WRITEABLE;
}

bool
OAF::CPropertyBag::isDefined (const Key& _key) const
{
	PropertyFlags f = getFlags (_key);

	if (f & PROPERTY_VIRTUAL)
		return isVirtualDefined (_key);

	return m_values.contains (_key);
}

OAF::IPropertyBag::Value
OAF::CPropertyBag::getValue (const Key& _key) const
{
	Value out;

	//
	// Виртуальные атрибуты получаем через специальную функцию
	//
	if (getFlags (_key) & PROPERTY_VIRTUAL)
		out = getVirtualValue (_key);

	//
	// Обычные читаемые атрибуты извлекаем из общего списка
	//
	else
		out = m_values.value (_key, Value ());

	return out;
}

std::size_t
OAF::CPropertyBag::getValues (PropertySet& _out, PropertyFlags _f) const
{
	//
	// Получаем список ключей возможных атрибутов
	//
	KeyList keys;
	enumKeys (keys);

	//
	// Проходим по всему списку ключей и заполняем результат непустыми значениями
	//
	for (KeyList::const_iterator k = keys.begin (); k != keys.end (); ++k)
	{
		PropertyFlags f = getFlags (*k);

		//
		// Выбираем все значения, у которых установлены все заданные атрибуты
		//
		if ((f & _f) == _f)
		{
			Value v = CPropertyBag::getValue (*k);
			if (!v.isNull ())
				_out.insert (*k, v);
		}
	}

	//
	// Возвращаем количество выбранных значений
	//
	return _out.size ();
}

void
OAF::CPropertyBag::setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_origin);

	PropertyFlags f = getFlags (_key);

	//
	// Можно изменять только атрибуты, для которых задан флаг
	// возможности записи
	//
	if (f & PROPERTY_WRITEABLE)
	{
		//
		// Устанавливаем виртуальный атрибут
		//
		if (f & PROPERTY_VIRTUAL)
			setVirtualValue (_key, _value);

		//
		// Для остальных атрибутов
		//
		else
		{
			//
			// Если новое значение атрибута NULL, то удаляем его из списка
			//
			if (_value.isNull ())
				m_values.remove (_key);
			//
			// Иначе сохраняем новый атрибут в списке
			//
			else
				m_values[_key] = _value;
		}
	}
}

void
OAF::CPropertyBag::setValues (const PropertySet& _values, OAF::INotifyListener* _origin)
{
	//
	// Проходим по всему списку новых значений и устанавливаем их
	//
	for (PropertySet::const_iterator v = _values.begin (); v != _values.end (); ++v)
		CPropertyBag::setValue (v.key (), v.value (), _origin);
}

OAF::CSlicePropertyBag::CSlicePropertyBag (const QString& _prefix, OAF::IPropertyBag* _props, bool _auto) :
	m_prefix (_prefix), m_auto (true)
{
	attach (_props, _auto);
}

OAF::CSlicePropertyBag::CSlicePropertyBag (const CSlicePropertyBag& _bag) :
	CNotifySource (_bag), m_prefix (_bag.m_prefix)
{
	attach (_bag.m_props, _bag.m_auto);
}

OAF::CSlicePropertyBag::~CSlicePropertyBag ()
{
	attach (NULL, m_auto);
}

void
OAF::CSlicePropertyBag::notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_source);

	if (OAF::IPropertyNotify* pev = OAF::queryInterface<OAF::IPropertyNotify> (_event))
	{
		//
		// Обрабатываем события только от прослушиваемого источника
		//
		if (m_props.is (OAF::queryInterface<OAF::IPropertyBag> (pev->object)))
		{
			//
			// Создаём набор изменённых свойств с заданным префиксом (с его удалением)
			//
			PropertySet values;
			for (PropertySet::const_iterator p = pev->values.begin (); p != pev->values.end (); ++p)
			{
				if (p.key ().startsWith (m_prefix))
					values.insert (p.key ().mid (m_prefix.length ()), p.value ());
			}

			//
			// Если данный набор свойств не пуст, то рассылаем его
			//
			if (!values.isEmpty ())
			{
				OAF::IPropertyNotify ppev (pev->when, values, this);
				notifyListeners (&ppev, _origin);
			}
		}
	}
}

std::size_t
OAF::CSlicePropertyBag::enumKeys (KeyList& _out) const
{
	if (m_props)
	{
		KeyList keys;
		m_props->enumKeys (keys);

		for (KeyList::const_iterator k = keys.begin (); k != keys.end (); ++k)
		{
			if (k->startsWith (m_prefix))
				_out.append (k->mid (m_prefix.length ()));
		}
	}

	return _out.size ();
}

OAF::IPropertyBag::PropertyFlags
OAF::CSlicePropertyBag::getFlags (const Key& _key) const
{
	if (m_props)
		return m_props->getFlags (m_prefix + _key);
	return 0;
}

bool
OAF::CSlicePropertyBag::isDefined (const Key& _key) const
{
	if (m_props)
		return m_props->isDefined (m_prefix + _key);
	return false;
}

OAF::IPropertyBag::Value
OAF::CSlicePropertyBag::getValue (const Key& _key) const
{
	if (m_props)
		return m_props->getValue (m_prefix + _key);
	return OAF::IPropertyBag::Value ();
}

std::size_t
OAF::CSlicePropertyBag::getValues (PropertySet& _out, PropertyFlags _f) const
{
	if (m_props)
	{
		KeyList keys;
		enumKeys (keys);

		for (KeyList::const_iterator k = keys.begin (); k != keys.end (); ++k)
		{
			PropertyFlags f = getFlags (*k);

			if ((f & _f) == _f)
				_out.insert (*k, getValue (*k));
		}
	}

	return _out.size ();
}

void
OAF::CSlicePropertyBag::setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin)
{
	if (m_props)
		m_props->setValue (m_prefix + _key, _value, _origin);
}

void
OAF::CSlicePropertyBag::setValues (const PropertySet& _values, OAF::INotifyListener* _origin)
{
	if (m_props)
	{
		PropertySet values;

		for (PropertySet::const_iterator v = _values.begin (); v != _values.end (); ++v)
			values[m_prefix + v.key ()] = v.value ();

		m_props->setValues (values, _origin);
	}
}

OAF::URef<OAF::IPropertyBag>
OAF::CSlicePropertyBag::attach (OAF::IPropertyBag* _props, bool _auto)
{
	OAF::URef<OAF::IPropertyBag> props = m_props;

	if (m_auto)
		OAF::unsubscribe (m_props, this);

	m_props = _props;
	m_auto  = _auto;

	if (m_auto)
		OAF::subscribe (m_props, this);

	return props;
}

OAF::CNotifyPropertyBag::CNotifyPropertyBag ()
{}

OAF::CNotifyPropertyBag::CNotifyPropertyBag (const PropertySet& _values) :
	CPropertyBag (_values)
{}

OAF::CNotifyPropertyBag::CNotifyPropertyBag (const CNotifyPropertyBag& _bag) :
	OAF::IInterface (_bag), OAF::IPropertyBag (_bag), OAF::INotifySource (_bag),
	CPropertyBag (_bag), CNotifySource (_bag)
{}

void
OAF::CNotifyPropertyBag::setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin)
{
	{
		//
		// Собираем список всех старых значений атрибутов
		//
		PropertySet ovalues;

		//
		// Все значения с автоматическим уведомлением
		//
		CPropertyBag::getValues (ovalues, PROPERTY_AUTONOTIFY);

		//
		// Изменяемый атрибут
		//
		PropertyFlags f = getFlags (_key);
		if ((f & PROPERTY_WRITEABLE) && !(f & PROPERTY_AUTONOTIFY))
			ovalues.insert (_key, CPropertyBag::getValue (_key));

		try
		{
			//
			// Рассылаем уведомление о начале изменения
			//
			OAF::IPropertyNotify bev (OAF::IPropertyNotify::BEFORE, ovalues, this);
			notifyListeners (&bev, _origin);
		}
		//
		// Если кто-то сказал, что изменение невозможно
		//
		catch (const OAF::IPropertyNotify::Cancel&)
		{
			//
			// Рассылаем уведомление об отмене изменения
			//
			OAF::IPropertyNotify cev (OAF::IPropertyNotify::CANCELLED, ovalues, this);
			notifyListeners (&cev, _origin);

			return;
		}
	}

	//
	// Устанавливаем новое значение атрибута
	//
	CPropertyBag::setValue (_key, _value, _origin);

	{
		//
		// Собираем список всех новых значений атрибутов
		//
		PropertySet nvalues;

		//
		// Все значения с автоматическим уведомлением
		//
		CPropertyBag::getValues (nvalues, PROPERTY_AUTONOTIFY);

		//
		// Изменяемый атрибут
		//
		PropertyFlags f = getFlags (_key);
		if ((f & PROPERTY_WRITEABLE) && !(f & PROPERTY_AUTONOTIFY))
			nvalues.insert (_key, CPropertyBag::getValue (_key));

		//
		// Рассылаем уведомление о выполнении изменения
		//
		OAF::IPropertyNotify aev (OAF::IPropertyNotify::AFTER, nvalues, this);
		notifyListeners (&aev, _origin);
	}
}

void
OAF::CNotifyPropertyBag::setValues (const PropertySet& _values, OAF::INotifyListener* _origin)
{
	{
		//
		// Собираем список всех старых значений атрибутов
		//
		PropertySet ovalues;

		//
		// Все значения с автоматическим уведомлением
		//
		CPropertyBag::getValues (ovalues, PROPERTY_AUTONOTIFY);

		//
		// Добавляем к ним все изменяемые значения
		//
		for (PropertySet::const_iterator i = _values.begin (); i != _values.end (); ++i)
		{
			PropertyFlags f = getFlags (i.key ());

			if ((f & PROPERTY_WRITEABLE) && !(f & PROPERTY_AUTONOTIFY))
				ovalues.insert (i.key (), CPropertyBag::getValue (i.key ()));
		}

		try
		{
			//
			// Рассылаем уведомление
			//
			OAF::IPropertyNotify bev (OAF::IPropertyNotify::BEFORE, ovalues, this);
			notifyListeners (&bev, _origin);
		}
		//
		// Если кто-то сказал, что изменение невозможно
		//
		catch (const OAF::IPropertyNotify::Cancel&)
		{
			//
			// Рассылаем уведомление об отмене изменений
			//
			OAF::IPropertyNotify cev (OAF::IPropertyNotify::CANCELLED, ovalues, this);
			notifyListeners (&cev, _origin);

			return;
		}
	}

	//
	// Устанавливаем новые значения атрибутов
	//
	CPropertyBag::setValues (_values, _origin);

	{
		//
		// Собираем список всех новых значений атрибутов
		//
		PropertySet nvalues;

		//
		// Все значения с автоматическим уведомлением
		//
		CPropertyBag::getValues (nvalues, PROPERTY_AUTONOTIFY);

		//
		// Добавляем к ним все изменяемые значения
		//
		for (PropertySet::const_iterator i = _values.begin (); i != _values.end (); ++i)
		{
			PropertyFlags f = getFlags (i.key ());

			if ((f & PROPERTY_WRITEABLE) && !(f & PROPERTY_AUTONOTIFY))
				nvalues.insert (i.key (), CPropertyBag::getValue (i.key ()));
		}

		//
		// Рассылаем уведомление о выполнении изменения
		//
		OAF::IPropertyNotify aev (OAF::IPropertyNotify::AFTER, nvalues, this);
		notifyListeners (&aev, _origin);
	}
}

OAF::CPropertyBagObject::CPropertyBagObject (const QString& _cid) : CUnknown (_cid)
{}

OAF::CPropertyBagObject::CPropertyBagObject (const QString& _cid, const PropertySet& _values) :
	CUnknown (_cid), CNotifyPropertyBag (_values)
{}

OAF::CPropertyBagObject::CPropertyBagObject (const CPropertyBagObject& _bag) :
	OAF::IInterface (_bag), OAF::IUnknown (_bag), OAF::IPropertyBag (_bag), OAF::INotifySource (_bag),
	CUnknown (_bag), CPropertyBag (_bag), CNotifySource (_bag), CNotifyPropertyBag (_bag)
{}

OAF::CSlicePropertyBagObject::CSlicePropertyBagObject (const QString& _cid, const QString& _prefix, OAF::IPropertyBag* _props, bool _auto) :
	CUnknown (_cid), CSlicePropertyBag (_prefix, _props, _auto)
{}

OAF::CSlicePropertyBagObject::CSlicePropertyBagObject (const CSlicePropertyBagObject& _bag) :
	OAF::IInterface (_bag), OAF::IUnknown (_bag), OAF::IPropertyBag (_bag), OAF::INotifySource (_bag),
	CUnknown (_bag), CNotifySource (_bag), CSlicePropertyBag (_bag)
{}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Пространство имён свойств
//
static QString a = "http://www.spectrumsoft.msk.ru/files/specs/attributes-syntax.dtd";

std::size_t
OAF::loadXML (OAF::IPropertyBag::PropertySet& _out, QXmlStreamReader& _xml)
{
	//
	// Если первый элемент это a:attributes
	//
	if (_xml.readNextStartElement () && (_xml.namespaceUri () == a) && (_xml.name () == "attributes"))
	{
		//
		// Пока следующим элементом является a:attribute
		//
		while (_xml.readNextStartElement () && (_xml.namespaceUri () == a) && (_xml.name () == "attribute"))
		{
			//
			// Проверяем обязательные атрибуты a:attribute
			//
			QXmlStreamAttributes aattrs = _xml.attributes ();
			if (aattrs.hasAttribute ("name") && aattrs.hasAttribute ("type"))
			{
				//
				// Если это список строк
				//
				if (aattrs.value ("type") == "stringv")
				{
					QStringList stringv;

					//
					// Пока следующим элементом является элемент списка строк
					//
					while (_xml.readNextStartElement () && (_xml.namespaceUri () == a) && (_xml.name () == "item"))
					{
						QXmlStreamAttributes iattrs = _xml.attributes ();
						if (iattrs.hasAttribute ("value"))
							stringv.append (iattrs.value ("value").toString ());

						//
						// Если ещё не находимся в конце элемента, то пропускаем всё
						// до конца p:item
						//
						if (!_xml.isEndElement ())
							_xml.skipCurrentElement ();
					}

					//
					// Устанавливаем значение соответствующего атрибута
					//
					_out.insert (aattrs.value ("name").toString (), QVariant (stringv));
				}

				//
				// Если это строка
				//
				else if (aattrs.value ("type") == "string")
				{
					//
					// Устанавливаем значение соответствующего атрибута
					//
					if (aattrs.hasAttribute ("value"))
						_out.insert (aattrs.value ("name").toString (), aattrs.value ("value").toString ());
				}

				//
				// Если это булевское значение
				//
				else if (aattrs.value ("type") == "boolean")
				{
					//
					// Устанавливаем значение соответствующего атрибута
					//
					if (aattrs.hasAttribute ("value"))
					{
						if (aattrs.value ("value").compare ("true", Qt::CaseInsensitive) == 0)
							_out.insert (aattrs.value ("name").toString (), true);
						else if (aattrs.value ("value").compare ("yes", Qt::CaseInsensitive) == 0)
							_out.insert (aattrs.value ("name").toString (), true);
						else if (aattrs.value ("value").compare ("false", Qt::CaseInsensitive) == 0)
							_out.insert (aattrs.value ("name").toString (), false);
						else if (aattrs.value ("value").compare ("no", Qt::CaseInsensitive) == 0)
							_out.insert (aattrs.value ("name").toString (), false);
					}
				}

				//
				// Если это целое число
				//
				else if (aattrs.value ("type") == "integer")
				{
					//
					// Устанавливаем значение соответствующего атрибута
					//
					if (aattrs.hasAttribute ("value"))
						_out.insert (aattrs.value ("name").toString (), aattrs.value ("value").toString ().toLongLong ());
				}

				//
				// Если это число
				//
				else if (aattrs.value ("type") == "double")
				{
					//
					// Устанавливаем значение соответствующего атрибута
					//
					if (aattrs.hasAttribute ("value"))
						_out.insert (aattrs.value ("name").toString (), aattrs.value ("value").toString ().toDouble ());
				}

				//
				// Если это дата и время
				//
				else if (aattrs.value ("type") == "datetime")
				{
					//
					// Устанавливаем значение соответствующего атрибута
					//
					if (aattrs.hasAttribute ("value"))
						_out.insert (aattrs.value ("name").toString (),
									 QDateTime::fromString (aattrs.value ("value").toString (), Qt::ISODate));
				}

				//
				// Если это неизвестный тип, то пропускаем его
				//
			}

			//
			// Если мы ещё не находимся в конце елемента, то пропускаем всё
			// до конца p:attribute
			//
			if (!_xml.isEndElement ())
				_xml.skipCurrentElement ();
		}
	}

	return _out.size ();
}

std::size_t
OAF::saveXML (const OAF::IPropertyBag::PropertySet& _in, QXmlStreamWriter& _xml)
{
	_xml.writeNamespace (a, "a");
	_xml.writeStartElement (a, "attributes");

	for (OAF::IPropertyBag::PropertySet::const_iterator v = _in.begin (); v != _in.end (); ++v)
	{
		_xml.writeStartElement (a, "attribute");
		_xml.writeAttribute ("name", v.key ());

		switch (v.value ().type ())
		{
			case QVariant::StringList:
				{
					_xml.writeAttribute ("type", "stringv");

					QStringList items = v.value ().value<QStringList> ();
					for (QStringList::const_iterator s = items.begin (); s != items.end (); ++s)
					{
						_xml.writeStartElement (a, "item");
						_xml.writeAttribute ("value", *s);
						_xml.writeEndElement ();
					}
				}
				break;

			case QVariant::String:
				_xml.writeAttribute ("type", "string");
				_xml.writeAttribute ("value", v.value ().value<QString> ());
				break;

			case QVariant::Bool:
				_xml.writeAttribute ("type", "boolean");
				_xml.writeAttribute ("value", ((v.value ().value<bool> ()) ? "true" : "false"));
				break;

			case QVariant::LongLong:
				_xml.writeAttribute ("type", "integer");
				_xml.writeAttribute ("value", v.value ().toString ());
				break;

			case QVariant::Double:
				_xml.writeAttribute ("type", "double");
				_xml.writeAttribute ("value", v.value ().toString ());
				break;

			case QVariant::DateTime:
				_xml.writeAttribute ("type", "datetime");
				_xml.writeAttribute ("value", v.value ().toString ());
				break;

			default:
				//
				// Остальное - игнорируем
				//
				break;
		}

		_xml.writeEndElement ();
	}

	_xml.writeEndElement ();

	return _in.size ();
}
