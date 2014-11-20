/**
 * @file
 * @brief Интерфейс общего класса для всех объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CCommon.h"

long OAF::STD::CCommon::m_objects = 0;

const QString OAF::STD::CCommon::factory_cid = "OAF/STD/CFactory:1.0";
const QString OAF::STD::CCommon::moniker_cid = "OAF/STD/CMonikerSTD:1.0";
