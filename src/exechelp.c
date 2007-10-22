/* exechelp.c - fork and exec helpers
 * Copyright (C) 2004, 2007 g10 Code GmbH
 *
 * This file is part of GpgEX.
 *
 * GpgEX is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GpgEX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <windows.h>

#include <gpg-error.h>

#include "debug.h"
#include "exechelp.h"

/* Define to 1 do enable debugging.  */
#define DEBUG_W32_SPAWN 0


/* Fork and exec the program with /dev/null as stdin, stdout and
   stderr.  Returns 0 on success or an error code.  */
gpg_error_t
gpgex_spawn_detached (const char *cmdline)
{
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi = 
    {
      NULL,      /* Returns process handle.  */
      0,         /* Returns primary thread handle.  */
      0,         /* Returns pid.  */
      0          /* Returns tid.  */
    };
  STARTUPINFO si;
  int cr_flags;

  TRACE_BEG1 (DEBUG_ASSUAN, "gpgex_spawn_detached", cmdline,
	      "cmdline=%s", cmdline);

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr);
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;
  
  /* Start the process.  Note that we can't run the PREEXEC function
     because this would change our own environment. */
  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = DEBUG_W32_SPAWN ? SW_SHOW : SW_MINIMIZE;

  cr_flags = (CREATE_DEFAULT_ERROR_MODE
              | GetPriorityClass (GetCurrentProcess ())
	      | CREATE_NEW_PROCESS_GROUP
              | DETACHED_PROCESS);

  if (!CreateProcess (NULL,          /* pgmname; Program to start.  */
                      (char *) cmdline, /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      cr_flags,      /* Creation flags.  */
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                      ))
    {
      (void) TRACE_LOG1 ("CreateProcess failed: %i\n", GetLastError ());
      return gpg_error (GPG_ERR_GENERAL);
    }
  
  /* Process has been created suspended; resume it now. */
  CloseHandle (pi.hThread); 
  CloseHandle (pi.hProcess);

  return 0;
}
