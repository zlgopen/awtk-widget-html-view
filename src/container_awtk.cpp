#include "awtk.h"
#include "container_awtk.h"

typedef struct _html_font_t {
  char* name;
  int size;
  int weight;
  bool_t italic;
  bool_t under_line;
  bool_t line_through;

  int ascent;
  int descent;
} html_font_t;

static void my_make_url(const litehtml::tchar_t* url, const litehtml::tchar_t* basepath,
                              litehtml::tstring& out) {
  if (url != NULL) {
    const char* path = strstr(url, "://");

    if (path == NULL) {
      path = url;
    } else {
      path += 3;
    }

    if (path[0] == '/' || basepath == NULL || *basepath == '\0') {
      out = url;
    } else {
      str_t str;
      str_init(&str, 0);

      str_append(&str, basepath);
      str_append(&str, "/");
      str_append(&str, path);
      out = str.str;
      str_reset(&str);
    }
  }
}

static ret_t awtk_load_image(widget_t* view, const litehtml::tchar_t* src,
                             const litehtml::tchar_t* baseurl, bitmap_t* img) {
  if(strchr(src, '.') != NULL) {
    litehtml::tstring abs_url;
    my_make_url(src, baseurl, abs_url);
    return widget_load_image(view, abs_url.c_str(), img);
  } else {
    return widget_load_image(view, src, img);
  }
}

static html_font_t* html_font_create(const char* name, int size, int weight, bool_t italic,
                                     bool_t line_through, bool_t under_line) {
  html_font_t* font = TKMEM_ZALLOC(html_font_t);
  return_value_if_fail(font != NULL, NULL);
  font->name = tk_strdup(name);
  font->size = size;
  font->weight = weight;
  font->italic = italic;
  font->under_line = under_line;
  font->line_through = line_through;

  return font;
}

static ret_t html_font_destroy(html_font_t* font) {
  return_value_if_fail(font != NULL, RET_BAD_PARAMS);
  TKMEM_FREE(font->name);
  TKMEM_FREE(font);

  return RET_OK;
}

static bool vgcanvas_add_arc(vgcanvas_t* vg, double x, double y, double rx, double ry, double a1,
                             double a2, bool neg) {
  if (rx > 0 && ry > 0) {
    vgcanvas_save(vg);
    vgcanvas_translate(vg, x, y);
    vgcanvas_scale(vg, 1, ry / rx);
    vgcanvas_translate(vg, -x, -y);
    vgcanvas_arc(vg, x, y, rx, a1, a2, neg);
    vgcanvas_restore(vg);
    return true;
  }

  return false;
}

static ret_t vgcanvas_rounded_rect_ex(vgcanvas_t* vg, const litehtml::position& pos,
                                      const litehtml::border_radiuses& radius) {
  vgcanvas_begin_path(vg);

  if (radius.top_left_x) {
    vgcanvas_arc(vg, pos.left() + radius.top_left_x, pos.top() + radius.top_left_x,
                 radius.top_left_x, M_PI, M_PI * 3.0 / 2.0, TRUE);
  } else {
    vgcanvas_move_to(vg, pos.left(), pos.top());
  }

  vgcanvas_line_to(vg, pos.right() - radius.top_right_x, pos.top());

  if (radius.top_right_x) {
    vgcanvas_arc(vg, pos.right() - radius.top_right_x, pos.top() + radius.top_right_x,
                 radius.top_right_x, M_PI * 3.0 / 2.0, 2.0 * M_PI, TRUE);
  }

  vgcanvas_line_to(vg, pos.right(), pos.bottom() - radius.bottom_right_x);

  if (radius.bottom_right_x) {
    vgcanvas_arc(vg, pos.right() - radius.bottom_right_x, pos.bottom() - radius.bottom_right_x,
                 radius.bottom_right_x, 0, M_PI / 2.0, TRUE);
  }

  vgcanvas_line_to(vg, pos.left() - radius.bottom_left_x, pos.bottom());

  if (radius.bottom_left_x) {
    vgcanvas_arc(vg, pos.left() + radius.bottom_left_x, pos.bottom() - radius.bottom_left_x,
                 radius.bottom_left_x, M_PI / 2.0, M_PI, TRUE);
  }

  return RET_OK;
}

static ret_t canvas_apply_font(canvas_t* c, html_font_t* font) {
  return_value_if_fail(c != NULL && font != NULL, RET_BAD_PARAMS);
  
  if(tk_str_eq(c->font_name, font->name) && c->font_size ==  font->size) {
    return RET_OK;
  }

  canvas_set_font(c, font->name, font->size);

  return RET_OK;
}

static color_t color_from_web_color(const litehtml::web_color& color) {
  return color_init(color.red, color.green, color.blue, color.alpha);
}

static rect_t rect_from_position(const litehtml::position& pos) {
  return rect_init(pos.x, pos.y, pos.width, pos.height);
}

static ret_t vgcanvas_apply_fill_color(vgcanvas_t* vg, litehtml::web_color color) {
  vgcanvas_set_fill_color(vg, color_from_web_color(color));

  return RET_OK;
}

static ret_t vgcanvas_apply_stroke_color(vgcanvas_t* vg, litehtml::web_color color) {
  vgcanvas_set_stroke_color(vg, color_from_web_color(color));

  return RET_OK;
}

container_awtk::container_awtk() {
}

container_awtk::~container_awtk() {
}

litehtml::uint_ptr container_awtk::create_font(const litehtml::tchar_t* faceName, int size,
                                               int weight, litehtml::font_style italic,
                                               unsigned int decoration,
                                               litehtml::font_metrics* fm) {
  bool_t aitalic = (italic == litehtml::fontStyleItalic) ? TRUE : FALSE;
  bool_t line_through = (decoration & litehtml::font_decoration_linethrough) ? TRUE : FALSE;
  bool_t under_line = (decoration & litehtml::font_decoration_underline) ? TRUE : FALSE;

  html_font_t* font = html_font_create(faceName, size, weight, aitalic, line_through, under_line);
  return_value_if_fail(font != NULL, (litehtml::uint_ptr)NULL);

  if (fm) {
    float_t ascent = 0;
    float_t descent = 0;
    float_t height = 0;

    canvas_t* c = widget_get_canvas(this->view);

    canvas_apply_font(c, font);
    canvas_get_text_metrics(c, &ascent, &descent, &height);

    descent = tk_abs(descent);

    fm->ascent = ascent;
    fm->descent = descent;
    fm->height = height;
    fm->x_height = 3;

    font->ascent = ascent;
    font->descent = descent;

    if (italic == litehtml::fontStyleItalic || decoration) {
      fm->draw_spaces = true;
    } else {
      fm->draw_spaces = false;
    }
  }

  return (litehtml::uint_ptr)font;
}

void container_awtk::delete_font(litehtml::uint_ptr hFont) {
  html_font_t* font = (html_font_t*)hFont;
  html_font_destroy(font);
}

int container_awtk::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) {
  canvas_t* c = widget_get_canvas(this->view);
  html_font_t* font = (html_font_t*)hFont;

  canvas_apply_font(c, font);
  return canvas_measure_utf8(c, text);
}

void container_awtk::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text,
                               litehtml::uint_ptr hFont, litehtml::web_color color,
                               const litehtml::position& pos) {
  canvas_t* c = (canvas_t*)hdc;
  int32_t ox = c->ox;
  int32_t oy = c->oy;
  html_font_t* font = (html_font_t*)hFont;
  rect_t r = rect_from_position(pos);
  
  if(!this->is_in_clip_rect(pos)) {
    return;
  }

  canvas_apply_font(c, font);
  canvas_set_text_color(c, color_from_web_color(color));
  canvas_set_text_align(c, ALIGN_H_LEFT, ALIGN_V_MIDDLE);

  canvas_translate(c, -ox, -oy);
  canvas_draw_utf8_in_rect(c, text, &r);
  canvas_translate(c, ox, oy);
}

int container_awtk::pt_to_px(int pt) {
  return (int)((double)pt * 96 / 72.0);
}

int container_awtk::get_default_font_size() const {
  return TK_DEFAULT_FONT_SIZE;
}

const litehtml::tchar_t* container_awtk::get_default_font_name() const {
  return _t(system_info()->default_font);
}

void container_awtk::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {
  canvas_t* c = (canvas_t*)hdc;
  vgcanvas_t* vg = lcd_get_vgcanvas(c->lcd);
  
  if(!this->is_in_clip_rect(marker.pos)) {
    return;
  }

  if (!marker.image.empty()) {
    bitmap_t img;
    if (awtk_load_image(this->view, marker.image.c_str(), m_base_url.c_str(), &img) == RET_OK) {
      int32_t ox = c->ox;
      int32_t oy = c->oy;
      rect_t r = rect_from_position(marker.pos);

      canvas_translate(c, -ox, -oy);
      canvas_draw_image_ex(c, &img, IMAGE_DRAW_ICON, &r);
      canvas_translate(c, ox, oy);
    }
  } else {
    switch (marker.marker_type) {
      case litehtml::list_style_type_circle: {
        vgcanvas_ellipse(vg, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
        vgcanvas_apply_stroke_color(vg, marker.color);
        vgcanvas_stroke(vg);
        break;
      }
      case litehtml::list_style_type_disc: {
        vgcanvas_ellipse(vg, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
        vgcanvas_apply_fill_color(vg, marker.color);
        vgcanvas_fill(vg);
        break;
      }
      case litehtml::list_style_type_square: {
        if (hdc) {
          vgcanvas_rect(vg, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
          vgcanvas_apply_fill_color(vg, marker.color);
          vgcanvas_fill(vg);
        }
        break;
      }
      default:
        break;
    }
  }
}

void container_awtk::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl,
                                bool redraw_on_ready) {
  bitmap_t img;
  awtk_load_image(this->view, src, m_base_url.c_str(), &img);
}

void container_awtk::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl,
                                    litehtml::size& sz) {
  bitmap_t img;
  if (awtk_load_image(this->view, src, m_base_url.c_str(), &img) == RET_OK) {
    float_t ratio = system_info()->device_pixel_ratio;
    sz.width = img.w / ratio;
    sz.height = img.h / ratio;
  }
}

void container_awtk::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) {
  canvas_t* c = (canvas_t*)hdc;
  vgcanvas_t* vg = lcd_get_vgcanvas(c->lcd);
  int32_t ox = c->ox;
  int32_t oy = c->oy;

  if(!this->is_in_clip_rect(bg.border_box)) {
    return;
  }

  vgcanvas_save(vg);

  if (bg.color.alpha) {
    vgcanvas_rounded_rect_ex(vg, bg.border_box, bg.border_radius);
    vgcanvas_apply_fill_color(vg, bg.color);
    vgcanvas_fill(vg);
  }

  if (bg.image.size() > 0) {
    bitmap_t img;
    const char* src = bg.image.c_str();
    rect_t r = rect_from_position(bg.clip_box);

    if (awtk_load_image(this->view, src, m_base_url.c_str(), &img) == RET_OK) {
      switch (bg.repeat) {
        case litehtml::background_repeat_no_repeat:
          vgcanvas_draw_image(vg, &img, 0, 0, img.w, img.h, r.x, r.y, r.w, r.h);
          break;

        case litehtml::background_repeat_repeat_x:
          canvas_translate(c, -ox, -oy);
          canvas_draw_image_ex(c, &img, IMAGE_DRAW_REPEAT_X, &r);
          canvas_translate(c, ox, oy);
          break;

        case litehtml::background_repeat_repeat_y:
          canvas_translate(c, -ox, -oy);
          canvas_draw_image_ex(c, &img, IMAGE_DRAW_REPEAT_Y, &r);
          canvas_translate(c, ox, oy);
          break;

        case litehtml::background_repeat_repeat:
          canvas_translate(c, -ox, -oy);
          canvas_draw_image_ex(c, &img, IMAGE_DRAW_REPEAT, &r);
          canvas_translate(c, ox, oy);
          break;
        default:
          break;
      }
    }
  }
  vgcanvas_restore(vg);
}


void container_awtk::make_url(const litehtml::tchar_t* url, const litehtml::tchar_t* basepath,
                              litehtml::tstring& out) {
   my_make_url(url, basepath, out);
}

void container_awtk::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders,
                                  const litehtml::position& draw_pos, bool root) {
  int bdr_top = 0;
  int bdr_bottom = 0;
  int bdr_left = 0;
  int bdr_right = 0;
  canvas_t* c = (canvas_t*)hdc;
  vgcanvas_t* vg = lcd_get_vgcanvas(c->lcd);
  vgcanvas_save(vg);

  this->apply_clip(vg);

  vgcanvas_begin_path(vg);

  if (borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden) {
    bdr_top = (int)borders.top.width;
  }
  if (borders.bottom.width != 0 && borders.bottom.style > litehtml::border_style_hidden) {
    bdr_bottom = (int)borders.bottom.width;
  }
  if (borders.left.width != 0 && borders.left.style > litehtml::border_style_hidden) {
    bdr_left = (int)borders.left.width;
  }
  if (borders.right.width != 0 && borders.right.style > litehtml::border_style_hidden) {
    bdr_right = (int)borders.right.width;
  }

  // draw right border
  if (bdr_right) {
    vgcanvas_apply_fill_color(vg, borders.right.color);
    double r_top = (double)borders.radius.top_right_x;
    double r_bottom = (double)borders.radius.bottom_right_x;
    vgcanvas_begin_path(vg);
    vgcanvas_move_to(vg, draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);

    if (r_top) {
      double end_angle = 2.0 * M_PI;
      double start_angle = end_angle - M_PI / 2.0 / ((double)bdr_top / (double)bdr_right + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_top, draw_pos.top() + r_top, r_top - bdr_right,
                            r_top - bdr_right + (bdr_right - bdr_top), end_angle, start_angle,
                            true)) {
        vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);
      }

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_top, draw_pos.top() + r_top, r_top, r_top,
                            start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.right(), draw_pos.top());
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);
      vgcanvas_line_to(vg, draw_pos.right(), draw_pos.top());
    }

    if (r_bottom) {
      vgcanvas_line_to(vg, draw_pos.right(), draw_pos.bottom() - r_bottom);

      double start_angle = 0;
      double end_angle = start_angle + M_PI / 2.0 / ((double)bdr_bottom / (double)bdr_right + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_bottom, draw_pos.bottom() - r_bottom, r_bottom,
                            r_bottom, start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.right(), draw_pos.bottom());
      }

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_bottom, draw_pos.bottom() - r_bottom,
                            r_bottom - bdr_right, r_bottom - bdr_right + (bdr_right - bdr_bottom),
                            end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom);
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.right(), draw_pos.bottom());
      vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom);
    }

    vgcanvas_fill(vg);
  }

  // draw bottom border
  if (bdr_bottom) {
    vgcanvas_apply_fill_color(vg, borders.bottom.color);

    double r_left = borders.radius.bottom_left_x;
    double r_right = borders.radius.bottom_right_x;
    vgcanvas_begin_path(vg);
    vgcanvas_move_to(vg, draw_pos.left(), draw_pos.bottom());

    if (r_left) {
      double start_angle = M_PI / 2.0;
      double end_angle = start_angle + M_PI / 2.0 / ((double)bdr_left / (double)bdr_bottom + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_left, draw_pos.bottom() - r_left,
                            r_left - bdr_bottom + (bdr_bottom - bdr_left), r_left - bdr_bottom,
                            start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
      }

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_left, draw_pos.bottom() - r_left, r_left,
                            r_left, end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.left(), draw_pos.bottom());
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.left(), draw_pos.bottom());
      vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
    }

    if (r_right) {
      vgcanvas_line_to(vg, draw_pos.right() - r_right, draw_pos.bottom());

      double end_angle = M_PI / 2.0;
      double start_angle = end_angle - M_PI / 2.0 / ((double)bdr_right / (double)bdr_bottom + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_right, draw_pos.bottom() - r_right, r_right,
                            r_right, end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.right(), draw_pos.bottom());
      }

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_right, draw_pos.bottom() - r_right,
                            r_right - bdr_bottom + (bdr_bottom - bdr_right), r_right - bdr_bottom,
                            start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom);
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.bottom() - bdr_bottom);
      vgcanvas_line_to(vg, draw_pos.right(), draw_pos.bottom());
    }

    vgcanvas_fill(vg);
  }

  // draw top border
  if (bdr_top) {
    vgcanvas_apply_fill_color(vg, borders.top.color);

    double r_left = borders.radius.top_left_x;
    double r_right = borders.radius.top_right_x;
    vgcanvas_begin_path(vg);
    vgcanvas_move_to(vg, draw_pos.left(), draw_pos.top());

    if (r_left) {
      double end_angle = M_PI * 3.0 / 2.0;
      double start_angle = end_angle - M_PI / 2.0 / ((double)bdr_left / (double)bdr_top + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_left, draw_pos.top() + r_left, r_left, r_left,
                            end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.left(), draw_pos.top());
      }

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_left, draw_pos.top() + r_left,
                            r_left - bdr_top + (bdr_top - bdr_left), r_left - bdr_top, start_angle,
                            end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.left(), draw_pos.top());
      vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
    }

    if (r_right) {
      vgcanvas_line_to(vg, draw_pos.right() - r_right, draw_pos.top() + bdr_top);

      double start_angle = M_PI * 3.0 / 2.0;
      double end_angle = start_angle + M_PI / 2.0 / ((double)bdr_right / (double)bdr_top + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_right, draw_pos.top() + r_right,
                            r_right - bdr_top + (bdr_top - bdr_right), r_right - bdr_top,
                            start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);
      }

      if (!vgcanvas_add_arc(vg, draw_pos.right() - r_right, draw_pos.top() + r_right, r_right,
                            r_right, end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.right(), draw_pos.top());
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);
      vgcanvas_line_to(vg, draw_pos.right(), draw_pos.top());
    }

    vgcanvas_fill(vg);
  }

  // draw left border
  if (bdr_left) {
    vgcanvas_apply_fill_color(vg, borders.left.color);

    double r_top = borders.radius.top_left_x;
    double r_bottom = borders.radius.bottom_left_x;
    vgcanvas_begin_path(vg);
    vgcanvas_move_to(vg, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);

    if (r_top) {
      double start_angle = M_PI;
      double end_angle = start_angle + M_PI / 2.0 / ((double)bdr_top / (double)bdr_left + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_top, draw_pos.top() + r_top, r_top - bdr_left,
                            r_top - bdr_left + (bdr_left - bdr_top), start_angle, end_angle,
                            false)) {
        vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
      }

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_top, draw_pos.top() + r_top, r_top, r_top,
                            end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.left(), draw_pos.top());
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
      vgcanvas_line_to(vg, draw_pos.left(), draw_pos.top());
    }

    if (r_bottom) {
      vgcanvas_line_to(vg, draw_pos.left(), draw_pos.bottom() - r_bottom);

      double end_angle = M_PI;
      double start_angle = end_angle - M_PI / 2.0 / ((double)bdr_bottom / (double)bdr_left + 0.5);

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_bottom, draw_pos.bottom() - r_bottom, r_bottom,
                            r_bottom, end_angle, start_angle, true)) {
        vgcanvas_line_to(vg, draw_pos.left(), draw_pos.bottom());
      }

      if (!vgcanvas_add_arc(vg, draw_pos.left() + r_bottom, draw_pos.bottom() - r_bottom,
                            r_bottom - bdr_left, r_bottom - bdr_left + (bdr_left - bdr_bottom),
                            start_angle, end_angle, false)) {
        vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
      }
    } else {
      vgcanvas_line_to(vg, draw_pos.left(), draw_pos.bottom());
      vgcanvas_line_to(vg, draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
    }

    vgcanvas_fill(vg);
  }
  vgcanvas_restore(vg);
}

void container_awtk::set_caption(const litehtml::tchar_t* caption){};  //: set_caption

void container_awtk::set_base_url(const litehtml::tchar_t* base_url) {
  m_base_url = base_url;
};  

void container_awtk::link(const std::shared_ptr<litehtml::document>& ptr,
                          const litehtml::element::ptr& el) {
 log_debug("container_awtk::link\n");
}

void container_awtk::on_anchor_click(const litehtml::tchar_t* url,
                                     const litehtml::element::ptr& el) {
  log_debug("click: %s\n", url);
  this->make_url(url, m_base_url.c_str(), m_clicked_url);
  widget_set_prop_str(this->view, "url", m_clicked_url.c_str());
}

void container_awtk::set_cursor(const litehtml::tchar_t* cursor) {
}

void container_awtk::transform_text(litehtml::tstring& text, litehtml::text_transform tt) {
}

void container_awtk::import_css(litehtml::tstring& text, const litehtml::tstring& url,
                                litehtml::tstring& baseurl) {
  uint32_t size = 0;
  litehtml::tstring abs_url;
  this->make_url(url.c_str(), m_base_url.c_str(), abs_url);

  char* content = (char*)data_reader_read_all(abs_url.c_str(), &size);
  if (content != NULL) {
    text = content;
    TKMEM_FREE(content);
  }
}

void container_awtk::set_clip(const litehtml::position& pos,
                              const litehtml::border_radiuses& bdr_radius, bool valid_x,
                              bool valid_y) {
}

void container_awtk::del_clip() {
}

void container_awtk::apply_clip(vgcanvas_t* vg) {
}

void container_awtk::get_client_rect(litehtml::position& client) const {
  client.x = 0;
  client.y = 0;
  client.width = this->view->w;
  client.height = this->view->h;
}

std::shared_ptr<litehtml::element> container_awtk::create_element(
    const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes,
    const std::shared_ptr<litehtml::document>& doc) {
  return 0;
}

void container_awtk::get_media_features(litehtml::media_features& media) const {
  litehtml::position client;
  widget_t* wm = window_manager();
  get_client_rect(client);
  media.type = litehtml::media_type_screen;
  media.width = client.width;
  media.height = client.height;
  media.device_width = wm->w;
  media.device_height = wm->h;
  media.color = 8;
  media.monochrome = 0;
  media.color_index = 256;
  media.resolution = 96;
}


void container_awtk::get_language(litehtml::tstring& language, litehtml::tstring& culture) const {
  language = _t("en");
  culture = _t("");
}

