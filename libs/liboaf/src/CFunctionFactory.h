/**
 * @file
 * @brief Реализация фабрики функций для OAF
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_CFUNCTION_FACTORY_H
#define __OAF_CFUNCTION_FACTORY_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IQuery.h>

#include <OAF/OafGlobal.h>

namespace OAF
{
	/**
	 * @brief Фабрика функций OAF
	 */
	class OAF_EXPORT CFunctionFactory :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IFunctionFactory
	{
		static FunctionDesc      funcs[];
		static const std::size_t funcs_size;

	public:
		CFunctionFactory ();
		~CFunctionFactory ();

		/**
		 * @brief Наити описатель функции по её имени
		 */
		const FunctionDesc* lookup (const QString& _name) const;
	};
}

#endif /* __OAF_CFUNCTION_FACTORY_H */
