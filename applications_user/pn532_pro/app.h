#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>

#include "pn532_cmd.h"

typedef enum {
    Pn532AppViewSubmenu,
    Pn532AppViewWidget,
} Pn532AppView;

typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    NotificationApp* notification;
    Pn532Dev* dev;
} Pn532App;

