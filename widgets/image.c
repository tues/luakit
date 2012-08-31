/*
 * widgets/image.c - gtk image widget
 *
 * Copyright © 2010 Mason Larobina <mason.larobina@gmail.com>
 * Copyright © 2012 Pawel Bartkiewicz <tuuresairon@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "luah.h"
#include "widgets/common.h"

static widget_t*
luaH_checkimage(lua_State *L, gint udx)
{
    widget_t *w = luaH_checkwidget(L, udx);
    if (w->info->tok != L_TK_IMAGE)
        luaL_argerror(L, udx, "incorrect widget type (expected image)");
    return w;
}

static gint
luaH_image_get_align(lua_State *L, widget_t *w)
{
    gfloat xalign, yalign;
    gtk_misc_get_alignment(GTK_MISC(w->widget), &xalign, &yalign);
    lua_createtable(L, 0, 2);
    /* set align.x */
    lua_pushliteral(L, "x");
    lua_pushnumber(L, xalign);
    lua_rawset(L, -3);
    /* set align.y */
    lua_pushliteral(L, "y");
    lua_pushnumber(L, yalign);
    lua_rawset(L, -3);
    return 1;
}

static gint
luaH_image_set_align(lua_State *L, widget_t *w)
{
    luaH_checktable(L, 3);
    /* get old alignment values */
    gfloat xalign, yalign;
    gtk_misc_get_alignment(GTK_MISC(w->widget), &xalign, &yalign);
    /* get align.x */
    if (luaH_rawfield(L, 3, "x")) {
        xalign = (gfloat) lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    /* get align.y */
    if (luaH_rawfield(L, 3, "y")) {
        yalign = (gfloat) lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    gtk_misc_set_alignment(GTK_MISC(w->widget), xalign, yalign);
    return 0;
}

static gint
luaH_image_get_padding(lua_State *L, widget_t *w)
{
    gint xpad, ypad;
    gtk_misc_get_padding(GTK_MISC(w->widget), &xpad, &ypad);
    lua_createtable(L, 0, 2);
    /* set padding.x */
    lua_pushliteral(L, "x");
    lua_pushnumber(L, xpad);
    lua_rawset(L, -3);
    /* set padding.y */
    lua_pushliteral(L, "y");
    lua_pushnumber(L, ypad);
    lua_rawset(L, -3);
    return 1;
}

static gint
luaH_image_set_padding(lua_State *L, widget_t *w)
{
    luaH_checktable(L, 3);
    /* get old padding values */
    gint xpad = 0, ypad = 0;
    gtk_misc_get_padding(GTK_MISC(w->widget), &xpad, &ypad);
    /* get padding.x */
    if (luaH_rawfield(L, 3, "x")) {
        xpad = (gint) lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    /* get padding.y */
    if (luaH_rawfield(L, 3, "y")) {
        ypad = (gint) lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    gtk_misc_set_padding(GTK_MISC(w->widget), xpad, ypad);
    return 0;
}

static gint
luaH_image_load(lua_State *L)
{
    widget_t *w = luaH_checkimage(L, 1);
    const gchar *path = luaL_checkstring(L, 2);

    gint width = -1, height = -1;
    gint *tmp;

    tmp = (gint*) g_object_get_data(G_OBJECT(w->widget), "width");
    if (tmp)
        width = *tmp;

    tmp = (gint*) g_object_get_data(G_OBJECT(w->widget), "height");
    if (tmp)
        height = *tmp;

    GError *error = NULL;
    GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new_from_file_at_size(path, width, height, &error);

    if (!pixbuf)
    {
        warn("Error: %s\n", error->message);
        g_error_free(error);
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(w->widget), pixbuf);
    g_object_unref(pixbuf);

    return 0;
}

static gint
luaH_image_index(lua_State *L, widget_t *w, luakit_token_t token)
{
    gint *tmp;

    switch(token) {
      LUAKIT_WIDGET_INDEX_COMMON(w)

      case L_TK_PADDING:
        return luaH_image_get_padding(L, w);

      case L_TK_ALIGN:
        return luaH_image_get_align(L, w);

      case L_TK_WIDTH:
        tmp = (gint*) g_object_get_data(G_OBJECT(w->widget), "width");
        lua_pushinteger(L, (tmp ? *tmp : -1));
        return 1;

      case L_TK_HEIGHT:
        tmp = (gint*) g_object_get_data(G_OBJECT(w->widget), "height");
        lua_pushinteger(L, (tmp ? *tmp : -1));
        return 1;

      PF_CASE(LOAD,   luaH_image_load)

      default:
        break;
    }
    return 0;
}

static gint
luaH_image_newindex(lua_State *L, widget_t *w, luakit_token_t token)
{
    gint *tmp;

    switch(token) {
      LUAKIT_WIDGET_NEWINDEX_COMMON(w)

      case L_TK_PADDING:
        return luaH_image_set_padding(L, w);

      case L_TK_ALIGN:
        return luaH_image_set_align(L, w);

      case L_TK_WIDTH:
        tmp = g_malloc(sizeof(int));
        *tmp = (gint)luaL_checknumber(L, 3);
        g_object_set_data_full(G_OBJECT(w->widget), "width", tmp, g_free);
        break;

      case L_TK_HEIGHT:
        tmp = g_malloc(sizeof(int));
        *tmp = (gint)luaL_checknumber(L, 3);
        g_object_set_data_full(G_OBJECT(w->widget), "height", tmp, g_free);
        break;

      default:
        warn("unknown property: %s", luaL_checkstring(L, 2));
        return 0;
    }

    return luaH_object_property_signal(L, 1, token);
}

widget_t *
widget_image(widget_t *w, luakit_token_t UNUSED(token))
{
    w->index = luaH_image_index;
    w->newindex = luaH_image_newindex;
    w->destructor = widget_destructor;

    /* create gtk image widget as main widget */
    w->widget = gtk_image_new();

    g_object_connect(G_OBJECT(w->widget),
      LUAKIT_WIDGET_SIGNAL_COMMON(w)
      "signal::key-press-event",   G_CALLBACK(key_press_cb),  w,
      NULL);

    gtk_widget_show(w->widget);
    return w;
}

// vim: ft=c:et:sw=4:ts=8:sts=4:tw=80
