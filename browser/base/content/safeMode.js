/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const appStartup = Services.startup;

const { ResetProfile } = ChromeUtils.importESModule(
  "resource://gre/modules/ResetProfile.sys.mjs"
);

var defaultToReset = false;

function showResetDialog() {
  // Prompt the user to confirm the reset.
  let retVals = {
    reset: false,
  };
  window.openDialog(
    "chrome://global/content/resetProfile.xhtml",
    null,
    "chrome,modal,centerscreen,titlebar,dialog=yes",
    retVals
  );
  if (!retVals.reset) {
    return;
  }

  ResetProfile.doReset();
}

function onDefaultButton(event) {
  if (defaultToReset) {
    // Prevent starting into safe mode while restarting.
    event.preventDefault();
    // Restart to reset the profile.
    ResetProfile.doReset();
  }
  // Dialog will be closed by default Event handler.
  // Continue in safe mode. No restart needed.
}

function onCancel() {
  appStartup.quit(appStartup.eForceQuit);
}

function onExtra1() {
  if (defaultToReset) {
    // Continue in safe mode
    window.close();
  }
  // The reset dialog will handle starting the reset process if the user confirms.
  showResetDialog();
}

window.addEventListener("load", () => {
  const dialog = document.getElementById("safeModeDialog");
  if (appStartup.automaticSafeModeNecessary) {
    document.getElementById("autoSafeMode").hidden = false;
    document.getElementById("safeMode").hidden = true;
    if (ResetProfile.resetSupported()) {
      document.getElementById("resetProfile").hidden = false;
    } else {
      // Hide the reset button is it's not supported.
      dialog.getButton("extra1").hidden = true;
    }
  } else if (!ResetProfile.resetSupported()) {
    // Hide the reset button and text if it's not supported.
    dialog.getButton("extra1").hidden = true;
    document.getElementById("resetProfileInstead").hidden = true;
  }
  document.addEventListener("dialogaccept", onDefaultButton);
  document.addEventListener("dialogcancel", onCancel);
  document.addEventListener("dialogextra1", onExtra1);
});
