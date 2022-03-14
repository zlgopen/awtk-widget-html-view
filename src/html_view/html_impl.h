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

#ifndef TK_HTML_IMPL_H
#define TK_HTML_IMPL_H

#include "base/widget.h"

BEGIN_C_DECLS

struct _html_impl_t;
typedef struct _html_impl_t html_impl_t;

html_impl_t* html_impl_create(widget_t* widget);
ret_t html_get_prop(widget_t* widget, const char* name, value_t* v);
ret_t html_set_prop(widget_t* widget, const char* name, const value_t* v);
ret_t html_on_paint_self(widget_t* widget, canvas_t* c);
ret_t html_on_event(widget_t* widget, event_t* e);
ret_t html_on_destroy(widget_t* widget);


END_C_DECLS

#endif /*TK_HTML_IMPL_H*/
