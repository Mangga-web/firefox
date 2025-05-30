/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsThreadUtils.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/RootedDictionary.h"
#include "mozilla/uniffi/Call.h"
#include "mozilla/uniffi/ResultPromise.h"

namespace mozilla::uniffi {
extern mozilla::LazyLogModule gUniffiLogger;

using dom::GlobalObject;
using dom::OwningUniFFIScaffoldingValue;
using dom::RootedDictionary;
using dom::Sequence;
using dom::UniFFIScaffoldingCallResult;

void UniffiSyncCallHandler::CallSync(
    UniquePtr<UniffiSyncCallHandler> aHandler, const GlobalObject& aGlobal,
    const Sequence<OwningUniFFIScaffoldingValue>& aArgs,
    RootedDictionary<UniFFIScaffoldingCallResult>& aReturnValue,
    ErrorResult& aError) {
  MOZ_ASSERT(NS_IsMainThread());
  aHandler->LowerRustArgs(aArgs, aError);
  if (aError.Failed()) {
    return;
  }
  RustCallStatus callStatus{};
  aHandler->MakeRustCall(&callStatus);
  aHandler->mUniffiCallStatusCode = callStatus.code;
  if (callStatus.error_buf.data) {
    aHandler->mUniffiCallStatusErrorBuf =
        FfiValueRustBuffer(callStatus.error_buf);
  }
  aHandler->LiftCallResult(aGlobal.Context(), aReturnValue, aError);
}

already_AddRefed<dom::Promise> UniffiSyncCallHandler::CallAsyncWrapper(
    UniquePtr<UniffiSyncCallHandler> aHandler, const dom::GlobalObject& aGlobal,
    const dom::Sequence<dom::OwningUniFFIScaffoldingValue>& aArgs,
    ErrorResult& aError) {
  MOZ_ASSERT(NS_IsMainThread());
  aHandler->LowerRustArgs(aArgs, aError);
  if (aError.Failed()) {
    return nullptr;
  }

  uniffi::ResultPromise resultPromise;
  resultPromise.Init(aGlobal, aError);
  if (aError.Failed()) {
    return nullptr;
  }

  nsresult dispatchResult = NS_DispatchBackgroundTask(
      NS_NewRunnableFunction("UniffiSyncCallHandler::CallAsyncWrapper",
                             [handler = std::move(aHandler),
                              resultPromise = resultPromise]() mutable {
                               RustCallStatus callStatus{};
                               handler->MakeRustCall(&callStatus);
                               handler->mUniffiCallStatusCode = callStatus.code;
                               if (callStatus.error_buf.data) {
                                 handler->mUniffiCallStatusErrorBuf =
                                     FfiValueRustBuffer(callStatus.error_buf);
                               }

                               resultPromise.Complete(std::move(handler));
                             }),
      NS_DISPATCH_EVENT_MAY_BLOCK);
  if (NS_FAILED(dispatchResult)) {
    MOZ_LOG(gUniffiLogger, LogLevel::Error,
            ("[UniFFI] NS_DispatchBackgroundTask failed"));
    resultPromise.RejectWithUnexpectedError();
  }

  return do_AddRef(resultPromise.GetPromise());
}

void UniffiCallHandlerBase::LiftCallResult(
    JSContext* aCx,
    dom::RootedDictionary<dom::UniFFIScaffoldingCallResult>& aDest,
    ErrorResult& aError) {
  switch (mUniffiCallStatusCode) {
    case RUST_CALL_SUCCESS: {
      aDest.mCode = dom::UniFFIScaffoldingCallCode::Success;
      LiftSuccessfulCallResult(aCx, aDest.mData, aError);
      break;
    }

    case RUST_CALL_ERROR: {
      // Rust Err() value.  Populate data with the `RustBuffer` containing the
      // error
      aDest.mCode = dom::UniFFIScaffoldingCallCode::Error;
      mUniffiCallStatusErrorBuf.Lift(aCx, &aDest.mData.Construct(), aError);
      break;
    }

    default: {
      // This indicates a RustError, which should rarely happen in practice.
      // The normal case is a Rust panic, but FF sets panic=abort.
      aDest.mCode = dom::UniFFIScaffoldingCallCode::Internal_error;
      if (mUniffiCallStatusErrorBuf.IsSet()) {
        mUniffiCallStatusErrorBuf.Lift(aCx, &aDest.mData.Construct(), aError);
      }

      break;
    }
  }
}

already_AddRefed<dom::Promise> UniffiAsyncCallHandler::CallAsync(
    UniquePtr<UniffiAsyncCallHandler> aHandler,
    const dom::GlobalObject& aGlobal,
    const dom::Sequence<dom::OwningUniFFIScaffoldingValue>& aArgs,
    ErrorResult& aError) {
  MOZ_ASSERT(NS_IsMainThread());
  // Async calls return a Future rather than doing any work.  This means we can
  // make the call right now on the JS main thread without fear of blocking it.
  aHandler->LowerArgsAndMakeRustCall(aArgs, aError);
  if (aError.Failed()) {
    return nullptr;
  }

  aHandler->mPromise.Init(aGlobal, aError);
  if (aError.Failed()) {
    return nullptr;
  }

  ResultPromise returnPromise(aHandler->mPromise);

  // Schedule a poll for the future in a background thread.
  nsresult dispatchResult = NS_DispatchBackgroundTask(
      NS_NewRunnableFunction("UniffiAsyncCallHandler::CallAsync",
                             [handler = std::move(aHandler)]() mutable {
                               UniffiAsyncCallHandler::Poll(std::move(handler));
                             }));
  if (NS_FAILED(dispatchResult)) {
    MOZ_LOG(gUniffiLogger, LogLevel::Error,
            ("[UniFFI] NS_DispatchBackgroundTask failed"));
    aHandler->mPromise.RejectWithUnexpectedError();
  }

  return do_AddRef(returnPromise.GetPromise());
}

// Callback function for async calls
//
// This is passed to Rust when we poll the future alongside a 64-bit handle that
// represents the callback data.  For uniffi-bindgen-gecko-js, the handle is a
// `UniffiAsyncCallHandler*` casted to an int.
//
// Rust calls this when either the future is ready or when it's time to poll it
// again.
void UniffiAsyncCallHandler::FutureCallback(uint64_t aCallHandlerHandle,
                                            int8_t aCode) {
  // Recreate the UniquePtr we previously released
  UniquePtr<UniffiAsyncCallHandler> handler(
      reinterpret_cast<UniffiAsyncCallHandler*>(
          static_cast<uintptr_t>(aCallHandlerHandle)));

  switch (aCode) {
    case UNIFFI_FUTURE_READY: {
      // `Future::poll` returned a `Ready` value on the Rust side.
      // Complete the future
      RustCallStatus callStatus{};
      handler->CallCompleteFn(&callStatus);
      handler->mUniffiCallStatusCode = callStatus.code;
      if (callStatus.error_buf.data) {
        handler->mUniffiCallStatusErrorBuf =
            FfiValueRustBuffer(callStatus.error_buf);
      }
      // Copy of the ResultPromise before moving the handler that contains it.
      ResultPromise promise(handler->mPromise);
      promise.Complete(std::move(handler));
      break;
    }

    case UNIFFI_FUTURE_MAYBE_READY: {
      // The Rust waker was invoked after `poll` returned a `Pending` value.
      // Poll the future again soon in a background task.

      nsresult dispatchResult =
          NS_DispatchBackgroundTask(NS_NewRunnableFunction(
              "UniffiAsyncCallHandler::FutureCallback",
              [handler = std::move(handler)]() mutable {
                UniffiAsyncCallHandler::Poll(std::move(handler));
              }));
      if (NS_FAILED(dispatchResult)) {
        MOZ_LOG(gUniffiLogger, LogLevel::Error,
                ("[UniFFI] NS_DispatchBackgroundTask failed"));
        handler->mPromise.RejectWithUnexpectedError();
      }
      break;
    }

    default: {
      // Invalid poll code, this should never happen, but if it does log an
      // error and reject the promise.
      MOZ_LOG(gUniffiLogger, LogLevel::Error,
              ("[UniFFI] Invalid poll code in "
               "UniffiAsyncCallHandler::FutureCallback %d",
               aCode));
      handler->mPromise.RejectWithUnexpectedError();
      break;
    }
  };
}

void UniffiAsyncCallHandler::Poll(UniquePtr<UniffiAsyncCallHandler> aHandler) {
  auto futureHandle = aHandler->mFutureHandle;
  auto pollFn = aHandler->mPollFn;
  // Release the UniquePtr into a raw pointer and convert it to a handle
  // so that we can pass it as a handle to the UniFFI code.  It gets converted
  // back in `UniffiAsyncCallHandler::FutureCallback()`, which the Rust code
  // guarentees will be called if the future makes progress.
  uint64_t selfHandle =
      static_cast<uint64_t>(reinterpret_cast<uintptr_t>(aHandler.release()));
  pollFn(futureHandle, UniffiAsyncCallHandler::FutureCallback, selfHandle);
}

UniffiAsyncCallHandler::~UniffiAsyncCallHandler() { mFreeFn(mFutureHandle); }

}  // namespace mozilla::uniffi
