/**
 * @file
 * @brief Стандартная реализация OAF::IUnknown
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CUnknown.h>

QSet<OAF::CUnknown*> OAF::CUnknown::m_objects;

OAF::CUnknown::CUnknown (const QString& _cid) : m_refs (0), m_cid (_cid)
{
	//qDebug ("Create: %s at %p", qPrintable (m_cid), this);

	m_objects.insert (this);
}

OAF::CUnknown::CUnknown (const CUnknown& _ou) : OAF::IInterface (_ou), OAF::IUnknown (_ou),
	m_refs (0), m_cid (_ou.m_cid)
{
	//qDebug ("Create: %s at %p", qPrintable (m_cid), this);

	m_objects.insert (this);
}

OAF::CUnknown::~CUnknown ()
{
	//qDebug ("Delete: %s at %p", qPrintable (m_cid), this);

	m_objects.remove (this);
}

std::size_t
OAF::CUnknown::ref ()
{
	return ++m_refs;
}

std::size_t
OAF::CUnknown:: unref ()
{
	//
	// Текущее значение счётчика ссылок
	//
	// Если счётчик ссылок уже ноль - то это тоже ноль, иначе - значение счётчика
	// ссылок, уменьшенное на единицу. Такая специальная обработка ноля нужна для
	// случая вызова OAF::IUnknown::unref для объекта, к которому никогда не
	// применялась функция увеличения счётчика ссылок OAF::IUnknown::ref.
	//
	// Сохранение текущего значения счётчика во временной переменной делаем
	// потому, что нам надо будет его вернуть, а делать это для m_refs от
	// удалённого объекта нельзя
	//
	std::size_t refs = (m_refs == 0) ? 0 : --m_refs;

	//
	// Если больше ссылок на объект не осталось, то удаляем его
	//
	if (refs == 0)
		delete this;

	//
	// Возвращаем текущее значение счётчика ссылок
	//
	return refs;
}

std::size_t
OAF::CUnknown::refs () const
{
	return m_refs;
}

const QString&
OAF::CUnknown::cid () const
{
	return m_cid;
}

const QSet<OAF::CUnknown*>& OAF::CUnknown::objects ()
{
	return m_objects;
}
