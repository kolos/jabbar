#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define JABBAR_TYPE_MANAGER (jabbar_manager_get_type())

G_DECLARE_FINAL_TYPE(JabBarManager, jabbar_manager, JABBAR, MANAGER, GObject)

JabBarManager *jabbar_manager_new(void);
void jabbar_manager_start(JabBarManager *self);
void jabbar_manager_stop(JabBarManager *self);

G_END_DECLS
