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

#define CHAR_UUID "0xffe9"

static bool connected = false;
static GAttrib *attrib = NULL;
static GIOChannel *chan = NULL;
static char *address = NULL;
static char *handle = NULL;

static GOptionEntry _params[] =
{
        { "target", 't', 0, G_OPTION_ARG_STRING, &address, "MAC address of the target device", NULL },
        { "handle", 'h', 0, G_OPTION_ARG_STRING, &handle, "Value handle", NULL },
        { NULL }
};

static GMainLoop *_event_loop;

static void exit_app(int __attribute__((unused)) (signo))
{
    g_message("Exitting app...");
    g_main_loop_quit(_event_loop);
}

static int strtohandle(const char *src)
{
    char *e;
    int dst;

    errno = 0;
    dst = strtoll(src, &e, 16);
    if (errno != 0 || *e != '\0')
        return -EINVAL;

    return dst;
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
        gatt_write_char(attrib, strtohandle(handle), value, plen, NULL, NULL);
    }
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
    if (err)
    {
        g_message("%s", err->message);
        g_main_loop_quit(_event_loop);
        connected = false;
        return;
    }
    connected = true;
    g_message("Successfully connected to BTLE device");
    attrib = g_attrib_new(io);

    char *color;
    awa_get_current_color(&color);
    set_bulb_color(color);
}


static void color_changed_cb(const char *color)
{
    set_bulb_color(color);
}

int main(int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

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
    if (handle == NULL)
    {
        g_error("You must specify value handle");
        return 1;
    }
    if (strtohandle(handle) == EINVAL)
    {
        g_error("Invalid handle");
        return 1;
    }

    g_option_context_free(context);

    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,  g_log_default_handler, NULL);

    struct sigaction action =
    { .sa_handler = exit_app, .sa_flags = 0 };

    if (sigemptyset(&action.sa_mask) < 0 || sigaction(SIGINT, &action, NULL) < 0)
    {
        g_error("Failed to set Control+C handler\n");
        return 1;
    }

    AwaClientSession *session = awa_setup(color_changed_cb, &error);

    if (error)
    {
        g_error("%s", error->message);
        return 1;
    }

    chan = gatt_connect("hci0", address, "", "low", 0, 0, connect_cb, &error);

    if (chan == NULL)
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
