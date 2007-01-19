/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "menu.h"

#include <ctype.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkradiomenuitem.h>
#include <gtk/gtktearoffmenuitem.h>
#include <gtk/gtkaccellabel.h>

#include "generic/callback.h"

#include "closure.h"
#include "container.h"
#include "pointer.h"

void menu_add_item(GtkMenu* menu, GtkMenuItem* item)
{
	gtk_container_add(GTK_CONTAINER(menu), GTK_WIDGET(item));
}

GtkMenuItem* menu_separator(GtkMenu* menu)
{
  GtkMenuItem* menu_item = GTK_MENU_ITEM(gtk_menu_item_new());
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(menu_item));
  gtk_widget_set_sensitive(GTK_WIDGET(menu_item), FALSE);
  gtk_widget_show(GTK_WIDGET(menu_item));
  return menu_item;
}

GtkTearoffMenuItem* menu_tearoff(GtkMenu* menu)
{
  GtkTearoffMenuItem* menu_item = GTK_TEAROFF_MENU_ITEM(gtk_tearoff_menu_item_new());
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(menu_item));
// gtk_widget_set_sensitive(GTK_WIDGET(menu_item), FALSE); -- controls whether menu is detachable
  gtk_widget_show(GTK_WIDGET(menu_item));
  return menu_item;
}

GtkMenuItem* new_sub_menu_item_with_mnemonic(const char* mnemonic)
{
  GtkMenuItem* item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(mnemonic));
  gtk_widget_show(GTK_WIDGET(item));

  GtkWidget* sub_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(item, sub_menu);

  return item;
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenuShell* parent, const char* mnemonic)
{
  GtkMenuItem* item = new_sub_menu_item_with_mnemonic(mnemonic);
  container_add_widget(GTK_CONTAINER(parent), GTK_WIDGET(item));
  return GTK_MENU(gtk_menu_item_get_submenu(item));
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenuBar* bar, const char* mnemonic)
{
  return create_sub_menu_with_mnemonic(GTK_MENU_SHELL(bar), mnemonic);
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenu* parent, const char* mnemonic)
{
  return create_sub_menu_with_mnemonic(GTK_MENU_SHELL(parent), mnemonic);
}

void activate_closure_callback(GtkWidget* widget, gpointer data)
{
  (*reinterpret_cast<Callback*>(data))();
}

guint check_menu_item_connect_callback(GtkCheckMenuItem* item, const Callback& callback)
{
#if 1
  guint handler = g_signal_connect_swapped(G_OBJECT(item), "toggled", G_CALLBACK(callback.getThunk()), callback.getEnvironment());
#else
  guint handler = g_signal_connect_closure(G_OBJECT(item), "toggled", create_cclosure(G_CALLBACK(activate_closure_callback), callback), TRUE);
#endif
  g_object_set_data(G_OBJECT(item), "handler", gint_to_pointer(handler));
  return handler;
}

GtkRadioMenuItem* new_radio_menu_item_with_mnemonic(GSList** group, const char* mnemonic, const Callback& callback)
{
  GtkRadioMenuItem* item = GTK_RADIO_MENU_ITEM(gtk_radio_menu_item_new_with_mnemonic(*group, mnemonic));
  if(*group == 0)
  {
    gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(item), TRUE);
  }
  *group = gtk_radio_menu_item_group(item);
  gtk_widget_show(GTK_WIDGET(item));
  check_menu_item_connect_callback(GTK_CHECK_MENU_ITEM(item), callback);
  return item;
}

GtkRadioMenuItem* create_radio_menu_item_with_mnemonic(GtkMenu* menu, GSList** group, const char* mnemonic, const Callback& callback)
{
  GtkRadioMenuItem* item = new_radio_menu_item_with_mnemonic(group, mnemonic, callback);
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(item));
  return item;
}

void check_menu_item_set_active_no_signal(GtkCheckMenuItem* item, gboolean active)
{
  guint handler_id = gpointer_to_int(g_object_get_data(G_OBJECT(item), "handler"));
  g_signal_handler_block(G_OBJECT(item), handler_id);
  gtk_check_menu_item_set_active(item, active);
  g_signal_handler_unblock(G_OBJECT(item), handler_id);
}



void radio_menu_item_set_active_no_signal(GtkRadioMenuItem* item, gboolean active)
{
  {
    for(GSList* l = gtk_radio_menu_item_get_group(item); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_block(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), active);
  {
    for(GSList* l = gtk_radio_menu_item_get_group(item); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_unblock(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
}

void check_menu_item_set_active_callback(GtkCheckMenuItem& item, bool enabled)
{
  check_menu_item_set_active_no_signal(&item, enabled);
}
typedef ReferenceCaller1<GtkCheckMenuItem, bool, check_menu_item_set_active_callback> CheckMenuItemSetActiveCaller;

