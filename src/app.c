/*
 *
 *  EW17 Bulb Controller
 *
 *  Copyright (C) 2017  Imagination Technologies Limited
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <awa/client.h>
#include <awa/common.h>
#include "libgatt/gatt.h"
#include "libgatt/bluetooth.h"
#include "libgatt/gattrib.h"
#include "colors.h"
#include "awa.h"

#define CHAR_UUID "0000ffe9-0000-1000-8000-00805f9b34fb"

static bool connected = false;
static GAttrib *attrib = NULL;
static GIOChannel *chan = NULL;
static char *address = NULL;
static int ipcPort = 12345;
static guint16 value_handle = 0;

static GOptionEntry _params[] =
{
        { "target", 't', 0, G_OPTION_ARG_STRING, &address, "MAC address of the target device", NULL },
        { "ipcPort", 'i', 0, G_OPTION_ARG_INT, &ipcPort, "IPC address of awa client", NULL },
        { NULL }
};

static GMainLoop *_event_loop;

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data);
static gboolean channel_watcher(GIOChannel *chan, GIOCondition cond, gpointer user_data);


static void exit_app(int __attribute__((unused)) (signo))
{
    g_message("Exitting app...");
    g_main_loop_quit(_event_loop);
}

static void set_bulb_color(const char *color)
{
    uint8_t *value = NULL;
    char *colorValue = NULL;

    for (int i = 0; i < 5; i++)
    {
        if (strcmp(colors[i].colorName, color) == 0)
        {
            colorValue = colors[i].colorValue;
            break;
        }
    }

    if (colorValue != NULL)
    {
        size_t plen = gatt_attr_data_from_string(colorValue, &value);
        if (plen == 0)
        {
            return;
        }
        gatt_write_char(attrib, value_handle, value, plen, NULL, NULL);
    }
}

static void disconnect_io()
{
    if (connected == false)
        return;

    g_attrib_unref(attrib);
    attrib = NULL;


    g_io_channel_shutdown(chan, FALSE, NULL);
    g_io_channel_unref(chan);
    chan = NULL;

    connected = false;
}

static void try_connect_to_bulb(GError** error)
{
    g_message("Attempting to connect to the bulb...");
    chan = gatt_connect("hci0", address, "", "low", 0, 0, connect_cb, error);
    if (chan == NULL)
    {
        return;
    }
    g_io_add_watch(chan, G_IO_HUP, channel_watcher, NULL);

}

static gboolean channel_watcher(GIOChannel *chan, GIOCondition cond, gpointer user_data)
{
    g_printerr("Bulb disconnected...");
    disconnect_io();
    GError *error = NULL;
    try_connect_to_bulb(&error);
    if (error)
    {
        g_error("%s", error->message);
        free(error);
    }
    return FALSE;
}

static void char_cb(GSList *characteristics, uint8_t status, void *user_data)
{
    if (status) {
        g_error("Discover all characteristics failed: %s", att_ecode2str(status));
        return;
    }
    g_message("Successfully found bulb control characteristic");
    struct gatt_char *characteristic = characteristics->data;
    value_handle = characteristic->value_handle;

    char *color;
    awa_get_current_color(&color);
    set_bulb_color(color);
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
    if (err)
    {
        g_message("%s", err->message);
        connected = false;
        GError *error = NULL;
        try_connect_to_bulb(&error);
        if (error)
        {
            g_error("%s", error->message);
            g_main_loop_quit(_event_loop);
        }
        return;
    }
    connected = true;
    g_message("Successfully connected to BTLE device");
    attrib = g_attrib_new(io);

    bt_uuid_t uuid;
    if (bt_string_to_uuid(&uuid, CHAR_UUID) < 0)
    {
        g_error("Invalid UUID\n");
        return;
    }
    g_message("Attempting to get bulb control characteristic...");
    gatt_discover_char(attrib, 0x0001, 0xffff, &uuid, char_cb, NULL);

}


static void color_changed_cb(const char *color)
{
    if (connected && value_handle > 0)
    {
        set_bulb_color(color);
    }
}

int main(int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    g_message("Starting Bulb Controller app");
    context = g_option_context_new("Bulb Controller Application");
    g_option_context_add_main_entries(context, _params, NULL);

    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_error("option parsing failed: %s\n", error->message);
        exit(1);
    }

    if (address == NULL)
    {
        g_error("You must specify device MAC address");
        return 1;
    }
    g_message("Bulb MAC address: %s", address);
    g_message("Awa IPCPort %d", ipcPort);

    g_option_context_free(context);

    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,  g_log_default_handler, NULL);

    struct sigaction action =
    { .sa_handler = exit_app, .sa_flags = 0 };

    if (sigemptyset(&action.sa_mask) < 0 || sigaction(SIGINT, &action, NULL) < 0)
    {
        g_error("Failed to set Control+C handler\n");
        return 1;
    }

    AwaClientSession *session = awa_setup(color_changed_cb, ipcPort, &error);

    if (error)
    {
        g_error("%s", error->message);
        return 1;
    }

    try_connect_to_bulb(&error);
    if (error)
    {
        g_error("%s", error->message);
        return 1;
    }

    _event_loop = g_main_loop_new(NULL, FALSE);


    g_timeout_add(1000, awa_process, (gpointer) session);

    g_main_loop_run(_event_loop);

    g_main_loop_unref(_event_loop);

    awa_cleanup();

    return 0;
}
