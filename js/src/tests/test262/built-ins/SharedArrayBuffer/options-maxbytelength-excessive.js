// |reftest| skip-if(!this.hasOwnProperty('SharedArrayBuffer')) -- SharedArrayBuffer is not enabled unconditionally
// Copyright (C) 2021 the V8 project authors. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
esid: sec-sharedarraybuffer-constructor
description: |
  Invoked with an options object whose `maxByteLength` property exceeds the
  maximum length value
info: |
  SharedArrayBuffer( length [ , options ] )

  1. If NewTarget is undefined, throw a TypeError exception.
  2. Let byteLength be ? ToIndex(length).
  3. Let requestedMaxByteLength be ? GetArrayBufferMaxByteLengthOption(options).
  [...]

  1.1.5 GetArrayBufferMaxByteLengthOption ( options )

  1. If Type(options) is not Object, return empty.
  2. Let maxByteLength be ? Get(options, "maxByteLength").
  3. If maxByteLength is undefined, return empty.
  4. Return ? ToIndex(maxByteLength).
features: [SharedArrayBuffer, resizable-arraybuffer]
---*/

assert.throws(RangeError, function() {
  // Math.pow(2, 53) = 9007199254740992
  new SharedArrayBuffer(0, { maxByteLength: 9007199254740992 });
});

reportCompare(0, 0);
