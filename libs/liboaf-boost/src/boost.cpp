/**
 * \file boost.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <QCoreApplication>
#include <QDebug>

#include <exception>

using namespace std;

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
void throw_exception(std::exception const & e)
{
	qCritical() << "Exception caught:\n" << e.what() << endl;
	Q_ASSERT(false);
}
#endif


void assertion_failed(char const * expr, char const * function,
			  char const * file, long line)
{
	qCritical() << "Assertion triggered in " << function
				<< " by failing check \"" << expr << "\""
				<< " in file " << file << ":" << line << endl;

	// FIXME: by default we exit here but we could also inform the user
	// about the assertion and do the emergency cleanup without exiting.
	// FIXME: do we have a list of exit codes defined somewhere?
	QCoreApplication::exit(1);
}

} // namespace boost
