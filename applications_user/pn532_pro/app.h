#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include "pn532_cmd.h"

typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    NotificationApp* notification;
    Pn532Dev* dev;
} Pn532App;

typedef enum {
    Pn532AppViewSubmenu,
    Pn532AppViewWidget,
} Pn532AppView;
