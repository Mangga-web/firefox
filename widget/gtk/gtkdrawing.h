/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * gtkdrawing.h: GTK widget rendering utilities
 *
 * gtkdrawing provides an API for rendering GTK widgets in the
 * current theme to a pixmap or window, without requiring an actual
 * widget instantiation, similar to the Macintosh Appearance Manager
 * or Windows XP's DrawThemeBackground() API.
 */

#ifndef _GTK_DRAWING_H_
#define _GTK_DRAWING_H_

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <algorithm>
#include "mozilla/Span.h"

/*** type definitions ***/
typedef struct {
  guint8 active;
  guint8 focused;
  guint8 selected;
  guint8 inHover;
  guint8 disabled;
  guint8 isDefault;
  guint8 canDefault;
  /* The depressed state is for buttons which remain active for a longer period:
   * activated toggle buttons or buttons showing a popup menu. */
  guint8 depressed;
  guint8 backdrop;
  gint32 curpos; /* curpos and maxpos are used for scrollbars */
  gint32 maxpos;
  gint32 image_scale; /* image scale */
} GtkWidgetState;

/**
 * A size in the same GTK pixel units as GtkBorder and GdkRectangle.
 */
struct MozGtkSize {
  gint width;
  gint height;

  MozGtkSize& operator+=(const GtkBorder& aBorder) {
    width += aBorder.left + aBorder.right;
    height += aBorder.top + aBorder.bottom;
    return *this;
  }
  MozGtkSize operator+(const GtkBorder& aBorder) const {
    MozGtkSize result = *this;
    return result += aBorder;
  }
  bool operator<(const MozGtkSize& aOther) const {
    return (width < aOther.width && height <= aOther.height) ||
           (width <= aOther.width && height < aOther.height);
  }
  void Include(MozGtkSize aOther) {
    width = std::max(width, aOther.width);
    height = std::max(height, aOther.height);
  }
  void Rotate() {
    gint tmp = width;
    width = height;
    height = tmp;
  }
};

struct ToolbarButtonGTKMetrics {
  MozGtkSize minSizeWithBorder{};
  gint iconXPosition = 0;
  gint iconYPosition = 0;
};

#define TOOLBAR_BUTTONS 3
struct ToolbarGTKMetrics {
  bool initialized = false;
  gint inlineSpacing = 0;
  ToolbarButtonGTKMetrics button[TOOLBAR_BUTTONS];
};

struct CSDWindowDecorationSize {
  bool initialized;
  GtkBorder decorationSize;
};

/** flags for tab state **/
enum GtkTabFlags {
  /* first eight bits are used to pass a margin */
  MOZ_GTK_TAB_MARGIN_MASK = 0xFF,
  /* the first tab in the group */
  MOZ_GTK_TAB_FIRST = 1 << 9,
  /* the selected tab */
  MOZ_GTK_TAB_SELECTED = 1 << 10
};

/*** result/error codes ***/
#define MOZ_GTK_SUCCESS 0
#define MOZ_GTK_UNKNOWN_WIDGET -1
#define MOZ_GTK_UNSAFE_THEME -2

/*** checkbox/radio flags ***/
#define MOZ_GTK_WIDGET_CHECKED 1
#define MOZ_GTK_WIDGET_INCONSISTENT (1 << 1)

/*** widget type constants ***/
enum WidgetNodeType : int {
  /* Paints a GtkButton. flags is a GtkReliefStyle. */
  MOZ_GTK_BUTTON,

  /* Vertical GtkScrollbar counterparts */
  MOZ_GTK_SCROLLBAR_VERTICAL,
  MOZ_GTK_SCROLLBAR_CONTENTS_VERTICAL,
  MOZ_GTK_SCROLLBAR_TROUGH_VERTICAL,
  MOZ_GTK_SCROLLBAR_THUMB_VERTICAL,

  /* Paints a GtkExpander. */
  MOZ_GTK_EXPANDER,
  /* Paints a GtkTextView or gets the style context corresponding to the
     root node of a GtkTextView. */
  MOZ_GTK_TEXT_VIEW,
  /* The "text" window or node of a GtkTextView */
  MOZ_GTK_TEXT_VIEW_TEXT,
  /* The "selection" node of a GtkTextView.text */
  MOZ_GTK_TEXT_VIEW_TEXT_SELECTION,

  /* Paints a GtkToolTip */
  MOZ_GTK_TOOLTIP,
  /* Paints a GtkBox from GtkToolTip  */
  MOZ_GTK_TOOLTIP_BOX,
  /* Paints a GtkLabel of GtkToolTip */
  MOZ_GTK_TOOLTIP_BOX_LABEL,
  /* Paints a GtkFrame (e.g. a status bar panel). */
  MOZ_GTK_FRAME,
  /* Paints the border of a GtkFrame */
  MOZ_GTK_FRAME_BORDER,
  /* Paints a resize grip for a GtkTextView */
  MOZ_GTK_RESIZER,
  /* Used as root style of whole GtkNotebook widget */
  MOZ_GTK_NOTEBOOK,
  /* Used as root style of active GtkNotebook area which contains tabs and
     arrows. */
  MOZ_GTK_NOTEBOOK_HEADER,
  /* Paints a tab of a GtkNotebook. flags is a GtkTabFlags, defined above. */
  MOZ_GTK_TAB_TOP,
  /* Paints a tab of a GtkNotebook. flags is a GtkTabFlags, defined above. */
  MOZ_GTK_TAB_BOTTOM,
  /* Paints the background and border of a GtkNotebook. */
  MOZ_GTK_TABPANELS,
  /* Paints the expander and border of a GtkTreeView */
  MOZ_GTK_TREEVIEW,
  /* Paints the border of a GtkTreeView */
  MOZ_GTK_TREEVIEW_VIEW,
  /* Paints treeheader cells */
  MOZ_GTK_TREE_HEADER_CELL,
  /* Paints the background of menus, context menus. */
  MOZ_GTK_MENUPOPUP,
  /* Menubar for -moz-headerbar colors */
  MOZ_GTK_MENUBAR,
  /* Paints an arrow in a toolbar button. flags is a GtkArrowType. */
  MOZ_GTK_TOOLBARBUTTON_ARROW,
  /* Paints items of popup menus. */
  MOZ_GTK_MENUITEM,
  /* Menubar menuitem for foreground colors. */
  MOZ_GTK_MENUBARITEM,
  /* GtkVPaned base class */
  MOZ_GTK_SPLITTER_HORIZONTAL,
  /* GtkHPaned base class */
  MOZ_GTK_SPLITTER_VERTICAL,
  /* Paints a GtkVPaned separator */
  MOZ_GTK_SPLITTER_SEPARATOR_HORIZONTAL,
  /* Paints a GtkHPaned separator */
  MOZ_GTK_SPLITTER_SEPARATOR_VERTICAL,
  /* Paints the background of a window, dialog or page. */
  MOZ_GTK_WINDOW,
  /* Used only as a container for MOZ_GTK_HEADER_BAR. */
  MOZ_GTK_HEADERBAR_WINDOW,
  /* Used only as a container for MOZ_GTK_HEADER_BAR_MAXIMIZED. */
  MOZ_GTK_HEADERBAR_WINDOW_MAXIMIZED,
  /* Used only as a container for MOZ_GTK_HEADER_BAR. */
  MOZ_GTK_HEADERBAR_FIXED,
  /* Used only as a container for MOZ_GTK_HEADER_BAR_MAXIMIZED. */
  MOZ_GTK_HEADERBAR_FIXED_MAXIMIZED,
  /* Window container for all widgets */
  MOZ_GTK_WINDOW_CONTAINER,
  /* Used for scrolled window shell. */
  MOZ_GTK_SCROLLED_WINDOW,
  /* Paints a GtkHeaderBar */
  MOZ_GTK_HEADER_BAR,
  /* Paints a GtkHeaderBar in maximized state */
  MOZ_GTK_HEADER_BAR_MAXIMIZED,
  /* Paints GtkHeaderBar title buttons.
   * Keep the order here as MOZ_GTK_HEADER_BAR_BUTTON_* are processed
   * as an array from MOZ_GTK_HEADER_BAR_BUTTON_CLOSE to the last one.
   */
  MOZ_GTK_HEADER_BAR_BUTTON_CLOSE,
  MOZ_GTK_HEADER_BAR_BUTTON_MINIMIZE,
  MOZ_GTK_HEADER_BAR_BUTTON_MAXIMIZE,

  /* MOZ_GTK_HEADER_BAR_BUTTON_MAXIMIZE_RESTORE is a state of
   * MOZ_GTK_HEADER_BAR_BUTTON_MAXIMIZE button and it's used as
   * an icon placeholder only.
   */
  MOZ_GTK_HEADER_BAR_BUTTON_MAXIMIZE_RESTORE,

  /* Client-side window decoration node. Available on GTK 3.20+. */
  MOZ_GTK_WINDOW_DECORATION,
  MOZ_GTK_WINDOW_DECORATION_SOLID,

  MOZ_GTK_WIDGET_NODE_COUNT
};

/* ButtonLayout represents a GTK CSD button and whether its on the left or
 * right side of the tab bar */
struct ButtonLayout {
  WidgetNodeType mType;
};

/*** General library functions ***/
/**
 * Initializes the drawing library.  You must call this function
 * prior to using any other functionality.
 * returns: MOZ_GTK_SUCCESS if there were no errors
 *          MOZ_GTK_UNSAFE_THEME if the current theme engine is known
 *                               to crash with gtkdrawing.
 */
gint moz_gtk_init();

/**
 * Updates the drawing library when the theme changes.
 */
void moz_gtk_refresh();

/**
 * Perform cleanup of the drawing library. You should call this function
 * when your program exits, or you no longer need the library.
 *
 * returns: MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_shutdown();

/*** Widget drawing ***/
/**
 * Paint a widget in the current theme.
 * widget:    a constant giving the widget to paint
 * drawable:  the drawable to paint to;
 *            it's colormap must be moz_gtk_widget_get_colormap().
 * rect:      the bounding rectangle for the widget
 * state:     the state of the widget.  ignored for some widgets.
 * flags:     widget-dependant flags; see the WidgetNodeType definition.
 * direction: the text direction, to draw the widget correctly LTR and RTL.
 */
gint moz_gtk_widget_paint(WidgetNodeType widget, cairo_t* cr,
                          GdkRectangle* rect, GtkWidgetState* state, gint flags,
                          GtkTextDirection direction);

/*** Widget metrics ***/
/**
 * Get the border size of a widget
 * left/right:  [OUT] the widget's left/right border
 * top/bottom:  [OUT] the widget's top/bottom border
 * direction:   the text direction for the widget.  Callers depend on this
 *              being used only for MOZ_GTK_DROPDOWN widgets, and cache
 *              results for other widget types across direction values.
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_widget_border(WidgetNodeType widget, gint* left, gint* top,
                               gint* right, gint* bottom,
                               GtkTextDirection direction);

/**
 * Get the border size of a notebook tab
 * left/right:  [OUT] the tab's left/right border
 * top/bottom:  [OUT] the tab's top/bottom border
 * direction:   the text direction for the widget
 * flags:       tab-dependant flags; see the GtkTabFlags definition.
 * widget:      tab widget
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_tab_border(gint* left, gint* top, gint* right, gint* bottom,
                            GtkTextDirection direction, GtkTabFlags flags,
                            WidgetNodeType widget);

/**
 * Some GTK themes draw their indication for the default button outside
 * the button (e.g. the glow in New Wave). This gets the extra space necessary.
 *
 * border_top:  [OUT] extra space to add above
 * border_left:  [OUT] extra space to add to the left
 * border_bottom:  [OUT] extra space to add underneath
 * border_right:  [OUT] extra space to add to the right
 *
 * returns:   MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_button_get_default_overflow(gint* border_top, gint* border_left,
                                         gint* border_bottom,
                                         gint* border_right);

/**
 * Gets the minimum size of a GtkScale.
 * orient:           [IN] the scale orientation
 * scale_width:      [OUT] the width of the scale
 * scale_height:     [OUT] the height of the scale
 */
void moz_gtk_get_scale_metrics(GtkOrientation orient, gint* scale_width,
                               gint* scale_height);

/**
 * Get the desired size of a GtkScale thumb
 * orient:           [IN] the scale orientation
 * thumb_length:     [OUT] the length of the thumb
 * thumb_height:     [OUT] the height of the thumb
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_scalethumb_metrics(GtkOrientation orient, gint* thumb_length,
                                    gint* thumb_height);

/**
 * Get the desired size of a scroll arrow widget
 * width:   [OUT] the desired width
 * height:  [OUT] the desired height
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_tab_scroll_arrow_size(gint* width, gint* height);

/**
 * Get the minimum height of a entry widget
 * min_content_height:    [OUT] the minimum height of the content box.
 * border_padding_height: [OUT] the size of borders and paddings.
 */
void moz_gtk_get_entry_min_height(gint* min_content_height,
                                  gint* border_padding_height);

/**
 * Get the desired size of a toolbar separator
 * size:    [OUT] the desired width
 *
 * returns: MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_toolbar_separator_width(gint* size);

/**
 * Get the desired size of a splitter
 * orientation:   [IN]  GTK_ORIENTATION_HORIZONTAL or GTK_ORIENTATION_VERTICAL
 * size:          [OUT] width or height of the splitter handle
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_splitter_get_metrics(gint orientation, gint* size);

/**
 * Get the YTHICKNESS of a tab (notebook extension).
 */
gint moz_gtk_get_tab_thickness(WidgetNodeType aNodeType);

/**
 * Get ToolbarButtonGTKMetrics for recent theme.
 */
const ToolbarButtonGTKMetrics* GetToolbarButtonMetrics(
    WidgetNodeType aAppearance);

gint moz_gtk_get_titlebar_button_spacing();

/**
 * Get toolbar button layout.
 * aButtonLayout:  [OUT] An array which will be filled by ButtonLayout
 *                       references to visible titlebar buttons. Must contain at
 *                       least TOOLBAR_BUTTONS entries if non-empty.
 * aReversedButtonsPlacement: [OUT] True if the buttons are placed in opposite
 *                                  titlebar corner.
 *
 * returns:    Number of returned entries at aButtonLayout.
 */
size_t GetGtkHeaderBarButtonLayout(mozilla::Span<ButtonLayout>,
                                   bool* aReversedButtonsPlacement);

#endif
