/* vim:set ts=2 sw=2 sts=2 et: */
/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include "core/TelemetryScalar.h"
#include "gtest/gtest.h"
#include "js/Conversions.h"
#include "js/PropertyAndElement.h"  // JS_GetProperty, JS_HasProperty
#include "mozilla/Telemetry.h"
#include "mozilla/TelemetryProcessEnums.h"
#include "mozilla/Unused.h"
#include "nsJSUtils.h"  // nsAutoJSString
#include "nsThreadUtils.h"
#include "TelemetryFixture.h"
#include "TelemetryTestHelpers.h"
#include "mozilla/glean/GleanTestsTestMetrics.h"
#include "mozilla/glean/fog_ffi_generated.h"

using namespace mozilla;
using namespace TelemetryTestHelpers;
using namespace mozilla::glean;
using namespace mozilla::glean::impl;
using mozilla::Telemetry::ProcessID;

#define EXPECTED_STRING "Nice, expected and creative string."

// Test that we can properly write unsigned scalars using the C++ API.
TEST_F(TelemetryTestFixture, ScalarUnsigned) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

  // Set the test scalar to a known value.
  const uint32_t kInitialValue = 1172015;
  const uint32_t kExpectedUint = 1172017;
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_UNSIGNED_INT_KIND,
                       kInitialValue);
  TelemetryScalar::Add(Telemetry::ScalarID::TELEMETRY_TEST_UNSIGNED_INT_KIND,
                       kExpectedUint - kInitialValue);

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
  CheckUintScalar("telemetry.test.unsigned_int_kind", cx.GetJSContext(),
                  scalarsSnapshot, kExpectedUint);

// Make sure that calls of the unsupported type don't corrupt the stored value.
// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_UNSIGNED_INT_KIND,
                       false);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_UNSIGNED_INT_KIND,
                       u"test"_ns);
#endif

  // Check the recorded value.
  GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
  CheckUintScalar("telemetry.test.unsigned_int_kind", cx.GetJSContext(),
                  scalarsSnapshot, kExpectedUint);
}

// Test that we can properly write boolean scalars using the C++ API.
TEST_F(TelemetryTestFixture, ScalarBoolean) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  Unused << mTelemetry->ClearScalars();

  // Set the test scalar to a known value.
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_BOOLEAN_KIND, true);

// Make sure that calls of the unsupported type don't corrupt the stored value.
// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_BOOLEAN_KIND,
                       static_cast<uint32_t>(12));
  TelemetryScalar::Add(Telemetry::ScalarID::TELEMETRY_TEST_BOOLEAN_KIND, 2);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_BOOLEAN_KIND,
                       u"test"_ns);
#endif

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
  CheckBoolScalar("telemetry.test.boolean_kind", cx.GetJSContext(),
                  scalarsSnapshot, true);
}

// Test that we can properly write string scalars using the C++ API.
TEST_F(TelemetryTestFixture, ScalarString) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  Unused << mTelemetry->ClearScalars();

  // Set the test scalar to a known value.
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_STRING_KIND,
                       NS_LITERAL_STRING_FROM_CSTRING(EXPECTED_STRING));

// Make sure that calls of the unsupported type don't corrupt the stored value.
// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_STRING_KIND,
                       static_cast<uint32_t>(12));
  TelemetryScalar::Add(Telemetry::ScalarID::TELEMETRY_TEST_STRING_KIND, 2);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_STRING_KIND, true);
#endif

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
  CheckStringScalar("telemetry.test.string_kind", cx.GetJSContext(),
                    scalarsSnapshot, EXPECTED_STRING);
}

// Test that we can properly write keyed unsigned scalars using the C++ API.
TEST_F(TelemetryTestFixture, KeyedScalarUnsigned) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  Unused << mTelemetry->ClearScalars();

  // Set the test scalar to a known value.
  const char* kScalarName = "telemetry.test.keyed_unsigned_int";
  const uint32_t kKey1Value = 1172015;
  const uint32_t kKey2Value = 1172017;
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"key1"_ns, kKey1Value);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"key2"_ns, kKey1Value);
  TelemetryScalar::Add(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"key2"_ns, 2);

// Make sure that calls of the unsupported type don't corrupt the stored value.
// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"key1"_ns, false);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"test"_ns);
#endif

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  // Check the keyed scalar we're interested in.
  CheckKeyedUintScalar(kScalarName, "key1", cx.GetJSContext(), scalarsSnapshot,
                       kKey1Value);
  CheckKeyedUintScalar(kScalarName, "key2", cx.GetJSContext(), scalarsSnapshot,
                       kKey2Value);
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 2);
}

TEST_F(TelemetryTestFixture, KeyedScalarBoolean) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  Unused << mTelemetry->ClearScalars();

  // Set the test scalar to a known value.
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_BOOLEAN_KIND,
                       u"key1"_ns, false);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_BOOLEAN_KIND,
                       u"key2"_ns, true);

// Make sure that calls of the unsupported type don't corrupt the stored value.
// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_BOOLEAN_KIND,
                       u"key1"_ns, static_cast<uint32_t>(12));
  TelemetryScalar::Add(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_BOOLEAN_KIND,
                       u"key1"_ns, 2);
#endif

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  // Make sure that the keys contain the expected values.
  const char* kScalarName = "telemetry.test.keyed_boolean_kind";
  CheckKeyedBoolScalar(kScalarName, "key1", cx.GetJSContext(), scalarsSnapshot,
                       false);
  CheckKeyedBoolScalar(kScalarName, "key2", cx.GetJSContext(), scalarsSnapshot,
                       true);
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 2);
}

TEST_F(TelemetryTestFixture, NonMainThreadAdd) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  Unused << mTelemetry->ClearScalars();

  // Define the function that will be called on the testing thread.
  nsCOMPtr<nsIRunnable> runnable = NS_NewRunnableFunction(
      "TelemetryTestFixture_NonMainThreadAdd_Test::TestBody", []() -> void {
        TelemetryScalar::Add(
            Telemetry::ScalarID::TELEMETRY_TEST_UNSIGNED_INT_KIND, 37);
      });

  // Spawn the testing thread and run the function.
  nsCOMPtr<nsIThread> testingThread;
  nsresult rv =
      NS_NewNamedThread("Test thread", getter_AddRefs(testingThread), runnable);
  ASSERT_EQ(rv, NS_OK);

  // Shutdown the thread. This also waits for the runnable to complete.
  testingThread->Shutdown();

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
  CheckUintScalar("telemetry.test.unsigned_int_kind", cx.GetJSContext(),
                  scalarsSnapshot, 37);
}

TEST_F(TelemetryTestFixture, ScalarUnknownID) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  const uint32_t kTestFakeIds[] = {
      static_cast<uint32_t>(Telemetry::ScalarID::ScalarCount),
      static_cast<uint32_t>(Telemetry::ScalarID::ScalarCount) + 378537,
      std::numeric_limits<uint32_t>::max()};

  for (auto id : kTestFakeIds) {
    Telemetry::ScalarID scalarId = static_cast<Telemetry::ScalarID>(id);
    TelemetryScalar::Set(scalarId, static_cast<uint32_t>(1));
    TelemetryScalar::Set(scalarId, true);
    TelemetryScalar::Set(scalarId, u"test"_ns);
    TelemetryScalar::Add(scalarId, 1);

    // Make sure that nothing was recorded in the plain scalars.
    JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
    GetScalarsSnapshot(false, cx.GetJSContext(), &scalarsSnapshot);
    ASSERT_TRUE(scalarsSnapshot.isUndefined())
    << "No scalar must be recorded";

    // Same for the keyed scalars.
    TelemetryScalar::Set(scalarId, u"key1"_ns, static_cast<uint32_t>(1));
    TelemetryScalar::Set(scalarId, u"key1"_ns, true);
    TelemetryScalar::Add(scalarId, u"key1"_ns, 1);

    // Make sure that nothing was recorded in the keyed scalars.
    JS::Rooted<JS::Value> keyedSnapshot(cx.GetJSContext());
    GetScalarsSnapshot(true, cx.GetJSContext(), &keyedSnapshot);
    ASSERT_TRUE(keyedSnapshot.isUndefined())
    << "No keyed scalar must be recorded";
  }
#endif
}

TEST_F(TelemetryTestFixture, ScalarEventSummary) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

  const char* kScalarName = "telemetry.event_counts";

  const char* kLongestEvent =
      "oohwowlookthiscategoryissolong#thismethodislongtooo#"
      "thisobjectisnoslouch";
  TelemetryScalar::SummarizeEvent(nsCString(kLongestEvent), ProcessID::Parent);

  // Check the recorded value.
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  CheckKeyedUintScalar(kScalarName, kLongestEvent, cx.GetJSContext(),
                       scalarsSnapshot, 1);

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  const char* kTooLongEvent =
      "oohwowlookthiscategoryissolong#thismethodislongtooo#"
      "thisobjectisnoslouch2";
  TelemetryScalar::SummarizeEvent(nsCString(kTooLongEvent), ProcessID::Parent);

  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 1);
#endif  // #ifndef DEBUG

  // Test we can fill the next 499 keys up to our 500 maximum
  for (int i = 1; i < 500; i++) {
    std::ostringstream eventName;
    eventName << "category#method#object" << i;
    TelemetryScalar::SummarizeEvent(nsCString(eventName.str().c_str()),
                                    ProcessID::Parent);
  }

  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 500);

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  TelemetryScalar::SummarizeEvent(nsCString("whoops#too#many"),
                                  ProcessID::Parent);

  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 500);
#endif  // #ifndef DEBUG
}

TEST_F(TelemetryTestFixture, TestKeyedScalarAllowedKeys) {
  AutoJSContextWithGlobal cx(mCleanGlobal);
  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

  const uint32_t kExpectedUint = 1172017;

  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"only"_ns, kExpectedUint);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"meant"_ns, kExpectedUint);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"for"_ns, kExpectedUint);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"testing"_ns, kExpectedUint);

  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"invalid"_ns, kExpectedUint);
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"not-valid"_ns, kExpectedUint);

  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar("telemetry.test.keyed_with_keys", "only",
                       cx.GetJSContext(), scalarsSnapshot, kExpectedUint);
  CheckKeyedUintScalar("telemetry.test.keyed_with_keys", "meant",
                       cx.GetJSContext(), scalarsSnapshot, kExpectedUint);
  CheckKeyedUintScalar("telemetry.test.keyed_with_keys", "for",
                       cx.GetJSContext(), scalarsSnapshot, kExpectedUint);
  CheckKeyedUintScalar("telemetry.test.keyed_with_keys", "testing",
                       cx.GetJSContext(), scalarsSnapshot, kExpectedUint);
  CheckNumberOfProperties("telemetry.test.keyed_with_keys", cx.GetJSContext(),
                          scalarsSnapshot, 4);

  CheckKeyedUintScalar("telemetry.keyed_scalars_unknown_keys",
                       "telemetry.test.keyed_with_keys", cx.GetJSContext(),
                       scalarsSnapshot, 2);
}

// Test that we can properly handle too long key.
TEST_F(TelemetryTestFixture, TooLongKey) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  const char* kScalarName = "telemetry.test.keyed_unsigned_int";
  const uint32_t kKey1Value = 1172015;

  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u"123456789012345678901234567890123456789012345678901234"
                       u"5678901234567890morethanseventy"_ns,
                       kKey1Value);

  const uint32_t kDummyUint = 1172017;

  // add dummy value so that object can be created from snapshot
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"dummy"_ns, kDummyUint);
  // Check the recorded value
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  bool foundp = true;
  JS::Rooted<JSObject*> scalarObj(cx.GetJSContext(),
                                  &scalarsSnapshot.toObject());
  ASSERT_TRUE(
      JS_HasProperty(cx.GetJSContext(), scalarObj, kScalarName, &foundp));
  EXPECT_FALSE(foundp);
#endif  // #ifndef DEBUG
}

// Test that we can properly handle empty key
TEST_F(TelemetryTestFixture, EmptyKey) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  const char* kScalarName = "telemetry.test.keyed_unsigned_int";
  const uint32_t kKey1Value = 1172015;

  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                       u""_ns, kKey1Value);

  const uint32_t kDummyUint = 1172017;

  // add dummy value so that object can be created from snapshot
  TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_WITH_KEYS,
                       u"dummy"_ns, kDummyUint);

  // Check the recorded value
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  bool foundp = true;
  JS::Rooted<JSObject*> scalarObj(cx.GetJSContext(),
                                  &scalarsSnapshot.toObject());
  ASSERT_TRUE(
      JS_HasProperty(cx.GetJSContext(), scalarObj, kScalarName, &foundp));
  EXPECT_FALSE(foundp);
#endif  // #ifndef DEBUG
}

// Test that we can properly handle too many keys
TEST_F(TelemetryTestFixture, TooManyKeys) {
  AutoJSContextWithGlobal cx(mCleanGlobal);

  // Make sure we don't get scalars from other tests.
  Unused << mTelemetry->ClearScalars();

// Don't run this part in debug builds as that intentionally asserts.
#ifndef DEBUG
  const char* kScalarName = "telemetry.test.keyed_unsigned_int";
  const uint32_t kKey1Value = 1172015;

  for (int i = 0; i < 150; i++) {
    std::u16string key = u"key";
    char16_t n = i + '0';
    key.push_back(n);
    TelemetryScalar::Set(Telemetry::ScalarID::TELEMETRY_TEST_KEYED_UNSIGNED_INT,
                         nsString(key.c_str()), kKey1Value);
  }

  // Check the recorded value
  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);

  // Check 100 keys are present.
  CheckNumberOfProperties(kScalarName, cx.GetJSContext(), scalarsSnapshot, 100);
#endif  // #ifndef DEBUG
}

TEST_F(TelemetryTestFixture, GleanLabeledGifft) {
  AutoJSContextWithGlobal cx(mCleanGlobal);
  // Need to test-reset Glean so it's working
  nsCString empty;
  ASSERT_EQ(NS_OK, fog_test_reset(&empty, &empty));

  ASSERT_EQ(mozilla::Nothing(),
            test_only_ipc::a_labeled_counter.Get("hot_air"_ns)
                .TestGetValue()
                .unwrap());

  const char* kScalarName = "telemetry.test.another_mirror_for_labeled_counter";
  const uint32_t kExpectedUint = 1172017;
  const int32_t kExpectedInt = (int32_t)1172017;

  test_only_ipc::a_labeled_counter.Get("hot_air"_ns).Add(kExpectedInt);
  ASSERT_EQ(kExpectedInt, test_only_ipc::a_labeled_counter.Get("hot_air"_ns)
                              .TestGetValue()
                              .unwrap()
                              .ref());

  JS::Rooted<JS::Value> scalarsSnapshot(cx.GetJSContext());
  GetScalarsSnapshot(true, cx.GetJSContext(), &scalarsSnapshot);
  CheckKeyedUintScalar(kScalarName, "hot_air", cx.GetJSContext(),
                       scalarsSnapshot, kExpectedUint);
}
