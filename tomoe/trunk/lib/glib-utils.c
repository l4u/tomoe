/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <glib.h>
#include "glib-utils.h"

void
g_ptr_array_foreach_reverse (GPtrArray *array,
                             GFunc      func,
                             gpointer   user_data)
{
    gint i;

    i = array->len - 1;
    while (i >= 0) {
        func (g_ptr_array_index (array, i--), user_data);
    }
}

#if !GLIB_CHECK_VERSION(2, 8, 10)
/* copied from GLib. */
gboolean
g_file_set_contents (const gchar *filename,
		     const gchar *contents,
		     gssize	     length,
		     GError	   **error)
{
  gchar *tmp_filename;
  gboolean retval;
  GError *rename_error = NULL;
  
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (contents != NULL || length == 0, FALSE);
  g_return_val_if_fail (length >= -1, FALSE);
  
  if (length == -1)
    length = strlen (contents);

  tmp_filename = write_to_temp_file (contents, length, filename, error);
  
  if (!tmp_filename)
    {
      retval = FALSE;
      goto out;
    }

  if (!rename_file (tmp_filename, filename, &rename_error))
    {
#ifndef G_OS_WIN32

      g_unlink (tmp_filename);
      g_propagate_error (error, rename_error);
      retval = FALSE;
      goto out;

#else /* G_OS_WIN32 */
      
      /* Renaming failed, but on Windows this may just mean
       * the file already exists. So if the target file
       * exists, try deleting it and do the rename again.
       */
      if (!g_file_test (filename, G_FILE_TEST_EXISTS))
	{
	  g_unlink (tmp_filename);
	  g_propagate_error (error, rename_error);
	  retval = FALSE;
	  goto out;
	}

      g_error_free (rename_error);
      
      if (g_unlink (filename) == -1)
	{
          gchar *display_filename = g_filename_display_name (filename);

	  int save_errno = errno;
	  
	  g_set_error (error,
		       G_FILE_ERROR,
		       g_file_error_from_errno (save_errno),
		       _("Existing file '%s' could not be removed: g_unlink() failed: %s"),
		       display_filename,
		       g_strerror (save_errno));

	  g_free (display_filename);
	  g_unlink (tmp_filename);
	  retval = FALSE;
	  goto out;
	}
      
      if (!rename_file (tmp_filename, filename, error))
	{
	  g_unlink (tmp_filename);
	  retval = FALSE;
	  goto out;
	}

#endif
    }

  retval = TRUE;
  
 out:
  g_free (tmp_filename);
  return retval;
}
#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
