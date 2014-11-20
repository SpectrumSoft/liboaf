/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_git_clone_h__
#define INCLUDE_git_clone_h__

#include "common.h"
#include "types.h"
#include "indexer.h"
#include "checkout.h"
#include "remote.h"


/**
 * @file git2/clone.h
 * @brief Git cloning routines
 * @defgroup git_clone Git cloning routines
 * @ingroup Git
 * @{
 */
GIT_BEGIN_DECL

/**
 * Clone options structure
 *
 * Use the GIT_CLONE_OPTIONS_INIT to get the default settings, like this:
 *
 *		git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
 *
 * - `checkout_opts` are option passed to the checkout step.  To disable
 *   checkout, set the `checkout_strategy` to GIT_CHECKOUT_NONE.
 *   Generally you will want the use GIT_CHECKOUT_SAFE_CREATE to create
 *   all files in the working directory for the newly cloned repository.
 * - `bare` should be set to zero (false) to create a standard repo,
 *   or non-zero for a bare repo
 * - `ignore_cert_errors` should be set to 1 if errors validating the
 *   remote host's certificate should be ignored.
 *
 *   ** "origin" remote options: **
 *
 * - `remote_name` is the name to be given to the "origin" remote.  The
 *   default is "origin".
 * - `checkout_branch` gives the name of the branch to checkout.  NULL
 *   means use the remote's HEAD.
 */

typedef struct git_clone_options {
	unsigned int version;

	git_checkout_opts checkout_opts;
	git_remote_callbacks remote_callbacks;

	int bare;
	int ignore_cert_errors;
	const char *remote_name;
	const char* checkout_branch;
} git_clone_options;

#define GIT_CLONE_OPTIONS_VERSION 1
#define GIT_CLONE_OPTIONS_INIT {GIT_CLONE_OPTIONS_VERSION, {GIT_CHECKOUT_OPTS_VERSION, GIT_CHECKOUT_SAFE_CREATE}, GIT_REMOTE_CALLBACKS_INIT}

/**
 * Clone a remote repository.
 *
 * This version handles the simple case. If you'd like to create the
 * repository or remote with non-default settings, you can create and
 * configure them and then use `git_clone_into()`.
 *
 * @param out pointer that will receive the resulting repository object
 * @param url the remote repository to clone
 * @param local_path local directory to clone to
 * @param options configuration options for the clone.  If NULL, the
 *        function works as though GIT_OPTIONS_INIT were passed.
 * @return 0 on success, any non-zero return value from a callback
 *         function, or a negative value to indicate an error (use
 *         `giterr_last` for a detailed error message)
 */
GIT_EXTERN(int) git_clone(
	git_repository **out,
	const char *url,
	const char *local_path,
	const git_clone_options *options);

/**
 * Clone into a repository
 *
 * After creating the repository and remote and configuring them for
 * paths and callbacks respectively, you can call this function to
 * perform the clone operation and optionally checkout files.
 *
 * @param repo the repository to use
 * @param remote the remote repository to clone from
 * @param co_opts options to use during checkout
 * @param branch the branch to checkout after the clone, pass NULL for the
 *        remote's default branch
 * @return 0 on success, any non-zero return value from a callback
 *         function, or a negative value to indicate an error (use
 *         `giterr_last` for a detailed error message)
 */
GIT_EXTERN(int) git_clone_into(
	git_repository *repo,
	git_remote *remote,
	const git_checkout_opts *co_opts,
	const char *branch);

/** @} */
GIT_END_DECL
#endif
