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

const QString OAF::TXT::CCommon::factory_cid       = "OAF/TXT/CFactory:1.0";
const QString OAF::TXT::CCommon::text_document_cid = "OAF/TXT/CTextDocument:1.0";
const QString OAF::TXT::CCommon::text_edit_cid     = "OAF/TXT/CTextEdit:1.0";
