/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

const Arithmetic = ChromeUtils.importESModule(
  "resource://gre/modules/RustArithmetic.sys.mjs"
);
const Geometry = ChromeUtils.importESModule(
  "resource://gre/modules/RustGeometry.sys.mjs"
);
const TodoList = ChromeUtils.importESModule(
  "resource://gre/modules/RustTodolist.sys.mjs"
);
const Rondpoint = ChromeUtils.importESModule(
  "resource://gre/modules/RustRondpoint.sys.mjs"
);
const { UniFFITypeError } = ChromeUtils.importESModule(
  "resource://gre/modules/UniFFI.sys.mjs"
);

add_task(async function testFunctionArguments() {
  await Assert.rejects(
    Arithmetic.add(2),
    UniFFITypeError,
    "add() call missing argument"
  );
  Assert.throws(
    () => new Geometry.Point(0.0),
    UniFFITypeError,
    "Point constructor missing argument"
  );
});

add_task(async function testNumberOverflow() {
  // MAX_SAFE_INTEGER + 1 fits in a Rust u64, but overflows a JS Number.
  // This overflow should be caught by the C++ code.
  await Assert.rejects(
    Arithmetic.add(Number.MAX_SAFE_INTEGER, 1),
    /RangeError/,
    "add() call that overflows Number.MAX_SAFE_INTEGER"
  );
});

add_task(async function testObjectPointers() {
  const todo = await TodoList.TodoList.init();
  const stringifier = await Rondpoint.Stringifier.init();
  await todo.getEntries(); // OK
  todo[TodoList.UnitTestObjs.uniffiObjectPtr] =
    stringifier[Rondpoint.UnitTestObjs.uniffiObjectPtr];

  await Assert.rejects(
    todo.getEntries(), // the pointer is incorrect, should throw
    /Incorrect UniFFI pointer type/,
    "getEntries() with wrong pointer type"
  );

  await Assert.rejects(
    TodoList.setDefaultList(1), // expecting an object
    /Object is not a 'TodoList' instance/,
    "attempting to lift the wrong object type"
  );
});

add_task(async function testEnumTypeCheck() {
  await Assert.rejects(
    Rondpoint.copieEnumeration("invalid"), // Not an integer value
    /e:/, // Ensure exception message includes the argument name
    "copieEnumeration() with non-Enumeration value should throw"
  );
  await Assert.rejects(
    Rondpoint.copieEnumeration(99), // Integer, but doesn't map to a variant
    /e:/, // Ensure exception message includes the argument name
    "copieEnumeration() with non-Enumeration value should throw"
  );
  await Assert.rejects(
    Rondpoint.copieEnumeration(4), // Integer, but doesn't map to a variant
    /e:/, // Ensure exception message includes the argument name
    "copieEnumeration() with non-Enumeration value should throw"
  );
});

add_task(async function testRecordTypeCheck() {
  await Assert.rejects(
    Geometry.gradient(123), // Not a Line object
    UniFFITypeError,
    "gradient with non-Line object should throw"
  );
});

add_task(async function testOptionTypeCheck() {
  const optionneur = await Rondpoint.Optionneur.init();
  await Assert.rejects(
    optionneur.sinonNull(0),
    UniFFITypeError,
    "sinonNull with non-string should throw"
  );
});

add_task(async function testSequenceTypeCheck() {
  const todo = await TodoList.TodoList.init();
  await Assert.rejects(
    todo.addEntries("not a list"),
    UniFFITypeError,
    "addEntries with non-list should throw"
  );

  await Assert.rejects(
    todo.addEntries(["not TodoEntry"]),
    /entries\[0]/,
    "addEntries with non TodoEntry item should throw"
  );
});

add_task(async function testMapTypeCheck() {
  await Assert.rejects(
    Rondpoint.copieCarte("not a map"),
    UniFFITypeError,
    "copieCarte with a non-map should throw"
  );

  await Assert.rejects(
    Rondpoint.copieCarte({ x: 1 }),
    /c\[x]/,
    "copieCarte with a wrong value type should throw"
  );

  // TODO: test key types once we implement https://bugzilla.mozilla.org/show_bug.cgi?id=1809459
});
