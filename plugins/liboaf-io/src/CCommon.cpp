/**
 * @file
 * @brief Интерфейс общего класса для всех объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include "CCommon.h"

using namespace OAF::IO;

long OAF::IO::CCommon::m_objects = 0;

const QString OAF::IO::CCommon::factory_cid       = "OAF/IO/CFactory:1.0";
const QString OAF::IO::CCommon::device_file_cid   = "OAF/IO/CDeviceFile:1.0";
const QString OAF::IO::CCommon::device_buffer_cid = "OAF/IO/CDeviceBuffer:1.0";
const QString OAF::IO::CCommon::device_git_cid    = "OAF/IO/CDeviceGIT:1.0";
const QString OAF::IO::CCommon::moniker_cid       = "OAF/IO/CMonikerIO:1.0";
