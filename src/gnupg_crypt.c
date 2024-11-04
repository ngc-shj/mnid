/*
 * mnid
 *
 * Copyright(C) 2005 NOGUCHI Shoji
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * We have copied a part of code from the following other programs.
 *
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2001 Werner Koch (dd9jn)
 *   o rfc2015.c
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if USE_GPGME

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gpgme.h>
#include <stdio.h>
#include <errno.h>

#include "gnupg_crypt.h"
#include "gnupg_pass.h"
#include "utils.h"

static gpgme_key_t pgp_get_key(const gchar *pattern);
static gpgme_data_t pgp_encrypt(gpgme_data_t plain, gpgme_key_t kset[]);
static gpgme_data_t pgp_decrypt(FILE *fp_cipher);

gboolean gnupg_encrypt(FILE *fp_cipher, const gchar *plain_data, size_t size,
			 const gchar *pattern)
{
	gpgme_data_t plain = NULL;
	gpgme_data_t cipher = NULL;
	gpgme_key_t kset[2];
	gpgme_error_t err;
	gchar *str = NULL;
	size_t f_size;

	debug_print("gnupg_encrypt() started...\n");

	kset[0] = pgp_get_key(pattern);
	if(!kset[0]){
		debug_print("%% keyid not found: %s\n", pattern);
		goto leave;
	}
	kset[1] = NULL;
	
	/* plain_data(plain text) -> gpgme_data_t(plain) */
	err = gpgme_data_new_from_mem(&plain, plain_data, size, 1);
	if(err){
		debug_print("%% gpgme_data_new_from_mem failed: %s\n",
				gpgme_strerror(err));
		goto leave;
	}

	cipher = pgp_encrypt(plain, kset);
	gpgme_data_release(plain);	plain = NULL;
	gpgme_key_unref(kset[0]);	kset[0] = NULL;

	if(!cipher)
		goto leave;

	/* gpgme_data_t(cipher) -> fp_cipher */
	str = gpgme_data_release_and_get_mem(cipher, &f_size);
	fwrite(str, f_size, 1, fp_cipher);
	g_free(str);

	debug_print("gnupg_encrypt() done.\n");
	return TRUE;

leave:
	gpgme_key_unref(kset[0]);
	gpgme_data_release(cipher);
	gpgme_data_release(plain);
	debug_print("gnupg_encrypt() failed.\n");

	return FALSE;
}

static gpgme_key_t pgp_get_key(const gchar *pattern)
{
	gpgme_ctx_t ctx = NULL;
	gpgme_key_t key = NULL;
	gpgme_error_t err;

	err = gpgme_new(&ctx);
	if(!err)
		err = gpgme_op_keylist_start(ctx, pattern, 0);
	if(!err){
		err = gpgme_op_keylist_next(ctx, &key);
		gpgme_op_keylist_end(ctx);
	}
	
	if(err){
		debug_print("%% gpgme_op_keylist_* failed: %s\n",
				gpgme_strerror(err));
		gpgme_key_unref(key);
		key = NULL;
	}

	gpgme_release(ctx);

	return key;
}

static gpgme_data_t pgp_encrypt(gpgme_data_t plain, gpgme_key_t kset[])
{
	gpgme_ctx_t ctx = NULL;
	gpgme_error_t err;
	gpgme_data_t cipher = NULL;

	/* encryption  - gpgme_data_t(plain) */
	err = gpgme_new(&ctx);
	if(!err)
		err = gpgme_data_new(&cipher);
	if(!err){
		/* output ASCII Armor */
		gpgme_set_armor(ctx, 1);
		err = (gpgme_data_seek(plain, 0, SEEK_SET) == -1)
			? gpgme_error_from_errno(errno)
			: 0;
		if(!err){
			err = gpgme_op_encrypt(ctx, kset,
					       GPGME_ENCRYPT_ALWAYS_TRUST,
					       plain, cipher);
		}
	}

	if(err){
		debug_print("%% encryption failed: %s\n", gpgme_strerror(err));
		gpgme_data_release(cipher);
		cipher = NULL;
	}else
		debug_print("%% encryption succeeded\n");

	gpgme_release(ctx);
	return cipher;
}

gboolean gnupg_decrypt(FILE *fp_cipher, gchar **plain_data, size_t *size)
{
	gpgme_data_t plain = NULL;

	g_return_val_if_fail(fp_cipher != NULL, FALSE);

	plain = pgp_decrypt(fp_cipher);
	if(!plain){
		gpgme_data_release(plain);
		*plain_data = NULL;
		*size = 0;
		
		debug_print("%s() failed.\n", __FUNCTION__);

		return FALSE;
	}
	*plain_data = gpgme_data_release_and_get_mem(plain, size);

	return TRUE;
}

static gpgme_data_t pgp_decrypt(FILE *fp_cipher)
{
	gpgme_ctx_t ctx = NULL;
	gpgme_error_t err;
	gpgme_data_t plain = NULL;
	gpgme_data_t cipher = NULL;

	g_return_val_if_fail(fp_cipher != NULL, FALSE);

	err = gpgme_new(&ctx);
	if(!err)
		err = gpgme_data_new_from_stream(&cipher, fp_cipher);
	if(!err){
		err = gpgme_data_new(&plain);
		if(!err){
			gpgme_set_passphrase_cb(ctx, gtk_gpgme_passphrase_cb,
						NULL);
			/* decrypt */
			err = gpgme_op_decrypt(ctx, cipher, plain);
		}
	}
	gpgme_data_release(cipher);

	if(err){
		debug_print("%% decryption failed: %s\n", gpgme_strerror(err));
		gpgme_data_release(plain);
		plain = NULL;
	}else
		debug_print("%% decryption succeeded\n");

	gpgme_release(ctx);
	return plain;
}
#endif /* USE_GPGME */
