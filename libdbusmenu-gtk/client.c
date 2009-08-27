/*
A library to take the object model made consistent by libdbusmenu-glib
and visualize it in GTK.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "client.h"
#include "menuitem.h"

/* Prototypes */
static void dbusmenu_gtkclient_class_init (DbusmenuGtkClientClass *klass);
static void dbusmenu_gtkclient_init       (DbusmenuGtkClient *self);
static void dbusmenu_gtkclient_dispose    (GObject *object);
static void dbusmenu_gtkclient_finalize   (GObject *object);
static void new_menuitem (DbusmenuClient * client, DbusmenuMenuitem * mi, gpointer userdata);
static void new_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, DbusmenuGtkClient * gtkclient);
static void delete_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, DbusmenuGtkClient * gtkclient);
static void move_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint new, guint old, DbusmenuGtkClient * gtkclient);

static gboolean new_item_normal     (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);
static gboolean new_item_seperator  (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);
static gboolean new_item_image      (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);

static void process_visible (GtkMenuItem * gmi, const gchar * value);
static void process_sensitive (GtkMenuItem * gmi, const gchar * value);

/* GObject Stuff */
G_DEFINE_TYPE (DbusmenuGtkClient, dbusmenu_gtkclient, DBUSMENU_TYPE_CLIENT);

static void
dbusmenu_gtkclient_class_init (DbusmenuGtkClientClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = dbusmenu_gtkclient_dispose;
	object_class->finalize = dbusmenu_gtkclient_finalize;

	return;
}

static void
dbusmenu_gtkclient_init (DbusmenuGtkClient *self)
{
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(self), DBUSMENU_CLIENT_TYPES_DEFAULT,   new_item_normal);
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(self), DBUSMENU_CLIENT_TYPES_SEPARATOR, new_item_seperator);
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(self), DBUSMENU_CLIENT_TYPES_IMAGE,     new_item_image);

	g_signal_connect(G_OBJECT(self), DBUSMENU_CLIENT_SIGNAL_NEW_MENUITEM, G_CALLBACK(new_menuitem), NULL);

	return;
}

static void
dbusmenu_gtkclient_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtkclient_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->finalize (object);
	return;
}

/* Internal Functions */

static const gchar * data_menuitem = "dbusmenugtk-data-gtkmenuitem";
static const gchar * data_menu =     "dbusmenugtk-data-gtkmenu";

/* This is the call back for the GTK widget for when it gets
   clicked on by the user to send it back across the bus. */
static gboolean
menu_pressed_cb (GtkMenuItem * gmi, DbusmenuMenuitem * mi)
{
	dbusmenu_menuitem_activate(mi);
	return TRUE;
}

/* Process the visible property */
static void
process_visible (GtkMenuItem * gmi, const gchar * value)
{
	if (value == NULL || !g_strcmp0(value, "true")) {
		gtk_widget_show(GTK_WIDGET(gmi));
	} else {
		gtk_widget_hide(GTK_WIDGET(gmi));
	}
	return;
}

/* Process the sensitive property */
static void
process_sensitive (GtkMenuItem * gmi, const gchar * value)
{
	if (value == NULL || !g_strcmp0(value, "true")) {
		gtk_widget_set_sensitive(GTK_WIDGET(gmi), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(gmi), FALSE);
	}
	return;
}

/* Whenever we have a property change on a DbusmenuMenuitem
   we need to be responsive to that. */
static void
menu_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, gchar * value, GtkMenuItem * gmi)
{
	if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_LABEL)) {
		gtk_menu_item_set_label(gmi, value);
	} else if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_VISIBLE)) {
		process_visible(gmi, value);
	} else if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_SENSITIVE)) {
		process_sensitive(gmi, value);
	}

	return;
}

/* Call back that happens when the DbusmenuMenuitem
   is destroyed.  We're making sure to clean up everything
   else down the pipe. */
static void
destoryed_dbusmenuitem_cb (gpointer udata, GObject * dbusmenuitem)
{
	/* g_debug("DbusmenuMenuitem was destroyed"); */
	gtk_widget_destroy(GTK_WIDGET(udata));
	return;
}

/* This takes a new DbusmenuMenuitem and attaches the
   various things that we need to make it work in a 
   GTK World.  */
static void
new_menuitem (DbusmenuClient * client, DbusmenuMenuitem * mi, gpointer userdata)
{
	g_warning("Got new menuitem signal, which means they want something");
	g_warning("  that I simply don't have.");

	return;
}

void
dbusmenu_gtkclient_newitem_base (DbusmenuGtkClient * client, DbusmenuMenuitem * item, GtkMenuItem * gmi, DbusmenuMenuitem * parent)
{
	/* Attach these two */
	g_object_set_data(G_OBJECT(item), data_menuitem, gmi);

	/* DbusmenuMenuitem signals */
	g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menu_prop_change_cb), gmi);
	g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(delete_child), client);
	g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED,   G_CALLBACK(move_child),   client);

	/* GtkMenuitem signals */
	g_signal_connect(G_OBJECT(gmi), "activate", G_CALLBACK(menu_pressed_cb), item);

	/* Life insurance */
	g_object_weak_ref(G_OBJECT(item), destoryed_dbusmenuitem_cb, gmi);

	process_visible(gmi, dbusmenu_menuitem_property_get(item, DBUSMENU_MENUITEM_PROP_VISIBLE));
	process_sensitive(gmi, dbusmenu_menuitem_property_get(item, DBUSMENU_MENUITEM_PROP_SENSITIVE));

	if (parent != NULL) {
		new_child(parent, item, dbusmenu_menuitem_get_position(item, parent), DBUSMENU_GTKCLIENT(client));
	}

	return;
}

static void
new_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, DbusmenuGtkClient * gtkclient)
{
	if (dbusmenu_menuitem_get_root(mi)) { return; }

	gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	GtkMenu * menu = GTK_MENU(ann_menu);
	if (menu == NULL) {
		/* Oh, we don't have a submenu, build one! */
		menu = GTK_MENU(gtk_menu_new());
		g_object_set_data(G_OBJECT(mi), data_menu, menu);

		GtkMenuItem * parent = dbusmenu_gtkclient_menuitem_get(gtkclient, mi);
		gtk_menu_item_set_submenu(parent, GTK_WIDGET(menu));
	} 

	GtkMenuItem * childmi  = dbusmenu_gtkclient_menuitem_get(gtkclient, child);
	gtk_menu_shell_insert(GTK_MENU_SHELL(menu), GTK_WIDGET(childmi), position);
	gtk_widget_show(GTK_WIDGET(menu));
	
	return;
}

static void
delete_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, DbusmenuGtkClient * gtkclient)
{
	if (dbusmenu_menuitem_get_root(mi)) { return; }

	if (g_list_length(dbusmenu_menuitem_get_children(mi)) == 0) {
		gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
		GtkMenu * menu = GTK_MENU(ann_menu);

		if (menu != NULL) {
			gtk_widget_destroy(GTK_WIDGET(menu));
			g_object_set_data(G_OBJECT(mi), data_menu, NULL);
		}
	}

	return;
}

static void
move_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint new, guint old, DbusmenuGtkClient * gtkclient)
{
	if (dbusmenu_menuitem_get_root(mi)) { return; }

	gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	if (ann_menu == NULL) {
		g_warning("Moving a child when we don't have a submenu!");
		return;
	}

	GtkMenuItem * childmi  = dbusmenu_gtkclient_menuitem_get(gtkclient, child);
	gtk_menu_reorder_child(GTK_MENU(ann_menu), GTK_WIDGET(childmi), new);

	return;
}

/* Public API */

/**
	dbusmenu_gtkclient_new:
	@dbus_name: Name of the #DbusmenuServer on DBus
	@dbus_name: Name of the object on the #DbusmenuServer

	Creates a new #DbusmenuGtkClient object and creates a #DbusmenuClient
	that connects across DBus to a #DbusmenuServer.

	Return value: A new #DbusmenuGtkClient sync'd with a server
*/
DbusmenuGtkClient *
dbusmenu_gtkclient_new (gchar * dbus_name, gchar * dbus_object)
{
	return g_object_new(DBUSMENU_GTKCLIENT_TYPE,
	                    DBUSMENU_CLIENT_PROP_DBUS_OBJECT, dbus_object,
	                    DBUSMENU_CLIENT_PROP_DBUS_NAME, dbus_name,
	                    NULL);
}

/**
	dbusmenu_gtkclient_menuitem_get:
	@client: A #DbusmenuGtkClient with the item in it.
	@item: #DbusmenuMenuitem to get associated #GtkMenuItem on.

	This grabs the #GtkMenuItem that is associated with the
	#DbusmenuMenuitem.

	Return value: The #GtkMenuItem that can be played with.
*/
GtkMenuItem *
dbusmenu_gtkclient_menuitem_get (DbusmenuGtkClient * client, DbusmenuMenuitem * item)
{
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), NULL);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(item), NULL);

	GtkMenuItem * mi = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(item), data_menuitem));
	if (mi == NULL) {
		// new_menuitem(DBUSMENU_CLIENT(client), item, NULL);
		g_warning("GTK not updated");
		mi = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(item), data_menuitem));
	}

	return mi;
}

/* The base type handler that builds a plain ol'
   GtkMenuItem to represent, well, the GtkMenuItem */
static gboolean
new_item_normal (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	GtkMenuItem * gmi;
	gmi = GTK_MENU_ITEM(gtk_menu_item_new_with_label(dbusmenu_menuitem_property_get(newitem, DBUSMENU_MENUITEM_PROP_LABEL)));

	if (gmi != NULL) {
		dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);
	} else {
		return FALSE;
	}

	return TRUE;
}

/* Type handler for the seperators where it builds
   a GtkSeparator to act as the GtkMenuItem */
static gboolean
new_item_seperator (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	GtkMenuItem * gmi;
	gmi = GTK_MENU_ITEM(gtk_separator_menu_item_new());

	if (gmi != NULL) {
		dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);
	} else {
		return FALSE;
	}

	return TRUE;
}

/* This handler looks at property changes for items that are
   image menu items. */
static void
image_property_handle (DbusmenuMenuitem * item, const gchar * property, const gchar * value, gpointer userdata)
{
	/* We're only looking at these two properties here */
	g_return_if_fail(!g_strcmp0(property, DBUSMENU_MENUITEM_PROP_ICON) || !g_strcmp0(property, DBUSMENU_MENUITEM_PROP_ICON_DATA));

	if (value == NULL || value[0] == '\0') {
		/* This means that we're unsetting a value. */
		/* Try to use the other one */
		if (g_strcmp0(property, DBUSMENU_MENUITEM_PROP_ICON)) {
			property = DBUSMENU_MENUITEM_PROP_ICON_DATA;
		} else {
			property = DBUSMENU_MENUITEM_PROP_ICON;
		}
	}

	/* Grab the data of the items that we've got, so that
	   we can know how things need to change. */
	GtkMenuItem * gimi = dbusmenu_gtkclient_menuitem_get (DBUSMENU_GTKCLIENT(userdata), item);
	if (gimi == NULL) {
		g_warning("Oddly we're handling image properties on a menuitem that doesn't have any GTK structures associated with it.");
		return;
	}
	GtkWidget * gtkimage = gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(gimi));

	if (!g_strcmp0(property, DBUSMENU_MENUITEM_PROP_ICON_DATA)) {
		/* If we have an image already built from a name that is
		   way better than a pixbuf.  Keep it. */
		if (gtk_image_get_storage_type(GTK_IMAGE(gtkimage)) == GTK_IMAGE_ICON_NAME) {
			return;
		}
	}

	/* Now figure out what to change */
	if (!g_strcmp0(property, DBUSMENU_MENUITEM_PROP_ICON)) {
		const gchar * iconname = dbusmenu_menuitem_property_get(item, property);
		if (iconname == NULL) {
			/* If there is no name, by golly we want no
			   icon either. */
			gtkimage = NULL;
		} else {
			/* If we don't have an image, we need to build
			   one so that we can set the name.  Otherwise we
			   can just convert it to this name. */
			if (gtkimage == NULL) {
				gtkimage = gtk_image_new_from_icon_name(iconname, GTK_ICON_SIZE_MENU);
			} else {
				gtk_image_set_from_icon_name(GTK_IMAGE(gtkimage), iconname, GTK_ICON_SIZE_MENU);
			}
		}
	} else {
		GdkPixbuf * image = dbusmenu_menuitem_property_get_image(item, property);
		if (image == NULL) {
			/* If there is no pixbuf, by golly we want no
			   icon either. */
			gtkimage = NULL;
		} else {
			/* Resize the pixbuf */
			gint width, height;
			gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
			if (gdk_pixbuf_get_width(image) > width ||
					gdk_pixbuf_get_height(image) > height) {
				GdkPixbuf * newimage = gdk_pixbuf_scale_simple(image,
				                                               width,
				                                               height,
				                                               GDK_INTERP_BILINEAR);
				g_object_unref(image);
				image = newimage;
			}
			
			/* If we don't have an image, we need to build
			   one so that we can set the pixbuf. */
			if (gtkimage == NULL) {
				gtkimage = gtk_image_new_from_pixbuf(image);
			} else {
				gtk_image_set_from_pixbuf(GTK_IMAGE(gtkimage), image);
			}
		}

	}

	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gimi), gtkimage);

	return;
}

/* This is a type call back for the image type where
   it uses the GtkImageMenuitem to create the menu item. */
static gboolean
new_item_image (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	GtkMenuItem * gmi;
	gmi = GTK_MENU_ITEM(gtk_image_menu_item_new_with_label(dbusmenu_menuitem_property_get(newitem, DBUSMENU_MENUITEM_PROP_LABEL)));

	if (gmi != NULL) {
		dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);
	} else {
		return FALSE;
	}

	image_property_handle(newitem,
	                      DBUSMENU_MENUITEM_PROP_ICON,
	                      dbusmenu_menuitem_property_get(newitem, DBUSMENU_MENUITEM_PROP_ICON),
	                      client);
	image_property_handle(newitem,
	                      DBUSMENU_MENUITEM_PROP_ICON_DATA,
	                      dbusmenu_menuitem_property_get(newitem, DBUSMENU_MENUITEM_PROP_ICON_DATA),
	                      client);
	g_signal_connect(G_OBJECT(newitem),
	                 DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED,
	                 G_CALLBACK(image_property_handle),
	                 client);

	return TRUE;
}