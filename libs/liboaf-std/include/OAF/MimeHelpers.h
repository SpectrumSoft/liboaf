/**
 * @file
 * @brief Функции для работы с MIME-типами
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_MIMEHELPERS_H
#define __OAF_MIMEHELPERS_H

#include <OAF/OafStdGlobal.h>

#include <QString>
#include <QStringList>
#include <QDir>
#include <QByteArrayMatcher>

#include <idl/IIODevice.h>

namespace OAF
{
	/**
	 * @brief Структура, описывающая MIME-тип
	 */
	struct OAFSTD_EXPORT MimeTypeInfo
	{
		/**
		 * @brief Наименование MIME-типа (например, "text/plain")
		 */
		QString mime_type;

		/**
		 * @brief Название MIME-типа для технических целей (например, "text_plain")
		 */
		QString name;

		/**
		 * @brief Псевдонимы данного MIME-типа (например, text/xml для application/xml)
		 */
		QStringList alias_mimes;

		/**
		 * @brief Родительские MIME-типы, от которых наследуется данный
		 *
		 * Например, для text/html это будут application/xml, text/plain
		 */
		QStringList parent_mimes;

		typedef QPair<QString /* *.ext */, bool /* case-sensitive */> Extension;
		/**
		 * @brief Расширения, которые могут иметь файлы данного MIME-типа
		 */
		QList<Extension> extensions;

		/**
		 * @brief Расширение по умолчанию для файлов данного типа
		 */
		QString def_extension;

		/**
		 * @brief Наименование корневого элемента XML-файла
		 *
		 * К примеру, для классификатора НПО МИС это root
		 *
		 * @attention Используется только для XML-файлов
		 */
		QString root_xml;

		/**
		 * @brief Наименование пространства имён XML-файла
		 *
		 * @attention Используется только для XML-файлов
		 */
		QString namespace_uri;

		/**
		 * @brief Описание "магических" данных, которые должны
		 *        присутствовать в заголовке файла данного MIME-типа,
		 *        и способ их обнаружения
		 */
		struct MagicMatch
		{
			enum DataType
			{
				DATA_UNKNOWN = 0,
				DATA_STRING,
				DATA_BYTE,
				DATA_HOST16,
				DATA_HOST32,
				DATA_BIG16,
				DATA_BIG32,
				DATA_LITTLE16,
				DATA_LITTLE32
			};

			/**
			 * @brief Тип данных в заголовке файла: строка, байт и т.д.
			 */
			DataType type;

			/**
			 * @brief Смещение или диапазон смещений, начиная с которого
			 * должны проверяться данные
			 */
			QPair<int, int> offset;

			/**
			 * @brief Собственно "магическое" значение, которое будет искаться
			 *        по заданному смещению
			 */
			QByteArrayMatcher value;

			/**
			 * @brief Маска, которая должна перед сравнением быть применена
			 *        к считанным из файла данным
			 *
			 * @note Для строковых магических значений маска в XML-файле
			 *       записана в шестнадцатеричном формате
			 *
			 * @note Использовать обычный числовой тип невозможно, так как длина маски может превышать даже 8 байт
			 *
			 * @note Параметр опциональный
			 */
			QByteArray mask;

			MagicMatch ();

			bool isValid () const;
			bool dataIsMatching (const QByteArray& _header_data) const;
		};

		struct MagicMatchExpr
		{
			/**
			 * @brief @brief Булево выражение в обратной польской записи,
			 * описывающее порядок обнаружения magic-данных, для каждого из обнаруженных magic-описаний
			 */
			QString polish_expr;

			/**
			 * @brief Описания магических данных
			 */
			QList<MagicMatch> matches;

			MagicMatchExpr ();
			MagicMatchExpr (const QString& _polish_expr, const QList<MagicMatch>& _matches);

			bool isValid () const;
			bool dataIsMatching (const QByteArray& _header_data) const;
		};

		typedef std::map<uint /*priority*/, QList<MagicMatchExpr> /*magic value desc*/, std::greater<uint> > MagicExpressions;
		/**
		 * @brief Коллекция magic-данных для определения принадлежности потока данных MIME-типу
		 *
		 * @note Используется std::map вместо QMap, т.к. последний не поддерживает
		 * задание собственной функции сравнения для конкретного словаря
		 *
		 * @note В качестве функции сравнения задан оператор ">",
		 * т.к. нужна сортировка по убыванию приоритета magic-значения
		 *
		 * @note Несколько match-элементов могут иметь одинаковый приоритет
		 */
		MagicExpressions magic_values;

		/**
		 * @brief Тип для задания вариаций информации по MIME-типу на разных языках
		 */
		typedef QMap<QString/*lang*/, QString/*string*/> StringMap;

		/**
		 * @brief Описания MIME-типа на различных языках (например, Plain text)
		 *
		 * @note Ключом является стандартное краткое название языка (de, ru и т.д.)
		 */
		StringMap descriptions;

		/**
		 * @brief Акроним, характеризующий MIME-тип
		 *
		 * Удобен для его быстрого опознания пользователем (например, XML, JPEG)
		 */
		QString acronym;

		/**
		 * @brief Расшифровка акронима MIME-типа на различных языках
		 *
		 * Например, для акронима "XML" это "eXtensible Markup Language"
		 *
		 * @note Ключом является стандартное краткое название языка (de, ru и т.д.)
		 */
		StringMap expanded_acronym;

		/**
		 * @brief Имя файла стандартного значка для данного MIME-типа
		 *        в формате freedesktop
		 *
		 * http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html,
		 * раздел Standard MIME Type Icons): например, x-office-document
		 *
		 * @note Имеет меньший приоритет, чем @a icon, но больший чем название MIME-типа с заменой "/" на "-"
		 */
		QString generic_icon;

		/**
		 * @brief Переопределенное пользователем имя файла значка, например virtualbox-ova
		 *
		 * @note Имеет максимальный приоритет
		 */
		QString icon;

		/**
		 * @brief Является ли данный MIME-тип корректным (т.е. имеет название и список расширений файлов)
		 */
		bool isValid () const;

		/**
		 * @brief Можно ли опознать данный MIME-тип по "магическим" данным в заголовке файла
		 */
		bool hasMagicData () const;

		/**
		 * @brief Является ли данный MIME-тип XML-файлом со строго заданным корневым тегом и/или пространством имён
		 */
		bool hasMagicXmlData () const;

		/**
		 * @brief Описание MIME-типа на языке системной локали
		 */
		QString sysLocaleDesc () const;

		/**
		 * @brief Фильтр файлов для данного MIME-типа (в формате "*.x *.yy")
		 */
		QString filterString () const;
	};

	/**
	 * @brief База данных MIME-типов, установленных в системе и самом Пикет
	 *
	 * @note Под Unix-based ОС реализует спецификацию freedesktop
	 *       http://standards.freedesktop.org/shared-mime-info-spec/shared-mime-info-spec-latest.html,
	 *       под Windows считывает информацию о MIME-типах из реестра, однако не всю
	 *
	 * @note Внутренняя база MIME-типов Пикет работает одинаково на любой поддерживаемой ОС
	 */
	class OAFSTD_EXPORT CMimeDatabase
	{
		Q_DISABLE_COPY (CMimeDatabase)

		/**
		 * @brief Коллекция описаний MIME-типов
		 */
		QList<MimeTypeInfo> m_mimes;

		/**
		 * @brief Язык, который используется в системе, в краткой форме (de, ru и т.р.)
		 *
		 * @note Под Unix язык берется из переменной LANG (имеет вид "ru_RU.UTF-8"),
		 */
		QString m_lang;

		/**
		 * @brief Считывает данные MIME-типа из указанного XML-файла в формате freedesktop
		 */
		QList<MimeTypeInfo> readMimesFromXML (const QString& _path);

		/**
		 * @brief Считать все описания MIME-типов из заданной директории и добавить их
		 *        к общему списку
		 */
		void loadMimesFromDir (const QDir& _d);

		/**
		 * @brief Определяем системный язык для выдачи тектовых описаний MIME-типов
		 */
		void detectCurrentLang ();

		/**
		 * @brief Перечень родительских MIME-типов для заданного MIME-типа
		 */
		void parentMimes (const MimeTypeInfo& _mime_type_info, QStringList& _parent_mimes);

		CMimeDatabase ();

	public:
		/**
		 * @brief Возвращает экземпляр базы данных MIME-типов
		 *
		 * База данных (БД) MIME-типов строится только один раз при запуске приложения.
		 * В качестве источников данных используется внутренняя БД Пикет из кучки XML-файлов
		 * (с несколькими жизненно важными типами), и внешняя: под Unix это аналогичная по
		 * формату БД (обычно из /usr/share/mime/packages), а под Windows информация берётся
		 * из реестра
		 *
		 * @attention Как обычно, Windows отличилась, и данных о MIME-типах там хранится
		 *            заметно меньше, чем в Unix; в частности, отсутствует возможность узнать
		 *            о псевдонимах (alias) и подклассах (subclass) MIME-типа
		 *
		 * @note Язык для описаний к MIME-типов выбирается согласно переменной среды LANG
		 */
		static CMimeDatabase& instance ();

		/**
		 * @brief Возвращает название языка локали, в которой запущено приложение
		 *
		 * @note Название возвращается в краткой форме: ru, de, en и т.д.
		 */
		QString currentLang () const;

		/**
		 * @brief Возвращает готовый фильтр для QFileDialog, запросив расширение и
		 *        описание MIME-типа файла из базы MIME-типов
		 */
		QString filterFromMime (const QString& _mime_type);

		/**
		 * @brief Возвращает готовый фильтр для QFileDialog с указанным описанием,
		 *        запросив расширения указанных MIME-типов файлов из базы MIME-типов
		 */
		QString filterFromMime (const QString& _desc, const QStringList& _mime_types);
		QString filterFromMime (const QString& _desc, const QString& _mime_type);

		/**
		 * @brief Возвращает список готовых фильтров для QFileDialog,
		 *        запросив список расширений и описаний типов файлов
		 *        из базы MIME-типов
		 *
		 * @param _all_supported Добавлять ли дополнительный фильтр
		 *                       "Все поддерживаемые типы файлов"
		 */
		QStringList filterFromMime (const QStringList& _mime_types, bool _all_supported = false, const QStringList& _all_supported_mimes = QStringList ());

		/**
		 * @brief Определение списка MIME-типов потока данных
		 * @note Размер передается отдельно, т.к. @a QIODevice::size() работает не для всех типов устройств
		 */
		QStringList lookupMimeTypes (const QString& _ext, qint64 _size, QIODevice* _d);

		/**
		 * @brief Возвращает все MIME-типы, наследующиеся от указанного (text/html от text/plain например)
		 */
		QStringList mimeSuccessors (const QString& _mime_type);

		/**
		 * @brief Возвращает описание указанного MIME-типа
		 *
		 * @note Если такого MIME-типа нет в БД, то возвращается невалидный
		 *       описатель (isValid() == false)
		 */
		MimeTypeInfo mimeInfo (const QString& _mime_type);

		/**
		 * @brief Проверяет наличие указанного MIME-типа в БД
		 */
		bool mimeExists (const QString& _mime_type);
	};
}

#endif /* __OAF_MIMEHELPERS_H */
