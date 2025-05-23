/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

// Check expanding/collapsing object with symbol properties in the console.
const TEST_URI = "data:text/html;charset=utf8,<!DOCTYPE html>";

add_task(async function () {
  const hud = await openNewTabAndConsole(TEST_URI);

  await SpecialPowers.spawn(gBrowser.selectedBrowser, [], function () {
    class MyClass {
      constructor(isParent = true) {
        this.publicProperty = "public property";
        // A public property can start with a # character. Here we're
        // adding a public property that looks like an existing private property
        // to check we do render both.
        this["#privateProperty"] = {
          content: "actually this is a public property",
        };
        this[Symbol()] = "first Symbol";
        this[Symbol()] = "second Symbol";

        if (isParent) {
          this.#privateProperty = new MyClass(false);
        } else {
          this.#privateProperty = null;
        }
      }

      #privateProperty;
      // eslint-disable-next-line no-unused-private-class-members
      #privateMethod() {
        return Math.random();
      }
      // eslint-disable-next-line no-unused-private-class-members
      get #privateGetter() {
        return 42;
      }
      getPrivateProperty() {
        return this.#privateProperty;
      }
    }

    content.wrappedJSObject.console.log(
      "private-properties-test",
      new MyClass(true)
    );
  });

  const node = await waitFor(() =>
    findConsoleAPIMessage(hud, "private-properties-test")
  );
  const objectInspectors = [...node.querySelectorAll(".tree")];
  is(
    objectInspectors.length,
    1,
    "There is the expected number of object inspectors"
  );

  const [oi] = objectInspectors;

  info("Expanding the Object");
  await expandObjectInspectorNode(oi.querySelector(".tree-node"));

  const oiNodes = getObjectInspectorNodes(oi);
  // The object inspector should look like this:
  /*
   * ▼ { … }
   * |  ▶︎ "#privateProperty": Object { content: "actually this is a public property" }
   * |    publicProperty: "public property"
   * |    Symbol(): "first Symbol",
   * |    Symbol(): "second Symbol",
   * |  ▶︎ #privateGetter: undefined
   * |  ▶︎ #privateProperty: Object { publicProperty: "public property", "#privateProperty": { … }, #privateProperty: null, Symbol(): "first Symbol", … }
   * |  ▶︎ <prototype>
   */
  is(oiNodes.length, 8, "There is the expected number of nodes in the tree");

  const [
    publicDisguisedAsPrivateNodeEl,
    publicNodeEl,
    firstSymbolNodeEl,
    secondSymbolNodeEl,
    // FIXME: This shouldn't appear here (See Bug 1759823)
    privateGetterNodeEl,
    privatePropertyNodeEl,
  ] = Array.from(oiNodes).slice(1);

  checkOiNodeText(
    publicDisguisedAsPrivateNodeEl,
    `"#privateProperty": Object { content: "actually this is a public property" }`,
    `"fake" private property has expected text`
  );
  checkOiNodeText(
    publicNodeEl,
    `publicProperty: "public property"`,
    "public property has expected text"
  );
  checkOiNodeText(
    firstSymbolNodeEl,
    `Symbol(): "first Symbol"`,
    "first symbol has expected text"
  );
  checkOiNodeText(
    secondSymbolNodeEl,
    `Symbol(): "second Symbol"`,
    "second symbol has expected text"
  );
  checkOiNodeText(
    privateGetterNodeEl,
    `#privateGetter: undefined`,
    "private getter is rendered (at the wrong place with the wrong content, see Bug 1759823)"
  );
  checkOiNodeText(
    privatePropertyNodeEl,
    `#privateProperty: Object { publicProperty: "public property", "#privateProperty": {…}, #privateGetter: undefined, Symbol(): "first Symbol", … }`,
    "private property is rendered as expected"
  );

  info("Expand public property disguised as private property");
  await expandObjectInspectorNode(publicDisguisedAsPrivateNodeEl);
  const publicPropChildren = await waitFor(() => {
    const children = getObjectInspectorChildrenNodes(
      publicDisguisedAsPrivateNodeEl
    );
    if (children.length === 0) {
      return null;
    }
    return children;
  });
  ok(true, "public property was expanded");

  /*
   * ObjectInspector now should look like:
   *
   * ▼ { … }
   * |  ▼ "#privateProperty": { … }
   * |  |    content: "actually this is a public property"
   * |  |  ▶︎ <prototype>
   * |    publicProperty: "public property"
   * |    Symbol(): "first Symbol",
   * |    Symbol(): "second Symbol",
   * |  ▶︎ #privateProperty: Object { publicProperty: "public property", "#privateProperty": { … }, #privateProperty: null, Symbol(): "first Symbol", … }
   * |  ▶︎ <prototype>
   */
  checkOiNodeText(
    publicPropChildren[0],
    `content: "actually this is a public property"`,
    "public property child has expected text"
  );

  info("Expand private property");
  await expandObjectInspectorNode(privatePropertyNodeEl);
  const privatePropChildren = await waitFor(() => {
    const children = getObjectInspectorChildrenNodes(privatePropertyNodeEl);
    if (children.length === 0) {
      return null;
    }
    return children;
  });
  ok(true, "private property was expanded");

  /*
   * ObjectInspector now should look like:
   *
   * ▼ { … }
   * |  ▼ "#privateProperty": { … }
   * |  |    content: "actually this is a public property"
   * |  |  ▶︎ <prototype>
   * |    publicProperty: "public property"
   * |    Symbol(): "first Symbol",
   * |    Symbol(): "second Symbol",
   * |  ▶︎ #privateGetter: undefined
   * |  ▼ #privateProperty: { … }
   * |  |  ▶︎ "#privateProperty": Object { content: "actually this is a public property" }
   * |  |    publicProperty: "public property"
   * |  |    Symbol(): "first Symbol"
   * |  |    Symbol(): "second Symbol"
   * |  |  ▶︎ #privateGetter: undefined
   * |  |    #privateProperty: null
   * |  |  ▶︎ <prototype>
   * |  ▶︎ <prototype>
   */
  checkOiNodeText(
    privatePropChildren[0],
    `"#privateProperty": Object { content: "actually this is a public property" }`,
    "child private property has expected text "
  );
  checkOiNodeText(
    privatePropChildren[1],
    `publicProperty: "public property"`,
    "public property of private property object has expected text"
  );
  checkOiNodeText(
    privatePropChildren[2],
    `Symbol(): "first Symbol"`,
    "first symbol of private property object has expected text"
  );
  checkOiNodeText(
    privatePropChildren[3],
    `Symbol(): "second Symbol"`,
    "second symbol of private property object has expected text"
  );
  checkOiNodeText(
    privatePropChildren[4],
    `#privateGetter: undefined`,
    "private getter of private property object is displayed (even though it shouldn't, see Bug 1759823)"
  );
  checkOiNodeText(
    privatePropChildren[5],
    `#privateProperty: null`,
    "private property of private property object has expected text"
  );
  const privatePropertyPrototypeEl = privatePropChildren[6];
  checkOiNodeText(
    privatePropertyPrototypeEl,
    `<prototype>: Object { … }`,
    "prototype of private property object has expected text"
  );

  info("Expand private property prototype");
  await expandObjectInspectorNode(privatePropertyPrototypeEl);
  const privatePropertyPrototypeChildren = await waitFor(() => {
    const children = getObjectInspectorChildrenNodes(
      privatePropertyPrototypeEl
    );
    if (children.length === 0) {
      return null;
    }
    return children;
  });
  ok(true, "private property prototype was expanded");

  /*
   * ObjectInspector now should look like:
   *
   * ▼ { … }
   * |  ▼ "#privateProperty": { … }
   * |  |    content: "actually this is a public property"
   * |  |  ▶︎ <prototype>
   * |    publicProperty: "public property"
   * |    Symbol(): "first Symbol",
   * |    Symbol(): "second Symbol",
   * |  ▶︎ #privateGetter: undefined
   * |  ▼ #privateProperty: { … }
   * |  |  ▶︎ "#privateProperty": Object { content: "actually this is a public property" }
   * |  |    publicProperty: "public property"
   * |  |    Symbol(): "first Symbol"
   * |  |    Symbol(): "second Symbol"
   * |  |  ▶︎ #privateGetter: undefined
   * |  |    #privateProperty: null
   * |  |  ▼ <prototype>
   * |  |  |   constructor: function MyClass()
   * |  |  |   getPrivateProperty: function getPrivateProperty()
   * |  ▶︎ <prototype>
   */
  checkOiNodeText(
    privatePropertyPrototypeChildren[0],
    `constructor: function MyClass()`,
    "constructor is displayed as expected"
  );

  checkOiNodeText(
    privatePropertyPrototypeChildren[1],
    `getPrivateProperty: function getPrivateProperty()`,
    "private method is displayed as expected"
  );

  // TODO: #privateMethod should be displayed (See Bug 1759826)
  // checkOiNodeText(
  //   privatePropertyPrototypeChildren[2],
  //   `#privateMethod: function #privateMethod()`,
  //   "private method is displayed as expected"
  // );

  // TODO: #privateGetter should be displayed here
  // checkOiNodeText(
  //   privatePropertyPrototypeChildren[3],
  //   `#privateGetter: ">>"`,
  //   "private getter value is displayed as expected"
  // );
  // checkOiNodeText(
  //   privatePropertyPrototypeChildren[4],
  //   `<get #privateGetter>: "function"`,
  //   "private getter function is displayed as expected"
  // );
});

// Check viewing private properties on Custom Elements.
add_task(async function () {
  const ELEMENT_TEST_URI =
    "https://example.com/browser/devtools/client/webconsole/test/browser/page_webconsole_object_inspector_private_properties.html";

  const hud = await openNewTabAndConsole(ELEMENT_TEST_URI);

  const node = await waitFor(() =>
    findConsoleAPIMessage(hud, "custom-element-private-properties-test")
  );
  const objectInspectors = [...node.querySelectorAll(".tree")];
  is(
    objectInspectors.length,
    1,
    "There is the expected number of object inspectors"
  );

  const [oi] = objectInspectors;

  info("Expanding the Element");
  await expandObjectInspectorNode(oi.querySelector(".tree-node"));

  const oiNodes = getObjectInspectorNodes(oi);
  // Asserting the number of oiNodes in this case feels very brittle,
  // since the addition of any new Node/Element/etc. properties to the
  // standard will change that number. Let's not.

  const publicDisguisedAsPrivateNodeEl = oiNodes[1];
  // As long as element properties are always sorted alphabetically,
  // same as any other object, this will be "#privateProperty" and
  // will sort above everything else. Note that oiNodes[0] is the
  // custom element itself.

  checkOiNodeText(
    publicDisguisedAsPrivateNodeEl,
    `"#privateProperty": \"Actually a public property\"`,
    `"fake" private property on a custom element has expected text`
  );

  // Since the number of oiNodes is unpredictable, the indices of the
  // properties we want is also unpredictable. Just search for them
  // instead.
  let foundPublicProperty = false;
  let foundPrivateProperty = false;
  for (const oiNode of oiNodes) {
    const textContent = oiNode.querySelector(".node").textContent.trim();
    switch (textContent) {
      case "publicProperty: 1":
        foundPublicProperty = true;
        break;
      case "#privateProperty: 2":
        foundPrivateProperty = true;
        break;
    }
  }
  is(
    foundPublicProperty,
    true,
    "public property of a custom element is displayed as expected"
  );
  is(
    foundPrivateProperty,
    true,
    "private property of a custom element is displayed as expected"
  );
});

function checkOiNodeText(oiNode, expectedText, assertionName) {
  // strip out unwanted character before the label
  is(
    oiNode.querySelector(".node").textContent.trim(),
    expectedText,
    assertionName
  );
}
