#include "html_view/html_view.h"
#include "gtest/gtest.h"

TEST(html_view, basic) {
  value_t v;
  widget_t* w = html_view_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
