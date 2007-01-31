/*
 * This file is part of telepathy-idle
 * 
 * Copyright (C) 2006 Nokia Corporation. All rights reserved.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License 
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "idle.h"

#include "idle-connection-manager.h"

#include <stdlib.h>
#include <stdio.h>

#include <telepathy-glib/errors.h>

GSource *timeout = NULL;
GMainLoop *mainloop = NULL;
IdleConnectionManager *manager = NULL;
gboolean connections_exist = FALSE;
guint timeout_id;

#define DIE_TIME 5000

static gboolean
kill_connection_manager (gpointer data)
{
  if (!g_getenv ("IDLE_PERSIST") && !connections_exist)
    {
      g_debug("no connections, and timed out");
      g_object_unref (manager);
      g_main_loop_quit (mainloop);
    }

  return FALSE;
}

static void
new_connection (IdleConnectionManager *conn, gchar *bus_name,
                gchar *object_path, gchar *proto)
{
  	g_debug("%s called with %s, %s, %s", G_STRFUNC, bus_name, object_path, proto);
  connections_exist = TRUE;
  g_source_remove (timeout_id);
}

static void
no_more_connections (IdleConnectionManager *conn)
{
  if (g_main_context_find_source_by_id (g_main_loop_get_context (mainloop),
                                       timeout_id))
    {
      g_source_remove (timeout_id);
    }
  connections_exist = FALSE;
  timeout_id = g_timeout_add(DIE_TIME, kill_connection_manager, NULL);
}

#if 0
static void gabble_critical_handler (const gchar *log_domain,
                              GLogLevelFlags log_level,
                              const gchar *message,
                              gpointer user_data)
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  fprintf (stderr, "%s: %s::%s, level %d\n", G_STRFUNC, log_domain, message, log_level);

  fprintf(stderr, "\n########## Backtrace ##########\n");
  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);

  fprintf (stderr, "Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     fprintf (stderr, "%s\n", strings[i]);

  free (strings);
  abort();
}

static void signal_handler_helper(int signum)
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  fprintf (stderr, "%s: %d", G_STRFUNC, signum);

  fprintf(stderr, "\n########## Backtrace ##########\n");
  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);

  fprintf (stderr, "Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     fprintf (stderr, "%s\n", strings[i]);

  free (strings);

  exit(1);
}
#endif

#include <signal.h>

int main(int argc, char **argv) {

  g_type_init();
/*
  {*/
    /*GLogLevelFlags fatal_mask;

    fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
    fatal_mask |= G_LOG_LEVEL_CRITICAL;
    g_log_set_always_fatal (fatal_mask);*/

      /* useful backtrace handler from gabble */
	  /*
    g_log_set_handler ("GLib-GObject", G_LOG_LEVEL_CRITICAL| G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL
                     | G_LOG_FLAG_RECURSION, gabble_critical_handler, NULL);
    g_log_set_handler ("GLib", G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL
                     | G_LOG_FLAG_RECURSION, gabble_critical_handler, NULL);
    g_log_set_handler (NULL, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL
                     | G_LOG_FLAG_RECURSION, gabble_critical_handler, NULL);
  }
*/
  /*
  signal(SIGHUP, signal_handler_helper);
  signal(SIGSEGV, signal_handler_helper);
  signal(SIGTERM, signal_handler_helper);
  signal(SIGPIPE, signal_handler_helper);
  signal(SIGABRT, signal_handler_helper);
  signal(SIGINT, signal_handler_helper);
*/
  
  g_set_prgname("telepathy-idle");
#if 0
  if (!g_thread_supported())
  {
	  g_thread_init(NULL);
  }
#endif

  mainloop = g_main_loop_new (NULL, FALSE);

  dbus_g_error_domain_register (TP_ERRORS, "org.freedesktop.Telepathy.Error", TP_TYPE_ERROR);

  manager = g_object_new (IDLE_TYPE_CONNECTION_MANAGER, NULL);

  g_signal_connect (manager, "new-connection",
                    (GCallback) new_connection, NULL);

  g_signal_connect (manager, "no-more-connections",
                    (GCallback) no_more_connections, NULL);

  _idle_connection_manager_register (manager);

  g_debug ("started");

  timeout_id = g_timeout_add(DIE_TIME, kill_connection_manager, NULL);

  g_main_loop_run (mainloop);

  return 0;
}
