#include "app.h"

enum {
    SubmenuIndexScanTag,
    SubmenuIndexAbout,
};

static void submenu_callback(void* context, uint32_t index) {
    Pn532App* app = context;
    if(index == SubmenuIndexScanTag) {
        widget_reset(app->widget);
        
        pn532_spi_reset(app->dev->spi);
        pn532_sam_config(app->dev);
        furi_delay_ms(20);

        uint8_t uid[10] = {0};
        uint8_t uid_len = 0;

        if(pn532_read_passive_target(app->dev, uid, &uid_len)) {
            char uid_str[64] = "UID:";
            for(uint8_t i = 0; i < uid_len; i++) {
                snprintf(uid_str + strlen(uid_str), sizeof(uid_str) - strlen(uid_str), " %02X", uid[i]);
            }
            
            widget_add_string_element(app->widget, 5, 5, FontPrimary, "Quet Thanh Cong!");
            widget_add_string_element(app->widget, 5, 25, FontSecondary, uid_str);
            notification_message(app->notification, &sequence_success);
        } else {
            widget_add_string_element(app->widget, 5, 5, FontPrimary, "Khong Co The / Loi Hardware!");
            widget_add_string_element(app->widget, 5, 25, FontSecondary, "Kiem tra DIP Switch & Cap SPI");
            notification_message(app->notification, &sequence_error);
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, Pn532AppViewWidget);
    } else if(index == SubmenuIndexAbout) {
        widget_reset(app->widget);
        widget_add_string_element(app->widget, 5, 5, FontPrimary, "PN532 SPI Toolkit");
        widget_add_string_element(app->widget, 5, 25, FontSecondary, "CS: PA4 | RST: PB2");
        widget_add_string_element(app->widget, 5, 40, FontSecondary, "SCK: PA6 | MISO: PA7 | MOSI: PB3");
        view_dispatcher_switch_to_view(app->view_dispatcher, Pn532AppViewWidget);
    }
}

static uint32_t back_event_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static uint32_t widget_back_event_callback(void* context) {
    UNUSED(context);
    return Pn532AppViewSubmenu;
}

int32_t pn532_pro_app(void* p) {
    UNUSED(p);
    Pn532App* app = malloc(sizeof(Pn532App));

    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->dev = pn532_dev_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();

    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Quet The NFC", SubmenuIndexScanTag, submenu_callback, app);
    submenu_add_item(app->submenu, "Thong Tin Hardware", SubmenuIndexAbout, submenu_callback, app);

    app->widget = widget_alloc();

    view_dispatcher_add_view(app->view_dispatcher, Pn532AppViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->view_dispatcher, Pn532AppViewWidget, widget_get_view(app->widget));

    view_set_previous_callback(submenu_get_view(app->submenu), back_event_callback);
    view_set_previous_callback(widget_get_view(app->widget), widget_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, Pn532AppViewSubmenu);

    view_dispatcher_run(app->view_dispatcher);

    view_dispatcher_remove_view(app->view_dispatcher, Pn532AppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, Pn532AppViewWidget);

    submenu_free(app->submenu);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);

    pn532_dev_free(app->dev);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
    return 0;
}
