/**
 * @file
 * @brief Интерфейс общего класса для всех объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CCommon.h"

long OAF::TXT::CCommon::m_objects = 0;

const QString OAF::TXT::CCommon::factory_cid        = "OAF/TXT/CFactory:1.0";
const QString OAF::TXT::CCommon::plain_document_cid = "OAF/TXT/CPlainDocument:1.0";
const QString OAF::TXT::CCommon::plain_edit_cid     = "OAF/TXT/CPlainEdit:1.0";
const QString OAF::TXT::CCommon::html_document_cid  = "OAF/TXT/CHTMLDocument:1.0";
const QString OAF::TXT::CCommon::html_edit_cid      = "OAF/TXT/CHTMLEdit:1.0";
