/**
 * @file
 * @brief Пакет интерфейсов алгоритмов вычисления сигнатур для текстов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __ITEXTSIGNATURE_H
#define __ITEXTSIGNATURE_H

#include <QtGlobal>
#include <QString>

#include <idl/IInterface.h>

namespace OAF
{
	/**
	 * @brief Интерфейс запроса проверки слова
	 */
	struct ITextSignatureCheck : virtual public OAF::IInterface
	{
		/**
		 * @brief Проверить, не является ли заданное слово словарным
		 */
		virtual bool checkDictionaryWord (const QString& _word) = 0;
	};

	/**
	 * @brief Интерфейс сбора статистики по текстам
	 *
	 * Данный интерфейс предназначен для сбора статистики для алгоритмов,
	 * которые предусматривают учёт глобального распределения слов по
	 * текстам.
	 */
	struct ITextSignatureStat : virtual public OAF::IInterface
	{
		/**
		 * @brief Добавить текст к статистике
		 */
		virtual void addSignatureStat (const QString& _s, ITextSignatureCheck* _checker = NULL) = 0;

		/**
		 * @brief Убрать текст из статистики
		 */
		virtual void removeSignatureStat (const QString& _s, ITextSignatureCheck* _checker = NULL) = 0;
	};

	/**
	 * @brief Интерфейс вычисления 32-х разрядной сигнатуры для заданного текста
	 *
	 * Если сигнатуры для двух текстов совпадают, то эти тексты можно считають
	 * эквивалентными по содержанию.
	 */
	struct ITextSignature : virtual public OAF::IInterface
	{
		/**
		 * @brief Вычислить сигнатуру для текста
		 */
		virtual quint32 signature (const QString& _s, ITextSignatureCheck* _checker = NULL) = 0;
	};
}

#endif /* __ITEXTSIGNATURE_H */
