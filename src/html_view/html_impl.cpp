/**
 * File:   html.c
 * Author: AWTK Develop Team
 * Brief:  Simple HTML View
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2021-03-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/path.h"
#include "tkc/utils.h"
#include "tkc/data_reader.h"
#include "widget_animators/widget_animator_scroll.h"

#include "html.h"
#include "html_impl.h"
#include "container_awtk.h"

static bool_t html_is_scollable(widget_t* widget);

struct _html_impl_t {
  container_awtk container;
  litehtml::context html_context;

  litehtml::tstring m_url;
  litehtml::tstring m_cursor;
  litehtml::tstring m_clicked_url;
  litehtml::tstring m_base_url;

  litehtml::document::ptr doc;
  litehtml::context* doc_context;

  ~_html_impl_t() {
    log_debug("~_html_impl_t\n");
  }
};

static const char s_master_css[] = {
#include "master.css.inc"
    , 0};

static uint32_t html_get_margin(widget_t* widget) {
  if (widget->astyle != NULL) {
    return style_get_int(widget->astyle, STYLE_ID_MARGIN, 0);
  } else {
    return 0;
  }
}

static uint32_t html_get_row_height(widget_t* widget) {
  return 40;
}

static ret_t html_load(widget_t* widget) {
  uint32_t size = 0;
  char base_url[MAX_PATH + 1];
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  litehtml::context* ctx = &(impl->html_context);
  litehtml::document_container* container = &(impl->container);

  if (html->url != NULL && html->url[0]) {
    char* data = (char*)data_reader_read_all(html->url, &size);
    return_value_if_fail(data != NULL, RET_BAD_PARAMS);

    path_dirname(html->url, base_url, MAX_PATH);
    container->set_base_url(base_url);

    impl->doc = litehtml::document::createFromString(data, container, ctx);
    TKMEM_FREE(data);
  } else if (widget->text.size > 0) {
    str_t str;
    str_init(&str, 0);
    str_from_wstr(&str, widget->text.str);

    container->set_base_url("");
    impl->doc = litehtml::document::createFromString(str.str, container, ctx);
    str_reset(&str);
  }

  if (impl->doc != NULL) {
    litehtml::document::ptr doc = impl->doc;
    doc->render(widget->w);
    widget_set_prop_int(widget, WIDGET_PROP_YOFFSET, 0);
  }

  return RET_OK;
}

static ret_t html_on_size_changed(widget_t* widget) {
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  litehtml::document::ptr doc = impl->doc;
  int32_t w = widget->w - 2 * html_get_margin(widget);

  if (doc != NULL && w != doc->width()) {
    doc->media_changed();
    doc->render(w);
  }

  return RET_OK;
}

ret_t html_get_prop(widget_t* widget, const char* name, value_t* v) {
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  return_value_if_fail(html != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(HTML_PROP_URL, name)) {
    value_set_str(v, html->url);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_YOFFSET, name)) {
    value_set_int32(v, html->yoffset);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_XOFFSET, name)) {
    value_set_int32(v, 0);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_VIRTUAL_W, name)) {
    value_set_int32(v, widget->w);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_VIRTUAL_H, name)) {
    int32_t virtual_h = widget->h;
    if (impl->doc != NULL) {
      virtual_h = tk_max(virtual_h, impl->doc->height());
    }
    value_set_int32(v, virtual_h);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_YSLIDABLE)) {
    value_set_bool(v, html->yslidable);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t idle_load_page(const idle_info_t* info) {
  widget_t* widget = WIDGET(info->ctx);
  
  html_load(widget);

  return RET_REMOVE;
}

ret_t html_set_prop(widget_t* widget, const char* name, const value_t* v) {
  html_t* html = HTML(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(HTML_PROP_URL, name)) {
    html_set_url(widget, value_str(v));
    if (widget_is_window_opened(widget)) {
      widget_add_idle(widget, idle_load_page);
    }
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_TEXT, name)) {
    wstr_from_value(&(widget->text), v);
    if (widget_is_window_opened(widget)) {
      widget_add_idle(widget, idle_load_page);
    }
  } else if (tk_str_eq(WIDGET_PROP_YOFFSET, name)) {
    html->yoffset = value_int32(v);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_XOFFSET, name)) {
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_VIRTUAL_W, name)) {
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_VIRTUAL_H, name)) {
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_YSLIDABLE)) {
    html->yslidable = value_bool(v);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

ret_t html_on_destroy(widget_t* widget) {
  html_t* html = HTML(widget);
  return_value_if_fail(widget != NULL && html != NULL, RET_BAD_PARAMS);

  delete html->impl;
  TKMEM_FREE(html->url);

  return RET_OK;
}

ret_t html_on_paint_self(widget_t* widget, canvas_t* c) {
  rect_t r;
  rect_t r_save;
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  int32_t yoffset = html->yoffset;
  //uint64_t start = time_now_us();

  r = rect_init(c->ox, c->oy, widget->w, widget->h);
  if (widget->astyle != NULL) {
    int32_t margin = style_get_int(widget->astyle, STYLE_ID_MARGIN, 0);

    r.x += margin;
    r.y += margin;
    r.w -= margin << 1;
    r.h -= margin << 1;
  }

  canvas_get_clip_rect(c, &r_save);
  r = rect_intersect(&r, &r_save);
  canvas_set_clip_rect(c, &r);

  if (impl->doc != NULL) {
    litehtml::position pos;
    vgcanvas_t* vg = canvas_get_vgcanvas(c);

    pos.x = 0;
    pos.y = yoffset;
    pos.width = widget->w;
    pos.height = widget->h;

    vgcanvas_save(vg);
    vgcanvas_translate(vg, c->ox, c->oy - yoffset);
    impl->container.set_clip_rect(pos);
    impl->doc->draw((litehtml::uint_ptr)c, 0, 0, &pos);
    vgcanvas_translate(vg, -c->ox, -c->oy);
    vgcanvas_restore(vg);
  }
  canvas_set_clip_rect(c, &r_save);
  //log_debug("html paint cost: %d us\n", (int)(time_now_us()-start));

  return RET_OK;
}

static ret_t html_on_scroll_done(void* ctx, event_t* e) {
  widget_t* widget = WIDGET(ctx);
  html_t* html = HTML(ctx);
  return_value_if_fail(widget != NULL && html != NULL, RET_BAD_PARAMS);

  html->wa = NULL;

  return RET_REMOVE;
}

static ret_t html_scroll_to(widget_t* widget, int32_t yoffset_end) {
  int32_t yoffset = 0;
  int32_t virtual_h = 0;
  html_t* html = HTML(widget);
  return_value_if_fail(html != NULL, RET_FAIL);
  virtual_h = widget_get_prop_int(widget, WIDGET_PROP_VIRTUAL_H, 0);

  if (!html_is_scollable(widget)) {
    html->yoffset = 0;
    return RET_OK;
  }

  if (yoffset_end < 0) {
    yoffset_end = 0;
  }

  if ((yoffset_end + widget->h) > virtual_h) {
    yoffset_end = virtual_h - widget->h;
  }

  if (yoffset_end == html->yoffset) {
    return RET_OK;
  }

  yoffset = html->yoffset;
  if (html->wa != NULL) {
    widget_animator_scroll_t* wa = (widget_animator_scroll_t*)html->wa;
    if (yoffset_end != html->yoffset) {
      bool_t changed = wa->y_to != yoffset_end;
      bool_t in_range = wa->y_to >= 0 && wa->y_to < (virtual_h - widget->h);
      if (changed && in_range) {
        wa->y_to = yoffset_end;
        wa->y_from = html->yoffset;
        wa->base.now = 0;
        wa->base.start_time = 0;
      }
    }
  } else {
    html->wa = widget_animator_scroll_create(widget, TK_ANIMATING_TIME, 0, EASING_SIN_INOUT);
    return_value_if_fail(html->wa != NULL, RET_OOM);
    widget_animator_scroll_set_params(html->wa, 0, yoffset, 0, yoffset_end);
    widget_animator_on(html->wa, EVT_ANIM_END, html_on_scroll_done, html);
    widget_animator_start(html->wa);
  }

  return RET_OK;
}

static ret_t html_scroll_delta_to(widget_t* widget, int32_t yoffset_delta) {
  html_t* html = HTML(widget);
  return_value_if_fail(html != NULL, RET_FAIL);

  html->yoffset_end = html->yoffset + yoffset_delta;

  return html_scroll_to(widget, html->yoffset_end);
}

static ret_t html_on_pointer_down(html_t* html, pointer_event_t* e) {
  velocity_t* v = &(html->velocity);
  return_value_if_fail(v != NULL, RET_BAD_PARAMS);

  velocity_reset(v);
  html->ydown = e->y;
  html->yoffset_save = html->yoffset;

  velocity_update(v, e->e.time, e->x, e->y);

  return RET_OK;
}

static ret_t html_on_pointer_move(html_t* html, pointer_event_t* e) {
  int32_t dy = 0;
  velocity_t* v = NULL;
  return_value_if_fail(html != NULL, RET_BAD_PARAMS);
  v = &(html->velocity);
  dy = e->y - html->ydown;

  velocity_update(v, e->e.time, e->x, e->y);

  if (html->wa == NULL && dy) {
    int32_t yoffset = html->yoffset_save - dy;
    widget_set_prop_int(WIDGET(html), WIDGET_PROP_YOFFSET, yoffset);
  }

  return RET_OK;
}

static ret_t html_on_pointer_up(html_t* html, pointer_event_t* e) {
  velocity_t* v = NULL;
  int32_t yoffset_end = 0;
  widget_t* widget = WIDGET(html);
  return_value_if_fail(html != NULL && widget != NULL, RET_BAD_PARAMS);
  v = &(html->velocity);

  velocity_update(v, e->e.time, e->x, e->y);
  yoffset_end = html->yoffset - v->yv;

  html_scroll_to(widget, yoffset_end);

  return RET_OK;
}

static ret_t html_down(widget_t* widget) {
  uint32_t row_height = html_get_row_height(widget);
  return html_scroll_delta_to(widget, row_height);
}

static ret_t html_up(widget_t* widget) {
  uint32_t row_height = html_get_row_height(widget);
  return html_scroll_delta_to(widget, -row_height);
}

static ret_t html_pagedown(widget_t* widget) {
  int32_t h = 0;
  int32_t pageh = 0;
  html_t* html = HTML(widget);
  int32_t row_height = html_get_row_height(widget);
  return_value_if_fail(html != NULL && widget != NULL, RET_BAD_PARAMS);

  h = widget->h - html_get_margin(widget) * 2 - 30;
  pageh = tk_max(h, row_height);

  return html_scroll_delta_to(widget, pageh);
}

static ret_t html_pageup(widget_t* widget) {
  int32_t h = 0;
  int32_t pageh = 0;
  html_t* html = HTML(widget);
  int32_t row_height = html_get_row_height(widget);
  return_value_if_fail(html != NULL && widget != NULL, RET_BAD_PARAMS);

  h = widget->h - html_get_margin(widget) * 2 - 30;
  pageh = tk_max(h, row_height);

  return html_scroll_delta_to(widget, -pageh);
}

static ret_t html_on_key_down(widget_t* widget, key_event_t* evt) {
  ret_t ret = RET_OK;
  return_value_if_fail(evt != NULL, RET_BAD_PARAMS);

  if (evt->key == TK_KEY_PAGEDOWN) {
    html_pagedown(widget);
    ret = RET_STOP;
  } else if (evt->key == TK_KEY_PAGEUP) {
    html_pageup(widget);
    ret = RET_STOP;
  } else if (evt->key == TK_KEY_UP) {
    html_up(widget);
    ret = RET_STOP;
  } else if (evt->key == TK_KEY_DOWN) {
    html_down(widget);
    ret = RET_STOP;
  }

  return ret;
}

static bool_t html_is_scollable(widget_t* widget) {
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  if (impl->doc == NULL) {
    return FALSE;
  } else {
    int32_t margin = html_get_margin(widget);
    return (impl->doc->height() + 2 * margin) > (int32_t)(widget->h);
  }
}

ret_t html_on_event(widget_t* widget, event_t* e) {
  ret_t ret = RET_OK;
  html_t* html = HTML(widget);
  html_impl_t* impl = html->impl;
  bool_t scrollable = html_is_scollable(widget);
  return_value_if_fail(widget != NULL && html != NULL, RET_BAD_PARAMS);

  switch (e->type) {
    case EVT_WINDOW_OPEN: {
      html_load(widget);
      break;
    }
    case EVT_RESIZE:
    case EVT_MOVE_RESIZE: {
      html_on_size_changed(widget);
      break;
    }
    case EVT_KEY_DOWN: {
      if (scrollable) {
        ret = html_on_key_down(widget, (key_event_t*)e);
      }
      break;
    }
    case EVT_POINTER_LEAVE: {
      litehtml::position::vector v;
      if (impl->doc != NULL) {
        impl->doc->on_mouse_leave(v);
      }
      break;
    }
    case EVT_POINTER_DOWN: {
      html->pressed = TRUE;
      if (scrollable && html->yslidable) {
        widget_grab(widget->parent, widget);
        html_on_pointer_down(html, (pointer_event_t*)e);
      }
      break;
    }
    case EVT_POINTER_MOVE: {
      if (scrollable && html->yslidable) {
        pointer_event_t* evt = (pointer_event_t*)e;
        if (evt->pressed && html->pressed) {
          html_on_pointer_move(html, evt);
          widget_invalidate(widget, NULL);
          ret = RET_STOP;
        }
      }
      break;
    }
    case EVT_POINTER_UP: {
      html->pressed = FALSE;
      if (scrollable && html->yslidable) {
        html_on_pointer_up(html, (pointer_event_t*)e);
        widget_ungrab(widget->parent, widget);
        ret = RET_STOP;
      }

      if (impl->doc != NULL && html->wa == NULL) {
        pointer_event_t* evt = (pointer_event_t*)e;
        litehtml::position::vector v;
        point_t p = {evt->x, evt->y};
        widget_to_local(widget, &p);

        p.y += html->yoffset;
        impl->doc->on_lbutton_down(p.x, p.y, evt->x, evt->y, v);
        impl->doc->on_lbutton_up(p.x, p.y, evt->x, evt->y, v);
      }
      break;
    }
    case EVT_WHEEL: {
      if (scrollable) {
        wheel_event_t* evt = (wheel_event_t*)e;
        if (evt->dy > 0) {
          html_down(widget);
        } else if (evt->dy < 0) {
          html_up(widget);
        }

        ret = RET_STOP;
      }
      break;
    }
    default:
      break;
  }

  return ret;
}

html_impl_t* html_impl_create(widget_t* widget) {
  html_impl_t* impl = new html_impl_t();
  if (impl != NULL) {
    impl->html_context.load_master_stylesheet(s_master_css);
    impl->container.set_view(widget);  
  }
  return impl;
}
