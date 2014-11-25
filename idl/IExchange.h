/**
 * @file
 * @brief Вспомогательные интерфейсы для текстового документа
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * FIXME: внести путь к файлу в список опций. Добавить опции в импорт
 */
#ifndef __IEXCHANGE_H
#define __IEXCHANGE_H

#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QMetaType>

#include <idl/IInterface.h>
#include <idl/IPropertyBag.h>

namespace OAF
{
	/**
	 * @brief Параметры экспорта
	 */
	enum ExportOption
	{
		/**
		 * @brief Экспортировать данные полностью
		 */
		AsFullText,

		/**
		 * @brief Экспортировать данные для включения в другой набор данных
		 */
		AsIncludedText,

		/**
		 * @brief Использовать пути к файлам изображений из временной папки буфера
		 */
		UseTempImgPathes,

		/**
		 * @brief Экспортировать как ячейку таблицы
		 */
		AsTableCell = 4
	};

	Q_DECLARE_FLAGS (ExportOptions, ExportOption)

	/**
	 * @brief Исключение рекурсивного экспорта
	 */
	struct RecursiveExportException {};

	/**
	 * @brief Интерфейс экспорта данных
	 */
	struct IExportable : virtual public OAF::IInterface
	{
		/**
		 * @brief Возвращает список MIME-типов, в которые можно экспортировать данный объект
		 */
		virtual std::size_t exportMimeTypes (QStringList& _out) const = 0;

		/**
		 * @brief Получить доп. информацию для экспорта в заданном формате
		 *
		 * Что конкретно представляет собой данная информация определяется форматом. Например
		 * для LaTeX это список дополнительных пакетов, которые необходимы для обработки
		 * экспортируемых данных.
		 */
		virtual std::size_t exportFeatures (QStringList& _out, const QString& _mime_type) = 0;

		/**
		 * @brief Выполнить проверку объекта, перед экспортом
		 */
		virtual void exportVerify (OAF::IPropertyBag* _warnings) const = 0;

		/**
		 * @brief Сохранить объект в заданном двоичном потоке
		 *
		 * FIXME: убрать _efp за счёт использования OAF::CDeviceDerived
		 */
		virtual QDataStream& exportTo (QDataStream& _os, const QStringList& _mime_types, OAF::IPropertyBag* _options = NULL,
									   const QString& _efp = QString ()) = 0;
	};

	/**
	 * @brief Интерфейс имспорта данных
	 */
	struct IImportable : virtual public OAF::IInterface
	{
		/**
		 * @brief Возвращает список MIME-типов, из которых можно импортировать данный документ
		 */
		virtual std::size_t importMimeTypes (QStringList& _out) const = 0;

		/**
		 * @brief Загрузить объект из заданного двоичного потока
		 */
		virtual QDataStream& importFrom (QDataStream& _os, const QStringList& _mime_types) = 0;
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS (OAF::ExportOptions)
Q_DECLARE_METATYPE(OAF::ExportOptions)

#endif /* __IEXCHANGE_H */
