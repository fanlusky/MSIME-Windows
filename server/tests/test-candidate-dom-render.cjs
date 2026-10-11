const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');
const vm = require('node:vm');

const source = fs.readFileSync(
  path.join(__dirname, '../src/webview2/windows_webview2_candidate.cpp'),
  'utf8',
);
const match = source.match(/constexpr wchar_t kRenderCandidateItemsScript\[\] = LR"MSIME_JS\(([\s\S]*?)\)MSIME_JS";/);
assert.ok(match, 'candidate DOM renderer must be present');

function element(tagName) {
  return {
    tagName,
    textContent: '',
    className: '',
    style: {},
    children: [],
    set innerHTML(_) {
      throw new Error('candidate text must not use innerHTML');
    },
    replaceChildren(...children) {
      this.children = children;
    },
    append(child) {
      this.children.push(child);
    },
  };
}

// Like Node.textContent: string children are text nodes.
function textOf(node) {
  if (typeof node === 'string') return node;
  return node.children.length ? node.children.map(textOf).join('') : node.textContent;
}

test('candidate DOM renderer keeps text, order, style, badge, and translation', () => {
  const slots = Array.from({ length: 3 }, () => element('div'));
  const wrappers = slots.map((slot) => ({
    querySelector(selector) {
      assert.equal(selector, '.cand-content');
      return slot;
    },
  }));
  const root = {
    querySelectorAll(selector) {
      assert.equal(selector, '.row-wrapper');
      return wrappers;
    },
  };
  const items = [
    { text: 'a,b\uF000', annotation: '<&', badge: '[固定]', translation: '译,\uF000<&', fixedPosition: true },
    { text: '${value}`', annotation: '&<', badge: '②', translation: '译${value}`', fixedPosition: false },
    { text: '末尾', annotation: '', badge: '', translation: '', fixedPosition: false },
  ];
  const context = { document: { createElement: element } };
  vm.runInNewContext(match[1], context);
  context.RenderCandidateItems(root, items);

  assert.equal(slots.length, items.length);
  assert.deepEqual(slots.map((slot) => slot.children.length), [2, 2, 1]);
  assert.deepEqual(
    slots.map((slot) => slot.children[0].textContent),
    items.map((item) => item.text + item.annotation + item.badge),
  );
  assert.deepEqual(
    slots.slice(0, 2).map((slot) => textOf(slot.children[1])),
    items.slice(0, 2).map((item) => item.translation),
  );
  assert.deepEqual(slots.slice(0, 2).map((slot) => slot.children[1].className), [
    'cand-translation',
    'cand-translation',
  ]);
  assert.equal(slots[0].children[0].style.color, '#379AD3');
  assert.equal(slots[1].children[0].style.color, undefined);
  assert.equal(slots[2].children[0].style.color, undefined);
});

test('candidate DOM renderer wraps part-of-speech labels and separators', () => {
  const slot = element('div');
  const root = {
    querySelectorAll: () => [{ querySelector: () => slot }],
  };
  const items = [{
    text: '笑',
    annotation: '',
    badge: '',
    translation: 'adj. <a>; n. laugh',
    translationParts: [['cand-pos', 'adj.'], ['', ' <a>'], ['cand-sep', ';'], ['', ' '], ['cand-pos', 'n.'], ['', ' laugh']],
    fixedPosition: false,
  }];
  const context = { document: { createElement: element } };
  vm.runInNewContext(match[1], context);
  context.RenderCandidateItems(root, items);

  const translation = slot.children[1];
  assert.equal(translation.className, 'cand-translation');
  const parts = translation.children.map((child) =>
    typeof child === 'string' ? ['', child] : [child.className, child.textContent]);
  assert.deepEqual(parts, items[0].translationParts);
});
