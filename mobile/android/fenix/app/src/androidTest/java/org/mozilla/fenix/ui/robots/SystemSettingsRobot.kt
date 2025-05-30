/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.fenix.ui.robots

import android.util.Log
import androidx.test.espresso.intent.Intents
import androidx.test.espresso.intent.matcher.IntentMatchers.hasAction
import androidx.test.uiautomator.UiSelector
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.mozilla.fenix.helpers.Constants.TAG
import org.mozilla.fenix.helpers.MatcherHelper.itemWithResIdAndDescription
import org.mozilla.fenix.helpers.TestHelper.appName
import org.mozilla.fenix.helpers.TestHelper.mDevice
import org.mozilla.fenix.helpers.TestHelper.waitForAppWindowToBeUpdated

class SystemSettingsRobot {

    fun verifyNotifications() {
        waitForAppWindowToBeUpdated()
        Log.i(TAG, "verifyNotifications: Trying to verify the intent to the notifications settings")
        Intents.intended(hasAction("android.settings.APP_NOTIFICATION_SETTINGS"))
        Log.i(TAG, "verifyNotifications: Verified the intent to the notifications settings")
    }

    fun verifyMakeDefaultBrowser() {
        waitForAppWindowToBeUpdated()
        Log.i(TAG, "verifyMakeDefaultBrowser: Trying to verify the intent to the default apps settings")
        Intents.intended(hasAction(SettingsRobot.DEFAULT_APPS_SETTINGS_ACTION))
        Log.i(TAG, "verifyMakeDefaultBrowser: Verified the intent to the default apps settings")
    }

    fun verifyAllSystemNotificationsToggleState(enabled: Boolean) {
        if (enabled) {
            Log.i(TAG, "verifyAllSystemNotificationsToggleState: Trying to verify that the system settings \"All Fenix notifications\" toggle is checked")
            assertTrue("$TAG: The system settings \"Show notifications\" toggle is not checked", systemSettingsAllFenixNotificationsToggle().isChecked)
            Log.i(TAG, "verifyAllSystemNotificationsToggleState: Verified that the system settings \"Show notifications\" toggle is checked")
        } else {
            Log.i(TAG, "verifyAllSystemNotificationsToggleState: Trying to verify that the system settings \"All Fenix notifications\" toggle is not checked")
            assertFalse("$TAG: The system settings \"Show notifications\" toggle is checked", systemSettingsAllFenixNotificationsToggle().isChecked)
            Log.i(TAG, "verifyAllSystemNotificationsToggleState: Verified that the system settings \"Show notifications\" toggle is not checked")
        }
    }

    fun verifyPrivateBrowsingSystemNotificationsToggleState(enabled: Boolean) {
        if (enabled) {
            Log.i(TAG, "verifyPrivateBrowsingSystemNotificationsToggleState: Trying to verify that the system settings \"Private browsing session\" toggle is checked")
            assertTrue("$TAG: The system settings \"Private browsing sessio\" toggle is not checked", privateBrowsingSystemSettingsNotificationsToggle().isChecked)
            Log.i(TAG, "verifyPrivateBrowsingSystemNotificationsToggleState: Verified that the system settings \"Private browsing session\" toggle is checked")
        } else {
            Log.i(TAG, "verifyPrivateBrowsingSystemNotificationsToggleState: Trying to verify that the system settings \"Private browsing session\" toggle is not checked")
            assertFalse("$TAG: The system settings \"Private browsing session\" toggle is checked", privateBrowsingSystemSettingsNotificationsToggle().isChecked)
            Log.i(TAG, "verifyPrivateBrowsingSystemNotificationsToggleState: Verified that the system settings \"Private browsing session\" toggle is not checked")
        }
    }

    fun clickPrivateBrowsingSystemNotificationsToggle() {
        Log.i(TAG, "clickPrivateBrowsingSystemNotificationsToggle: Trying to click the system settings \"Private browsing session\" toggle")
        privateBrowsingSystemSettingsNotificationsToggle().click()
        Log.i(TAG, "clickPrivateBrowsingSystemNotificationsToggle: Clicked the system settings \"Private browsing session\" toggle")
    }

    fun clickAllSystemNotificationsToggle() {
        Log.i(TAG, "clickAllSystemNotificationsToggle: Trying to click the system settings \"Show notifications\" toggle")
        systemSettingsAllFenixNotificationsToggle().click()
        Log.i(TAG, "clickAllSystemNotificationsToggle: Clicked the system settings \"Show notifications\" toggle")
    }

    class Transition {
        // Difficult to know where this will go
        fun goBack(interact: SettingsRobot.() -> Unit): SettingsRobot.Transition {
            Log.i(TAG, "goBack: Trying to click device back button")
            mDevice.pressBack()
            Log.i(TAG, "goBack: Clicked device back button")

            SettingsRobot().interact()
            return SettingsRobot.Transition()
        }
    }
}

fun systemSettings(interact: SystemSettingsRobot.() -> Unit): SystemSettingsRobot.Transition {
    SystemSettingsRobot().interact()
    return SystemSettingsRobot.Transition()
}

private fun systemSettingsAllFenixNotificationsToggle() =
    mDevice.findObject(
        UiSelector().text("All $appName notifications"),
    ).getFromParent(UiSelector().resourceId("android:id/switch_widget"))

private fun privateBrowsingSystemSettingsNotificationsToggle() =
    itemWithResIdAndDescription("com.android.settings:id/switchWidget", "Private browsing session")
