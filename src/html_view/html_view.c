/**
 * File:   html_view.h
 * Author: AWTK Develop Team
 * Brief:  html_view
 *
 * Copyright (c) 2018 - 2021  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2021-03-14 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "scroll_view/scroll_bar.h"

#include "html_view/html.h"
#include "html_view/html_view.h"

static ret_t html_view_sync_scroll_bar_to_html(widget_t* widget) {
  int32_t h = 0;
  int32_t max = 0;
  int32_t yoffset = 0;
  html_t* html = NULL;
  int32_t value = 0;
  int32_t virtual_h = 0;
  html_view_t* html_view = HTML_VIEW(widget);
  return_value_if_fail(html_view != NULL, RET_BAD_PARAMS);
  html = HTML(html_view->html);
  value = widget_get_value(html_view->scroll_bar);
  virtual_h = widget_get_prop_int(html_view->html, WIDGET_PROP_VIRTUAL_H, 0);
  h = html_view->html->h;
  max = tk_max(virtual_h, h);

  if (max > h) {
    yoffset = (value * (max - h) ) / max;
  }

  if (html != NULL) {
    emitter_disable(html_view->html->emitter);
    widget_set_prop_int(html_view->html, WIDGET_PROP_YOFFSET, yoffset);
    emitter_enable(html_view->html->emitter);
  }

  return RET_OK;
}

static ret_t html_view_on_scroll_bar_changed(void* ctx, event_t* e) {
  html_view_sync_scroll_bar_to_html(WIDGET(ctx));
  return RET_OK;
}

static ret_t html_view_sync_html_to_scroll_bar(widget_t* widget) {
  int32_t h = 0;
  int32_t max = 0;
  int32_t value = 0;
  int32_t yoffset = 0;
  int32_t virtual_h = 0;
  html_view_t* html_view = HTML_VIEW(widget);
  return_value_if_fail(html_view != NULL, RET_BAD_PARAMS);

  yoffset = widget_get_prop_int(html_view->html, WIDGET_PROP_YOFFSET, 0);
  virtual_h = widget_get_prop_int(html_view->html, WIDGET_PROP_VIRTUAL_H, 0);
  h = html_view->html->h;
  max = tk_max(virtual_h, h);

  if (max > h) {
    value = (yoffset * max) / (max - h);
  }

  if (html_view->scroll_bar != NULL) {
    emitter_disable(html_view->scroll_bar->emitter);
    widget_set_prop_int(html_view->scroll_bar, WIDGET_PROP_MAX, max);
    widget_set_prop_int(html_view->scroll_bar, WIDGET_PROP_VALUE, value);
    emitter_enable(html_view->scroll_bar->emitter);
  }

  return RET_OK;
}

static ret_t html_view_on_html_scrolled(void* ctx, event_t* e) {
  prop_change_event_t* evt = (prop_change_event_t*)e;
  return_value_if_fail(evt != NULL, RET_BAD_PARAMS);
  if (tk_str_eq(evt->name, WIDGET_PROP_YOFFSET)) {
    html_view_sync_html_to_scroll_bar(WIDGET(ctx));
  }

  return RET_OK;
}

static ret_t html_view_on_html_resize(void* ctx, event_t* e) {
  html_view_t* html_view = HTML_VIEW(ctx);
  return_value_if_fail(html_view != NULL, RET_BAD_PARAMS);
  return_value_if_fail(html_view->scroll_bar != NULL, RET_BAD_PARAMS);

  widget_layout(html_view->scroll_bar);

  return RET_OK;
}

static ret_t html_view_on_add_child(widget_t* widget, widget_t* child) {
  html_view_t* html_view = HTML_VIEW(widget);
  return_value_if_fail(html_view != NULL, RET_BAD_PARAMS);
  if (tk_str_eq(widget_get_type(child), WIDGET_TYPE_HTML)) {
    html_view->html = child;
    widget_on(child, EVT_PROP_CHANGED, html_view_on_html_scrolled, widget);
    widget_on(child, EVT_RESIZE, html_view_on_html_resize, widget);
  } else if (tk_str_eq(widget_get_type(child), WIDGET_TYPE_SCROLL_BAR) ||
             tk_str_eq(widget_get_type(child), WIDGET_TYPE_SCROLL_BAR_DESKTOP) ||
             tk_str_eq(widget_get_type(child), WIDGET_TYPE_SCROLL_BAR_MOBILE)) {
    html_view->scroll_bar = child;
    widget_on(child, EVT_VALUE_CHANGED, html_view_on_scroll_bar_changed, widget);
  }

  return RET_FAIL;
}

static ret_t html_view_on_remove_child(widget_t* widget, widget_t* child) {
  html_view_t* html_view = HTML_VIEW(widget);
  return_value_if_fail(html_view != NULL, RET_BAD_PARAMS);

  widget_off_by_ctx(child, widget);
  if (html_view->html == child) {
    html_view->html = NULL;
  }

  if (html_view->scroll_bar == child) {
    html_view->scroll_bar = NULL;
  }

  return RET_FAIL;
}

TK_DECL_VTABLE(html_view) = {.size = sizeof(html_view_t),
                             .type = WIDGET_TYPE_HTML_VIEW,
                             .parent = TK_PARENT_VTABLE(widget),
                             .on_add_child = html_view_on_add_child,
                             .on_remove_child = html_view_on_remove_child,
                             .create = html_view_create};

widget_t* html_view_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  return widget_create(parent, TK_REF_VTABLE(html_view), x, y, w, h);
}

widget_t* html_view_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, html_view), NULL);

  return widget;
}
