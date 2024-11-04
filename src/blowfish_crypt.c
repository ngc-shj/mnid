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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defines.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/blowfish.h>

#include "blowfish_crypt.h"
#include "blowfish_key.h"
#include "blowfish.h"
#include "utils.h"

static gint blowfish_op_decrypt		(const gchar	*chiper,
					 size_t		 size,
					 gchar		*plain);

gboolean blowfish_encrypt(FILE *fp_cipher, const gchar *plain_data, size_t size)
{
	BF_KEY *bf_key;
	guchar ivec[BF_BLOCK];
	const gchar *key;
	gchar *cipher;
	gchar *plain;
	size_t b_size;

	debug_print("blowfish_encrypt() start...\n");

	if(!(key = blowfish_key_get()))
		return FALSE;

	bf_key = g_new0(BF_KEY, 1);
	BF_set_key(bf_key, strlen(key), key);
	key = NULL;

	memset(ivec, 0, BF_BLOCK);

	b_size = (size + (BF_BLOCK - 1)) >> 3 << 3;

	plain = g_malloc0(b_size);
	g_memmove(plain, plain_data, size);

	cipher = g_malloc0(b_size);
	
	/* ivec initialize zeros */
	BF_cbc_encrypt(plain, cipher, b_size, bf_key, ivec, BF_ENCRYPT);

	g_free(plain);
	memset(ivec, 0, BF_BLOCK);
	memset(bf_key, 0, sizeof(BF_KEY));
	g_free(bf_key);

	if(fwrite(cipher, b_size, 1, fp_cipher) != 1){
		FILE_OP_ERROR(__FUNCTION__, "fwrite");
		g_free(cipher);
		return FALSE;
	}
	g_free(cipher);

	debug_print("%s() done.\n", __FUNCTION__);
	return TRUE;
}

gboolean blowfish_decrypt(FILE *fp_cipher, gchar **plain_data, size_t *size)
{
	gchar *cipher;
	gchar *plain;
	gsize f_size;
	gint ret_val;

	g_return_val_if_fail(fp_cipher != NULL, FALSE);

	*plain_data = NULL;
	*size = 0;

	/* fp_cipher ---> mem */
	if(!file_get_contents(fp_cipher, &cipher, &f_size))
		return FALSE;

	if(!f_size || (f_size & (BF_BLOCK -1))){
		debug_print("%s() failed: file length error.\n", __FUNCTION__);
		g_free(cipher);
		return FALSE;
	}
	plain = g_malloc0(f_size);

	while(!(ret_val = blowfish_op_decrypt(cipher, f_size, plain))){
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						_("Incorrect key"));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	g_free(cipher);

	if(ret_val == -1){
		debug_print("%s() failed:\n", __FUNCTION__);
		g_free(plain);
		return FALSE;
	}

	*plain_data = plain;
	*size = f_size;
	
	debug_print("%s() done.\n", __FUNCTION__);
	
	return TRUE;
}

static gint blowfish_op_decrypt(const gchar *cipher, size_t size, gchar *plain)
{
	static int retry_cnt = 0;
	BF_KEY *bf_key;
	const gchar *key;
	guchar ivec[BF_BLOCK];
	gint ret_val = -1;
	GtkResponseType response_id;

	response_id = blowfish_key_open();

	switch(response_id){
	case GTK_RESPONSE_OK:
		if(!(key = blowfish_key_get())){
			ret_val = -1;
			break;
		}

		/* initialize */
		bf_key = g_new0(BF_KEY, 1);
		memset(ivec, 0, BF_BLOCK);

		BF_set_key(bf_key, strlen(key), key);
		BF_cbc_encrypt(cipher, plain, size, bf_key, ivec, BF_DECRYPT);
		if(plain && !strncmp(plain, "<?xml ", 6))
			ret_val = 1;
		else
			ret_val = 0;

		/* initialize */
		key = NULL;
		memset(bf_key, 0, sizeof(BF_KEY));
		g_free(bf_key);
		memset(ivec, 0, BF_BLOCK);
		break;
	case GTK_RESPONSE_CANCEL:
		ret_val = -1;
		break;
	default:
		ret_val = -1;
		break;
	}

	if(++retry_cnt > 2)
		ret_val = -1;

	return ret_val;
}
