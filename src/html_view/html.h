/**
 * File:   html.h
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

#ifndef TK_HTML_H
#define TK_HTML_H

#include "base/widget.h"
#include "base/velocity.h"
#include "base/widget_animator.h"

BEGIN_C_DECLS
struct _html_impl_t;
typedef struct _html_impl_t html_impl_t;

/**
 * @class html_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * Simple HTML View
 * 在xml中使用"html\_view"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <html x="c" y="50" w="100" h="100"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 * 
 * ```xml
 * <!-- style -->
 * <html>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </html>
 * ```
 */
typedef struct _html_t {
  widget_t widget;

  /** 
   * @property {int32_t} yoffset
   * @annotation ["set_prop","get_prop","readable"]
   * y偏移。
   */
  int32_t yoffset;

  /** 
   * @property {bool_t} yslidable
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 标识控件是否允许上下拖动。
   */
  bool_t yslidable;

  /**
   * @property {char*} url
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 网址。。
   */
  char* url;

  /*private*/
  darray_t backs;
  darray_t forwards;
  html_impl_t* impl;

  bool_t pressed;
  int32_t ydown;
  int32_t yoffset_end;
  widget_animator_t* wa;
  velocity_t velocity;
  int32_t yoffset_save;
} html_t;

/**
 * @method html_create
 * @annotation ["constructor", "scriptable"]
 * 创建html对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} html对象。
 */
widget_t* html_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method html_cast
 * 转换为html对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget html对象。
 *
 * @return {widget_t*} html对象。
 */
widget_t* html_cast(widget_t* widget);

/**
 * @method html_set_url
 * 设置 网址。。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} url 网址。。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t html_set_url(widget_t* widget, const char* url);

/**
 * @method html_set_yslidable
 * 设置是否允许y方向滑动。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} yslidable 是否允许滑动。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t html_set_yslidable(widget_t* widget, bool_t yslidable);

#define HTML_PROP_URL "url"

#define WIDGET_TYPE_HTML "html"

#define HTML(widget) ((html_t*)(html_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(html);

END_C_DECLS

#endif /*TK_HTML_H*/
