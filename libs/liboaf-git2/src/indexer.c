/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#ifndef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) (y)
#endif

#include <zlib.h>

#include "git2/indexer.h"
#include "git2/object.h"

#include "common.h"
#include "pack.h"
#include "mwindow.h"
#include "posix.h"
#include "pack.h"
#include "filebuf.h"
#include "oid.h"
#include "oidmap.h"
#include "compress.h"

#define UINT31_MAX (0x7FFFFFFF)

struct entry {
	git_oid oid;
	uint32_t crc;
	uint32_t offset;
	uint64_t offset_long;
};

struct git_indexer {
	unsigned int parsed_header :1,
		opened_pack :1,
		have_stream :1,
		have_delta :1;
	struct git_pack_header hdr;
	struct git_pack_file *pack;
	git_filebuf pack_file;
	unsigned int mode;
	git_off_t off;
	git_off_t entry_start;
	git_packfile_stream stream;
	size_t nr_objects;
	git_vector objects;
	git_vector deltas;
	unsigned int fanout[256];
	git_hash_ctx hash_ctx;
	git_oid hash;
	git_transfer_progress_callback progress_cb;
	void *progress_payload;
	char objbuf[8*1024];

	/* Needed to look up objects which we want to inject to fix a thin pack */
	git_odb *odb;

	/* Fields for calculating the packfile trailer (hash of everything before it) */
	char inbuf[GIT_OID_RAWSZ];
	size_t inbuf_len;
	git_hash_ctx trailer;
};

struct delta_info {
	git_off_t delta_off;
};

const git_oid *git_indexer_hash(const git_indexer *idx)
{
	return &idx->hash;
}

static int open_pack(struct git_pack_file **out, const char *filename)
{
	struct git_pack_file *pack;

	if (git_packfile_alloc(&pack, filename) < 0)
		return -1;

	if ((pack->mwf.fd = p_open(pack->pack_name, O_RDONLY)) < 0) {
		giterr_set(GITERR_OS, QT_TRANSLATE_NOOP("libgit2", "Failed to open packfile."));
		git_packfile_free(pack);
		return -1;
	}

	*out = pack;
	return 0;
}

static int parse_header(struct git_pack_header *hdr, struct git_pack_file *pack)
{
	int error;

	/* Verify we recognize this pack file format. */
	if ((error = p_read(pack->mwf.fd, hdr, sizeof(*hdr))) < 0) {
		giterr_set(GITERR_OS, QT_TRANSLATE_NOOP("libgit2", "Failed to read in pack header"));
		return error;
	}

	if (hdr->hdr_signature != ntohl(PACK_SIGNATURE)) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "Wrong pack signature"));
		return -1;
	}

	if (!pack_version_ok(hdr->hdr_version)) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "Wrong pack version"));
		return -1;
	}

	return 0;
}

static int objects_cmp(const void *a, const void *b)
{
	const struct entry *entrya = a;
	const struct entry *entryb = b;

	return git_oid__cmp(&entrya->oid, &entryb->oid);
}

int git_indexer_new(
		git_indexer **out,
		const char *prefix,
		unsigned int mode,
		git_odb *odb,
		git_transfer_progress_callback progress_cb,
		void *progress_payload)
{
	git_indexer *idx;
	git_buf path = GIT_BUF_INIT;
	static const char suff[] = "/pack";
	int error;

	idx = git__calloc(1, sizeof(git_indexer));
	GITERR_CHECK_ALLOC(idx);
	idx->odb = odb;
	idx->progress_cb = progress_cb;
	idx->progress_payload = progress_payload;
	idx->mode = mode ? mode : GIT_PACK_FILE_MODE;
	git_hash_ctx_init(&idx->trailer);

	error = git_buf_joinpath(&path, prefix, suff);
	if (error < 0)
		goto cleanup;

	error = git_filebuf_open(&idx->pack_file, path.ptr,
		GIT_FILEBUF_TEMPORARY | GIT_FILEBUF_DO_NOT_BUFFER,
		idx->mode);
	git_buf_free(&path);
	if (error < 0)
		goto cleanup;

	*out = idx;
	return 0;

cleanup:
	git_buf_free(&path);
	git_filebuf_cleanup(&idx->pack_file);
	git__free(idx);
	return -1;
}

/* Try to store the delta so we can try to resolve it later */
static int store_delta(git_indexer *idx)
{
	struct delta_info *delta;

	delta = git__calloc(1, sizeof(struct delta_info));
	GITERR_CHECK_ALLOC(delta);
	delta->delta_off = idx->entry_start;

	if (git_vector_insert(&idx->deltas, delta) < 0)
		return -1;

	return 0;
}

static void hash_header(git_hash_ctx *ctx, git_off_t len, git_otype type)
{
	char buffer[64];
	size_t hdrlen;

	hdrlen = git_odb__format_object_header(buffer, sizeof(buffer), (size_t)len, type);
	git_hash_update(ctx, buffer, hdrlen);
}

static int hash_object_stream(git_indexer*idx, git_packfile_stream *stream)
{
	ssize_t read;

	assert(idx && stream);

	do {
		if ((read = git_packfile_stream_read(stream, idx->objbuf, sizeof(idx->objbuf))) < 0)
			break;

		git_hash_update(&idx->hash_ctx, idx->objbuf, read);
	} while (read > 0);

	if (read < 0)
		return (int)read;

	return 0;
}

/* In order to create the packfile stream, we need to skip over the delta base description */
static int advance_delta_offset(git_indexer *idx, git_otype type)
{
	git_mwindow *w = NULL;

	assert(type == GIT_OBJ_REF_DELTA || type == GIT_OBJ_OFS_DELTA);

	if (type == GIT_OBJ_REF_DELTA) {
		idx->off += GIT_OID_RAWSZ;
	} else {
		git_off_t base_off = get_delta_base(idx->pack, &w, &idx->off, type, idx->entry_start);
		git_mwindow_close(&w);
		if (base_off < 0)
			return (int)base_off;
	}

	return 0;
}

/* Read from the stream and discard any output */
static int read_object_stream(git_indexer *idx, git_packfile_stream *stream)
{
	ssize_t read;

	assert(stream);

	do {
		read = git_packfile_stream_read(stream, idx->objbuf, sizeof(idx->objbuf));
	} while (read > 0);

	if (read < 0)
		return (int)read;

	return 0;
}

static int crc_object(uint32_t *crc_out, git_mwindow_file *mwf, git_off_t start, git_off_t size)
{
	void *ptr;
	uint32_t crc;
	unsigned int left, len;
	git_mwindow *w = NULL;

	crc = crc32(0L, Z_NULL, 0);
	while (size) {
		ptr = git_mwindow_open(mwf, &w, start, (size_t)size, &left);
		if (ptr == NULL)
			return -1;

		len = min(left, (unsigned int)size);
		crc = crc32(crc, ptr, len);
		size -= len;
		start += len;
		git_mwindow_close(&w);
	}

	*crc_out = htonl(crc);
	return 0;
}

static int store_object(git_indexer *idx)
{
	int i, error;
	khiter_t k;
	git_oid oid;
	struct entry *entry;
	git_off_t entry_size;
	struct git_pack_entry *pentry;
	git_hash_ctx *ctx = &idx->hash_ctx;
	git_off_t entry_start = idx->entry_start;

	entry = git__calloc(1, sizeof(*entry));
	GITERR_CHECK_ALLOC(entry);

	pentry = git__calloc(1, sizeof(struct git_pack_entry));
	GITERR_CHECK_ALLOC(pentry);

	git_hash_final(&oid, ctx);
	entry_size = idx->off - entry_start;
	if (entry_start > UINT31_MAX) {
		entry->offset = UINT32_MAX;
		entry->offset_long = entry_start;
	} else {
		entry->offset = (uint32_t)entry_start;
	}

	git_oid_cpy(&pentry->sha1, &oid);
	pentry->offset = entry_start;

	k = kh_put(oid, idx->pack->idx_cache, &pentry->sha1, &error);
	if (!error) {
		git__free(pentry);
		goto on_error;
	}

	kh_value(idx->pack->idx_cache, k) = pentry;

	git_oid_cpy(&entry->oid, &oid);

	if (crc_object(&entry->crc, &idx->pack->mwf, entry_start, entry_size) < 0)
		goto on_error;

	/* Add the object to the list */
	if (git_vector_insert(&idx->objects, entry) < 0)
		goto on_error;

	for (i = oid.id[0]; i < 256; ++i) {
		idx->fanout[i]++;
	}

	return 0;

on_error:
	git__free(entry);

	return -1;
}

static int save_entry(git_indexer *idx, struct entry *entry, struct git_pack_entry *pentry, git_off_t entry_start)
{
	int i, error;
	khiter_t k;

	if (entry_start > UINT31_MAX) {
		entry->offset = UINT32_MAX;
		entry->offset_long = entry_start;
	} else {
		entry->offset = (uint32_t)entry_start;
	}

	pentry->offset = entry_start;
	k = kh_put(oid, idx->pack->idx_cache, &pentry->sha1, &error);
	if (!error)
		return -1;

	kh_value(idx->pack->idx_cache, k) = pentry;

	/* Add the object to the list */
	if (git_vector_insert(&idx->objects, entry) < 0)
		return -1;

	for (i = entry->oid.id[0]; i < 256; ++i) {
		idx->fanout[i]++;
	}

	return 0;
}

static int hash_and_save(git_indexer *idx, git_rawobj *obj, git_off_t entry_start)
{
	git_oid oid;
	size_t entry_size;
	struct entry *entry;
	struct git_pack_entry *pentry;

	entry = git__calloc(1, sizeof(*entry));
	GITERR_CHECK_ALLOC(entry);

	if (git_odb__hashobj(&oid, obj) < 0) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "Failed to hash object"));
		goto on_error;
	}

	pentry = git__calloc(1, sizeof(struct git_pack_entry));
	GITERR_CHECK_ALLOC(pentry);

	git_oid_cpy(&pentry->sha1, &oid);
	git_oid_cpy(&entry->oid, &oid);
	entry->crc = crc32(0L, Z_NULL, 0);

	entry_size = (size_t)(idx->off - entry_start);
	if (crc_object(&entry->crc, &idx->pack->mwf, entry_start, entry_size) < 0)
		goto on_error;

	return save_entry(idx, entry, pentry, entry_start);

on_error:
	git__free(entry);
	git__free(obj->data);
	return -1;
}

static int do_progress_callback(git_indexer *idx, git_transfer_progress *stats)
{
	if (idx->progress_cb)
		return giterr_set_after_callback_function(
			idx->progress_cb(stats, idx->progress_payload),
			QT_TRANSLATE_NOOP("libgit2", "indexer progress"));
	return 0;
}

/* Hash everything but the last 20B of input */
static void hash_partially(git_indexer *idx, const uint8_t *data, size_t size)
{
	size_t to_expell, to_keep;

	if (size == 0)
		return;

	/* Easy case, dump the buffer and the data minus the last 20 bytes */
	if (size >= GIT_OID_RAWSZ) {
		git_hash_update(&idx->trailer, idx->inbuf, idx->inbuf_len);
		git_hash_update(&idx->trailer, data, size - GIT_OID_RAWSZ);

		data += size - GIT_OID_RAWSZ;
		memcpy(idx->inbuf, data, GIT_OID_RAWSZ);
		idx->inbuf_len = GIT_OID_RAWSZ;
		return;
	}

	/* We can just append */
	if (idx->inbuf_len + size <= GIT_OID_RAWSZ) {
		memcpy(idx->inbuf + idx->inbuf_len, data, size);
		idx->inbuf_len += size;
		return;
	}

	/* We need to partially drain the buffer and then append */
	to_keep   = GIT_OID_RAWSZ - size;
	to_expell = idx->inbuf_len - to_keep;

	git_hash_update(&idx->trailer, idx->inbuf, to_expell);

	memmove(idx->inbuf, idx->inbuf + to_expell, to_keep);
	memcpy(idx->inbuf + to_keep, data, size);
	idx->inbuf_len += size - to_expell;
}

int git_indexer_append(git_indexer *idx, const void *data, size_t size, git_transfer_progress *stats)
{
	int error = -1;
	size_t processed;
	struct git_pack_header *hdr = &idx->hdr;
	git_mwindow_file *mwf = &idx->pack->mwf;

	assert(idx && data && stats);

	processed = stats->indexed_objects;

	if ((error = git_filebuf_write(&idx->pack_file, data, size)) < 0)
		return error;

	hash_partially(idx, data, (int)size);

	/* Make sure we set the new size of the pack */
	if (idx->opened_pack) {
		idx->pack->mwf.size += size;
	} else {
		if ((error = open_pack(&idx->pack, idx->pack_file.path_lock)) < 0)
			return error;
		idx->opened_pack = 1;
		mwf = &idx->pack->mwf;
		if ((error = git_mwindow_file_register(&idx->pack->mwf)) < 0)
			return error;
	}

	if (!idx->parsed_header) {
		unsigned int total_objects;

		if ((unsigned)idx->pack->mwf.size < sizeof(struct git_pack_header))
			return 0;

		if ((error = parse_header(&idx->hdr, idx->pack)) < 0)
			return error;

		idx->parsed_header = 1;
		idx->nr_objects = ntohl(hdr->hdr_entries);
		idx->off = sizeof(struct git_pack_header);

		/* for now, limit to 2^32 objects */
		assert(idx->nr_objects == (size_t)((unsigned int)idx->nr_objects));
		if (idx->nr_objects == (size_t)((unsigned int)idx->nr_objects))
			total_objects = (unsigned int)idx->nr_objects;
		else
			total_objects = UINT_MAX;

		idx->pack->idx_cache = git_oidmap_alloc();
		GITERR_CHECK_ALLOC(idx->pack->idx_cache);

		idx->pack->has_cache = 1;
		if (git_vector_init(&idx->objects, total_objects, objects_cmp) < 0)
			return -1;

		if (git_vector_init(&idx->deltas, total_objects / 2, NULL) < 0)
			return -1;

		stats->received_objects = 0;
		stats->local_objects = 0;
		stats->total_deltas = 0;
		stats->indexed_deltas = 0;
		processed = stats->indexed_objects = 0;
		stats->total_objects = total_objects;

		if ((error = do_progress_callback(idx, stats)) != 0)
			return error;
	}

	/* Now that we have data in the pack, let's try to parse it */

	/* As the file grows any windows we try to use will be out of date */
	git_mwindow_free_all(mwf);

	while (processed < idx->nr_objects) {
		git_packfile_stream *stream = &idx->stream;
		git_off_t entry_start = idx->off;
		size_t entry_size;
		git_otype type;
		git_mwindow *w = NULL;

		if (idx->pack->mwf.size <= idx->off + 20)
			return 0;

		if (!idx->have_stream) {
			error = git_packfile_unpack_header(&entry_size, &type, mwf, &w, &idx->off);
			if (error == GIT_EBUFS) {
				idx->off = entry_start;
				return 0;
			}
			if (error < 0)
				goto on_error;

			git_mwindow_close(&w);
			idx->entry_start = entry_start;
			git_hash_ctx_init(&idx->hash_ctx);

			if (type == GIT_OBJ_REF_DELTA || type == GIT_OBJ_OFS_DELTA) {
				error = advance_delta_offset(idx, type);
				if (error == GIT_EBUFS) {
					idx->off = entry_start;
					return 0;
				}
				if (error < 0)
					goto on_error;

				idx->have_delta = 1;
			} else {
				idx->have_delta = 0;
				hash_header(&idx->hash_ctx, entry_size, type);
			}

			idx->have_stream = 1;

			error = git_packfile_stream_open(stream, idx->pack, idx->off);
			if (error < 0)
				goto on_error;
		}

		if (idx->have_delta) {
			error = read_object_stream(idx, stream);
		} else {
			error = hash_object_stream(idx, stream);
		}

		idx->off = stream->curpos;
		if (error == GIT_EBUFS)
			return 0;

		/* We want to free the stream reasorces no matter what here */
		idx->have_stream = 0;
		git_packfile_stream_free(stream);

		if (error < 0)
			goto on_error;

		if (idx->have_delta) {
			error = store_delta(idx);
		} else {
			error = store_object(idx);
		}

		if (error < 0)
			goto on_error;

		if (!idx->have_delta) {
			stats->indexed_objects = (unsigned int)++processed;
		}
		stats->received_objects++;

		if ((error = do_progress_callback(idx, stats)) != 0)
			goto on_error;
	}

	return 0;

on_error:
	git_mwindow_free_all(mwf);
	return error;
}

static int index_path(git_buf *path, git_indexer *idx, const char *suffix)
{
	const char prefix[] = "pack-";
	size_t slash = (size_t)path->size;

	/* search backwards for '/' */
	while (slash > 0 && path->ptr[slash - 1] != '/')
		slash--;

	if (git_buf_grow(path, slash + 1 + strlen(prefix) +
					 GIT_OID_HEXSZ + strlen(suffix) + 1) < 0)
		return -1;

	git_buf_truncate(path, slash);
	git_buf_puts(path, prefix);
	git_oid_fmt(path->ptr + git_buf_len(path), &idx->hash);
	path->size += GIT_OID_HEXSZ;
	git_buf_puts(path, suffix);

	return git_buf_oom(path) ? -1 : 0;
}

/**
 * Rewind the packfile by the trailer, as we might need to fix the
 * packfile by injecting objects at the tail and must overwrite it.
 */
static git_off_t seek_back_trailer(git_indexer *idx)
{
	git_off_t off;

	if ((off = p_lseek(idx->pack_file.fd, -GIT_OID_RAWSZ, SEEK_CUR)) < 0)
		return -1;

	idx->pack->mwf.size -= GIT_OID_RAWSZ;
	git_mwindow_free_all(&idx->pack->mwf);

	return off;
}

static int inject_object(git_indexer *idx, git_oid *id)
{
	git_odb_object *obj;
	struct entry *entry;
	struct git_pack_entry *pentry;
	git_oid foo = {{0}};
	unsigned char hdr[64];
	git_buf buf = GIT_BUF_INIT;
	git_off_t entry_start;
	const void *data;
	size_t len, hdr_len;
	int error;

	entry = git__calloc(1, sizeof(*entry));
	GITERR_CHECK_ALLOC(entry);

	entry_start = seek_back_trailer(idx);

	if (git_odb_read(&obj, idx->odb, id) < 0)
		return -1;

	data = git_odb_object_data(obj);
	len = git_odb_object_size(obj);

	entry->crc = crc32(0L, Z_NULL, 0);

	/* Write out the object header */
	hdr_len = git_packfile__object_header(hdr, len, git_odb_object_type(obj));
	git_filebuf_write(&idx->pack_file, hdr, hdr_len);
	idx->pack->mwf.size += hdr_len;
	entry->crc = crc32(entry->crc, hdr, hdr_len);

	if ((error = git__compress(&buf, data, len)) < 0)
		goto cleanup;

	/* And then the compressed object */
	git_filebuf_write(&idx->pack_file, buf.ptr, buf.size);
	idx->pack->mwf.size += buf.size;
	entry->crc = htonl(crc32(entry->crc, (unsigned char *)buf.ptr, (uInt)buf.size));
	git_buf_free(&buf);

	/* Write a fake trailer so the pack functions play ball */
	if ((error = git_filebuf_write(&idx->pack_file, &foo, GIT_OID_RAWSZ)) < 0)
		goto cleanup;

	idx->pack->mwf.size += GIT_OID_RAWSZ;

	pentry = git__calloc(1, sizeof(struct git_pack_entry));
	GITERR_CHECK_ALLOC(pentry);

	git_oid_cpy(&pentry->sha1, id);
	git_oid_cpy(&entry->oid, id);
	idx->off = entry_start + hdr_len + len;

	if ((error = save_entry(idx, entry, pentry, entry_start)) < 0)
		git__free(pentry);

cleanup:
	git_odb_object_free(obj);
	return error;
}

static int fix_thin_pack(git_indexer *idx, git_transfer_progress *stats)
{
	int error, found_ref_delta = 0;
	unsigned int i;
	struct delta_info *delta;
	size_t size;
	git_otype type;
	git_mwindow *w = NULL;
	git_off_t curpos;
	unsigned char *base_info;
	unsigned int left = 0;
	git_oid base;

	assert(git_vector_length(&idx->deltas) > 0);

	if (idx->odb == NULL) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "cannot fix a thin pack without an ODB"));
		return -1;
	}

	/* Loop until we find the first REF delta */
	git_vector_foreach(&idx->deltas, i, delta) {
		curpos = delta->delta_off;
		error = git_packfile_unpack_header(&size, &type, &idx->pack->mwf, &w, &curpos);
		git_mwindow_close(&w);
		if (error < 0)
			return error;

		if (type == GIT_OBJ_REF_DELTA) {
			found_ref_delta = 1;
			break;
		}
	}

	if (!found_ref_delta) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "no REF_DELTA found, cannot inject object"));
		return -1;
	}

	/* curpos now points to the base information, which is an OID */
	base_info = git_mwindow_open(&idx->pack->mwf, &w, curpos, GIT_OID_RAWSZ, &left);
	if (base_info == NULL) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "failed to map delta information"));
		return -1;
	}

	git_oid_fromraw(&base, base_info);
	git_mwindow_close(&w);

	if (inject_object(idx, &base) < 0)
		return -1;

	stats->local_objects++;

	return 0;
}

static int resolve_deltas(git_indexer *idx, git_transfer_progress *stats)
{
	unsigned int i;
	struct delta_info *delta;
	int progressed = 0, progress_cb_result;

	while (idx->deltas.length > 0) {
		progressed = 0;
		git_vector_foreach(&idx->deltas, i, delta) {
			git_rawobj obj;

			idx->off = delta->delta_off;
			if (git_packfile_unpack(&obj, idx->pack, &idx->off) < 0)
				continue;

			if (hash_and_save(idx, &obj, delta->delta_off) < 0)
				continue;

			git__free(obj.data);
			stats->indexed_objects++;
			stats->indexed_deltas++;
			progressed = 1;
			if ((progress_cb_result = do_progress_callback(idx, stats)) < 0)
				return progress_cb_result;

			/*
			 * Remove this delta from the list and
			 * decrease i so we don't skip over the next
			 * delta.
			 */
			git_vector_remove(&idx->deltas, i);
			git__free(delta);
			i--;
		}

		if (!progressed && (fix_thin_pack(idx, stats) < 0)) {
			giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "missing delta bases"));
			return -1;
		}
	}

	return 0;
}

static int update_header_and_rehash(git_indexer *idx, git_transfer_progress *stats)
{
	void *ptr;
	size_t chunk = 1024*1024;
	git_off_t hashed = 0;
	git_mwindow *w = NULL;
	git_mwindow_file *mwf;
	unsigned int left;
	git_hash_ctx *ctx;

	mwf = &idx->pack->mwf;
	ctx = &idx->trailer;

	git_hash_ctx_init(ctx);
	git_mwindow_free_all(mwf);

	/* Update the header to include the numer of local objects we injected */
	idx->hdr.hdr_entries = htonl(stats->total_objects + stats->local_objects);
	if (p_lseek(idx->pack_file.fd, 0, SEEK_SET) < 0) {
		giterr_set(GITERR_OS, QT_TRANSLATE_NOOP("libgit2", "failed to seek to the beginning of the pack"));
		return -1;
	}

	if (p_write(idx->pack_file.fd, &idx->hdr, sizeof(struct git_pack_header)) < 0) {
		giterr_set(GITERR_OS, QT_TRANSLATE_NOOP("libgit2", "failed to update the pack header"));
		return -1;
	}

	/*
	 * We now use the same technique as before to determine the
	 * hash. We keep reading up to the end and let
	 * hash_partially() keep the existing trailer out of the
	 * calculation.
	 */
	idx->inbuf_len = 0;
	while (hashed < mwf->size) {
		ptr = git_mwindow_open(mwf, &w, hashed, chunk, &left);
		if (ptr == NULL)
			return -1;

		hash_partially(idx, ptr, left);
		hashed += left;

		git_mwindow_close(&w);
	}

	return 0;
}

int git_indexer_commit(git_indexer *idx, git_transfer_progress *stats)
{
	git_mwindow *w = NULL;
	unsigned int i, long_offsets = 0, left;
	int error;
	struct git_pack_idx_header hdr;
	git_buf filename = GIT_BUF_INIT;
	struct entry *entry;
	git_oid trailer_hash, file_hash;
	git_hash_ctx ctx;
	git_filebuf index_file = {0};
	void *packfile_trailer;

	if (git_hash_ctx_init(&ctx) < 0)
		return -1;

	/* Test for this before resolve_deltas(), as it plays with idx->off */
	if (idx->off < idx->pack->mwf.size - 20) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "Unexpected data at the end of the pack"));
		return -1;
	}

	packfile_trailer = git_mwindow_open(&idx->pack->mwf, &w, idx->pack->mwf.size - GIT_OID_RAWSZ, GIT_OID_RAWSZ, &left);
	if (packfile_trailer == NULL) {
		git_mwindow_close(&w);
		goto on_error;
	}

	/* Compare the packfile trailer as it was sent to us and what we calculated */
	git_oid_fromraw(&file_hash, packfile_trailer);
	git_mwindow_close(&w);

	git_hash_final(&trailer_hash, &idx->trailer);
	if (git_oid_cmp(&file_hash, &trailer_hash)) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "packfile trailer mismatch"));
		return -1;
	}

	/* Freeze the number of deltas */
	stats->total_deltas = stats->total_objects - stats->indexed_objects;

	if ((error = resolve_deltas(idx, stats)) < 0)
		return error;

	if (stats->indexed_objects != stats->total_objects) {
		giterr_set(GITERR_INDEXER, QT_TRANSLATE_NOOP("libgit2", "early EOF"));
		return -1;
	}

	if (stats->local_objects > 0) {
		if (update_header_and_rehash(idx, stats) < 0)
			return -1;

		git_hash_final(&trailer_hash, &idx->trailer);
		if (p_lseek(idx->pack_file.fd, -GIT_OID_RAWSZ, SEEK_END) < 0)
			return -1;

		if (p_write(idx->pack_file.fd, &trailer_hash, GIT_OID_RAWSZ) < 0) {
			giterr_set(GITERR_OS, QT_TRANSLATE_NOOP("libgit2", "failed to update pack trailer"));
			return -1;
		}
	}

	git_vector_sort(&idx->objects);

	git_buf_sets(&filename, idx->pack->pack_name);
	git_buf_shorten(&filename, strlen("pack"));
	git_buf_puts(&filename, "idx");
	if (git_buf_oom(&filename))
		return -1;

	if (git_filebuf_open(&index_file, filename.ptr,
		GIT_FILEBUF_HASH_CONTENTS, idx->mode) < 0)
		goto on_error;

	/* Write out the header */
	hdr.idx_signature = htonl(PACK_IDX_SIGNATURE);
	hdr.idx_version = htonl(2);
	git_filebuf_write(&index_file, &hdr, sizeof(hdr));

	/* Write out the fanout table */
	for (i = 0; i < 256; ++i) {
		uint32_t n = htonl(idx->fanout[i]);
		git_filebuf_write(&index_file, &n, sizeof(n));
	}

	/* Write out the object names (SHA-1 hashes) */
	git_vector_foreach(&idx->objects, i, entry) {
		git_filebuf_write(&index_file, &entry->oid, sizeof(git_oid));
		git_hash_update(&ctx, &entry->oid, GIT_OID_RAWSZ);
	}
	git_hash_final(&idx->hash, &ctx);

	/* Write out the CRC32 values */
	git_vector_foreach(&idx->objects, i, entry) {
		git_filebuf_write(&index_file, &entry->crc, sizeof(uint32_t));
	}

	/* Write out the offsets */
	git_vector_foreach(&idx->objects, i, entry) {
		uint32_t n;

		if (entry->offset == UINT32_MAX)
			n = htonl(0x80000000 | long_offsets++);
		else
			n = htonl(entry->offset);

		git_filebuf_write(&index_file, &n, sizeof(uint32_t));
	}

	/* Write out the long offsets */
	git_vector_foreach(&idx->objects, i, entry) {
		uint32_t split[2];

		if (entry->offset != UINT32_MAX)
			continue;

		split[0] = htonl(entry->offset_long >> 32);
		split[1] = htonl(entry->offset_long & 0xffffffff);

		git_filebuf_write(&index_file, &split, sizeof(uint32_t) * 2);
	}

	/* Write out the packfile trailer to the index */
	if (git_filebuf_write(&index_file, &trailer_hash, GIT_OID_RAWSZ) < 0)
		goto on_error;

	/* Write out the hash of the idx */
	if (git_filebuf_hash(&trailer_hash, &index_file) < 0)
		goto on_error;

	git_filebuf_write(&index_file, &trailer_hash, sizeof(git_oid));

	/* Figure out what the final name should be */
	if (index_path(&filename, idx, ".idx") < 0)
		goto on_error;

	/* Commit file */
	if (git_filebuf_commit_at(&index_file, filename.ptr) < 0)
		goto on_error;

	git_mwindow_free_all(&idx->pack->mwf);
	/* We need to close the descriptor here so Windows doesn't choke on commit_at */
	p_close(idx->pack->mwf.fd);
	idx->pack->mwf.fd = -1;

	if (index_path(&filename, idx, ".pack") < 0)
		goto on_error;
	/* And don't forget to rename the packfile to its new place. */
	if (git_filebuf_commit_at(&idx->pack_file, filename.ptr) < 0)
		return -1;

	git_buf_free(&filename);
	return 0;

on_error:
	git_mwindow_free_all(&idx->pack->mwf);
	git_filebuf_cleanup(&index_file);
	git_buf_free(&filename);
	git_hash_ctx_cleanup(&ctx);
	return -1;
}

void git_indexer_free(git_indexer *idx)
{
	if (idx == NULL)
		return;

	git_vector_free_deep(&idx->objects);

	if (idx->pack) {
		struct git_pack_entry *pentry;
		kh_foreach_value(
			idx->pack->idx_cache, pentry, { git__free(pentry); });

		git_oidmap_free(idx->pack->idx_cache);
	}

	git_vector_free_deep(&idx->deltas);
	git_packfile_free(idx->pack);
	git_filebuf_cleanup(&idx->pack_file);
	git__free(idx);
}
