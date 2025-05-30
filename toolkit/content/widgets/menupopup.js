/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// This is loaded into all XUL windows. Wrap in a block to prevent
// leaking to window scope.
{
  const { AppConstants } = ChromeUtils.importESModule(
    "resource://gre/modules/AppConstants.sys.mjs"
  );

  // For the non-native context menu styling, we need to know if we need a
  // gutter for checkboxes or icons. On linux any checkbox / radio / icon
  // requires a gutter. On Windows, only checked items do. On macOS, we also
  // need selected to deal with the menulists (like `<select>`).
  const ITEM_NEEDS_GUTTER_SELECTOR = (() => {
    if (AppConstants.platform == "macosx") {
      return "[checked=true], [selected=true]";
    }
    if (AppConstants.platform == "windows") {
      return "[checked=true]";
    }
    return "[type=checkbox], [type=radio]";
  })();

  const GUTTER_SELECTOR = `:scope > menuitem:not([hidden]):is(${ITEM_NEEDS_GUTTER_SELECTOR})`;

  document.addEventListener(
    "popupshowing",
    function (e) {
      if (
        e.target.nodeName == "menupopup" &&
        e.target.getAttribute("needsgutter") != "always"
      ) {
        e.target.toggleAttribute(
          "needsgutter",
          !!e.target.querySelector(GUTTER_SELECTOR)
        );
      }
    },
    // we use a system bubbling event listener to ensure we run *after* the
    // "normal" popupshowing listeners, so (visibility) changes they make to
    // their items take effect first, before we check for checkable menuitems.
    { mozSystemGroup: true }
  );

  class MozMenuPopup extends MozElements.MozElementMixin(XULPopupElement) {
    constructor() {
      super();

      this.AUTOSCROLL_INTERVAL = 25;
      this.NOT_DRAGGING = 0;
      this.DRAG_OVER_BUTTON = -1;
      this.DRAG_OVER_POPUP = 1;
      this._draggingState = this.NOT_DRAGGING;
      this._scrollTimer = 0;

      this.attachShadow({ mode: "open" });

      this.addEventListener("popupshowing", event => {
        if (event.target != this) {
          return;
        }

        // Make sure we generated shadow DOM to place menuitems into.
        this.ensureInitialized();
      });

      this.addEventListener("DOMMenuItemActive", this);
    }

    connectedCallback() {
      if (this.delayConnectedCallback() || this.hasConnected) {
        return;
      }

      this.hasConnected = true;
      if (this.parentNode?.localName == "menulist") {
        this._setUpMenulistPopup();
      }
    }

    initShadowDOM() {
      // Retarget events from shadow DOM arrowscrollbox to the host.
      this.scrollBox.addEventListener("scroll", () =>
        this.dispatchEvent(new Event("scroll"))
      );
      this.scrollBox.addEventListener("overflow", () =>
        this.dispatchEvent(new Event("overflow"))
      );
      this.scrollBox.addEventListener("underflow", () =>
        this.dispatchEvent(new Event("underflow"))
      );
    }

    ensureInitialized() {
      this.shadowRoot;
    }

    get shadowRoot() {
      if (!super.shadowRoot.firstChild) {
        // We generate shadow DOM lazily on popupshowing event to avoid extra
        // load on the system during browser startup.
        super.shadowRoot.appendChild(this.fragment);
        this.initShadowDOM();
      }
      return super.shadowRoot;
    }

    get fragment() {
      if (!this.constructor.hasOwnProperty("_fragment")) {
        this.constructor._fragment = MozXULElement.parseXULToFragment(
          this.markup
        );
      }
      return document.importNode(this.constructor._fragment, true);
    }

    get markup() {
      return `
        <html:link rel="stylesheet" href="chrome://global/skin/global.css"/>
        <html:link rel="stylesheet" href="chrome://global/content/elements/menupopup.css"/>
        <arrowscrollbox class="menupopup-arrowscrollbox"
                        part="arrowscrollbox content"
                        exportparts="scrollbox: arrowscrollbox-scrollbox"
                        flex="1"
                        orient="vertical"
                        smoothscroll="false">
          <html:slot></html:slot>
        </arrowscrollbox>
      `;
    }

    get scrollBox() {
      if (!this._scrollBox) {
        this._scrollBox = this.shadowRoot.querySelector("arrowscrollbox");
      }
      return this._scrollBox;
    }

    /**
     * Adds event listeners for a MozMenuPopup inside a menulist element.
     */
    _setUpMenulistPopup() {
      // Access shadow root to generate menupoup shadow DOMs. We do generate
      // shadow DOM on popupshowing, but it doesn't work for HTML:selects,
      // which are implemented via menulist elements living in the main process.
      // So make them a special case then.
      this.ensureInitialized();
      this.classList.add("in-menulist");

      this.addEventListener("popupshown", () => {
        // Enable drag scrolling even when the mouse wasn't used. The
        // mousemove handler will remove it if the mouse isn't down.
        this._enableDragScrolling(false);
      });

      this.addEventListener("popuphidden", () => {
        this._draggingState = this.NOT_DRAGGING;
        this._clearScrollTimer();
        this.releaseCapture();
        this.scrollBox.scrollbox.scrollTop = 0;
      });

      this.addEventListener("mousedown", event => {
        if (event.button != 0) {
          return;
        }

        if (
          this.state == "open" &&
          (event.target.localName == "menuitem" ||
            event.target.localName == "menu" ||
            event.target.localName == "menucaption")
        ) {
          this._enableDragScrolling(true);
        }
      });

      this.addEventListener("mouseup", event => {
        if (event.button != 0) {
          return;
        }

        this._draggingState = this.NOT_DRAGGING;
        this._clearScrollTimer();
      });

      this.addEventListener("mousemove", event => {
        if (!this._draggingState) {
          return;
        }

        this._clearScrollTimer();

        // If the user released the mouse before the menupopup opens, we will
        // still be capturing, so check that the button is still pressed. If
        // not, release the capture and do nothing else. This also handles if
        // the dropdown was opened via the keyboard.
        if (!(event.buttons & 1)) {
          this._draggingState = this.NOT_DRAGGING;
          this.releaseCapture();
          return;
        }

        // If dragging outside the top or bottom edge of the menupopup, but
        // within the menupopup area horizontally, scroll the list in that
        // direction. The _draggingState flag is used to ensure that scrolling
        // does not start until the mouse has moved over the menupopup first,
        // preventing scrolling while over the dropdown button.
        let popupRect = this.getOuterScreenRect();
        if (
          event.screenX >= popupRect.left &&
          event.screenX <= popupRect.right
        ) {
          if (this._draggingState == this.DRAG_OVER_BUTTON) {
            if (
              event.screenY > popupRect.top &&
              event.screenY < popupRect.bottom
            ) {
              this._draggingState = this.DRAG_OVER_POPUP;
            }
          }

          if (
            this._draggingState == this.DRAG_OVER_POPUP &&
            (event.screenY <= popupRect.top ||
              event.screenY >= popupRect.bottom)
          ) {
            let scrollAmount = event.screenY <= popupRect.top ? -1 : 1;
            this.scrollBox.scrollByIndex(scrollAmount, true);

            let win = this.ownerGlobal;
            this._scrollTimer = win.setInterval(() => {
              this.scrollBox.scrollByIndex(scrollAmount, true);
            }, this.AUTOSCROLL_INTERVAL);
          }
        }
      });

      this._menulistPopupIsSetUp = true;
    }

    _enableDragScrolling(overItem) {
      if (!this._draggingState) {
        this.setCaptureAlways();
        this._draggingState = overItem
          ? this.DRAG_OVER_POPUP
          : this.DRAG_OVER_BUTTON;
      }
    }

    _clearScrollTimer() {
      if (this._scrollTimer) {
        this.ownerGlobal.clearInterval(this._scrollTimer);
        this._scrollTimer = 0;
      }
    }

    on_DOMMenuItemActive(event) {
      // Scroll buttons may overlap the active item. In that case, scroll
      // further to stay clear of the buttons.
      if (
        this.parentNode?.localName == "menulist" ||
        !this.scrollBox.overflowing
      ) {
        return;
      }
      let item = event.target;
      if (item.parentNode != this) {
        return;
      }
      let itemRect = item.getBoundingClientRect();
      let buttonRect = this.scrollBox._scrollButtonUp.getBoundingClientRect();
      if (buttonRect.bottom > itemRect.top) {
        this.scrollBox.scrollByPixels(itemRect.top - buttonRect.bottom, true);
      } else {
        buttonRect = this.scrollBox._scrollButtonDown.getBoundingClientRect();
        if (buttonRect.top < itemRect.bottom) {
          this.scrollBox.scrollByPixels(itemRect.bottom - buttonRect.top, true);
        }
      }
    }
  }

  customElements.define("menupopup", MozMenuPopup);

  MozElements.MozMenuPopup = MozMenuPopup;
}
