/**
 * @file
 * @brief Вспомогательные функции для экспорта/импорта
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __EXCHANGE_HELPERS_H
#define __EXCHANGE_HELPERS_H

#include <OAF/OafStdGlobal.h>

#include <idl/IExchange.h>

#include <OAF/OAF.h>

namespace OAF
{
	/**
	 * @brief Вспомогательные функции, для экспорта в строку
	 */
	/** @{ */
	OAFSTD_EXPORT QString exportToString (OAF::IExportable* _eo, const QString& _mime_type, IPropertyBag* _options = NULL, const QString& _efp = QString ());
	OAFSTD_EXPORT QString exportToString (OAF::IExportable* _eo, const QStringList& _mime_types, IPropertyBag* _options = NULL, const QString& _efp = QString ());
	/** @} */

	OAFSTD_EXPORT void importFromString (OAF::IImportable* _io, const QString& _data_to_import, const QString& _mime_type);
	OAFSTD_EXPORT void importFromString (OAF::IImportable* _io, const QString& _data_to_import, const QStringList& _mime_types);

	/**
	 * @brief Вспомогательные функции, для экспорта в команд LaTeX'а
	 */
	/** @{ */
	OAFSTD_EXPORT QDataStream& latexRawCommand (QDataStream& _os, const QString& _raw_cmd);
	/*
	 %cmd - комманда
	 %val1, %val2 ... - значения
	 */
	OAFSTD_EXPORT QDataStream& latexCommand (QDataStream& _os, const QString& _cmd, const QString& _val, const QString& _format);
	OAFSTD_EXPORT QDataStream& latexCommand (QDataStream& _os, const QString& _cmd, const QString& _val1, const QString& _val2, const QString& _format);
	OAFSTD_EXPORT QDataStream& latexCommand (QDataStream& _os, const QString& _cmd, const QStringList& _values, const QString& _format);
	OAFSTD_EXPORT QDataStream& latexCommand (QDataStream& _os, const QString& _cmd, OAF::IExportable* _val, const QString& _format, const QString& _mime_type, IPropertyBag* _o, const QString &_efp = QString ());
	/** @} */

	/**
	 * @brief Представление UUID без фигурных скобок
	 */
	OAFSTD_EXPORT QString cleanUid (const QUuid& _id);

	/**
	 * @brief Закодировать строку в соответствии с правилами LaTeX
	 */
	OAFSTD_EXPORT QString asLatex (const QString& _s);
}

#endif /* __EXCHANGE_HELPERS_H */
