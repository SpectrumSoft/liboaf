/**
 * @file
 * @brief Интерфейс HTML/plain документа
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CTEXTDOCUMENT_H
#define __OAF_TXT_CTEXTDOCUMENT_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#include <QtPrintSupport>
#endif

#include <idl/IUnknown.h>
#include <idl/IIODevice.h>
#include <idl/ITXTPersist.h>
#include <idl/INotifySource.h>
#include <idl/IPropertyBag.h>
#include <idl/ITextDocument.h>
#include <idl/IDifference.h>
#include <idl/ISearch.h>
#include <idl/IExchange.h>
#include <idl/IConfig.h>

#include <OAF/CUnknown.h>
#include <OAF/CPropertyBag.h>

#include "CCommon.h"
#include "CFactory.h"

namespace OAF
{
	namespace TXT
	{
		/**
		 * @brief HTML/plain документ
		 */
		class CTextDocument : public QTextDocument,
			//
			// Импортируемые реализации
			//
			public CUnknown,
			public CNotifyPropertyBag,
			public CCommon,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IIODeviceExtender,
			virtual public OAF::ITXTPersist,
			virtual public OAF::INotifySource,
			virtual public OAF::IPropertyBag,
			virtual public OAF::IDifference,
			virtual public OAF::IMatchable,
			virtual public OAF::IExportable,
			virtual public OAF::IImportable,
			virtual public OAF::IConfigurable,
			virtual public OAF::ITextDocument
		{
			Q_OBJECT

			/**
			 * @brief Фабрика компонента
			 */
			OAF::URef<CFactory> m_factory;

			/**
			 * @brief Текстовая ссылка на источник, откуда были загружены данные
			 */
			QString m_path;

			/**
			 * @brief Ссылка на конфигурацию
			 */
			OAF::URef<OAF::IPropertyBag> m_config;

			/**
			 * @brief Получить заданный ресурс
			 */
			QVariant loadResource (int _type, const QUrl& _name);

			/**
			 * @name Переопределение методов CNotifyPropertyBag
			 */
			/** @{*/
			Value getVirtualValue (const Key& _key) const;
			void setVirtualValue (const Key& _key, const Value& _value);
			bool isVirtualDefined (const Key& _key) const;
			/** @}*/

			/**
			 * @brief Формирует документ, в котором все пути к изображениям преобразованы
			 *        из относительных в абсолютные с учётом заданного @a _path для корректной
			 *        загрузки изображений
			 */
			QString imgPathsToRelative (const QString& _path);

			/**
			 * @brief Формирует документ, в котором все пути к изображениям преобразованы из
			 *        абсолютных в относительные с учё1том заданного @a _path для упрощения обмена
			 *        документами
			 */
			void imgPathsFromRelative (const QString& _path);

		private slots:
			/**
			 * @brief Реакция на изменение содержимого документа
			 */
			void aboutContentsChanged ();

		public:
			CTextDocument (CFactory* _factory, const QString& _cid);
			~CTextDocument ();

			/**
			 * @name Реализация интерфейса OAF::IIODeviceExtender
			 */
			/** @{*/
			OAF::URef<OAF::IUnknown> setIODevice (OAF::IIODevice* _io);
			/** @}*/

			/**
			 * @name Реализация интерфейса OAF::ITXTPersist
			 */
			/** @{*/
			QTextStream& load (QTextStream& _is, const QStringList& _mime_types);
			QTextStream& save (QTextStream& _os, const QStringList& _mime_types);
			/** @}*/

			/**
			 * @name Реализация методов интерфейса OAF::IPropertyBag
			 */
			/** @{*/
			std::size_t enumKeys (KeyList& _out) const;
			PropertyFlags getFlags (const Key& _key) const;
			/** @}*/

			/**
			 * @name Реализация методов интерфейса OAF::IDifference
			 */
			/** @{*/
			bool isDiffer (IUnknown* _other);
			void diff (OAF::CHTMLGenerator& _hg, bool _inserted = true);
			void diff (OAF::CHTMLGenerator& _hg, IUnknown* _other);
			/** @{*/

			/**
			 * @name Реализация интерфейса IMatchable
			 */
			/** @{ */
			int match (const QString& _pattern, Qt::CaseSensitivity _cs);
			/** @} */

			/**
			* @name Реализация интерфейса OAF::IExportable
			*/
			/** @{*/
			std::size_t exportMimeTypes (QStringList& _out) const;
			std::size_t exportFeatures (QStringList& _out, const QString& _mime_type);
			void exportVerify (IPropertyBag *_warnings) const;
			QDataStream& exportTo (QDataStream& _os, const QStringList& _mime_types, IPropertyBag* _options, const QString& _efp);
			/** @} */

			/**
			* @name Реализация интерфейса OAF::IImportable
			*/
			/** @{*/
			std::size_t importMimeTypes (QStringList& _out) const;
			QDataStream& importFrom (QDataStream& _is, const QStringList& _mime_types);
			/** @}*/

			/**
			 * @name Реализация интерфейса OAF::IConfigurable
			 */
			/** @{*/
			OAF::URef<OAF::IPropertyBag> setConfiguration (OAF::IPropertyBag* _config);
			OAF::URef<OAF::IPropertyBag> getConfiguration ();
			void showConfigurationDialog ();
			/** @}*/

			/**
			 * @name Реализация методов интерфейса OAF::ITextDocument
			 */
			/** @{*/
			QString getDocumentAnnotation ();
			bool isEmptyDocument ();
			void clear ();
			/** @}*/
		};
	}
}

#endif /* __OAF_TXT_CTEXTDOCUMENT_H */
