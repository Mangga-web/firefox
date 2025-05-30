/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebTransportSessionBase.h"

#include "nsIWebTransport.h"

namespace mozilla::net {

void WebTransportSessionBase::SetWebTransportSessionEventListener(
    WebTransportSessionEventListener* listener) {
  mListener = listener;
}

}  // namespace mozilla::net
