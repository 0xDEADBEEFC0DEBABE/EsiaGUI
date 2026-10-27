# EsiaGUI and Esia

EsiaGUI is [Esia](https://github.com/0xDEADBEEFC0DEBABE/Esia) with a widget layer added: the liquid-glass widgets
(`esia::ui`), their examples and their documents. Esia is the engine underneath - the UI core, the renderer, the text
system, the RHI with its backends, the platform layers, the example `glass_window`, the tests and CI - and the
repository where that engine is developed. EsiaGUI shares Esia's history and merges Esia's `main` as its upstream: it
is a fork in everything but GitHub's label (GitHub does not fork a repository into the account that owns it).

## What is only in EsiaGUI

* `src/esia/ui`, `include/esia/ui`, `tests/ui`: the widget layer and its tests. The build adds them when their
  directories exist (`src/esia/CMakeLists.txt`, `tests/CMakeLists.txt`); without them the tree is Esia's.
* `examples/showcase`, `examples/workbench`: the examples on the widgets (the top-level `CMakeLists.txt` adds them
  when they and the widget layer are there).
* `docs/UI_WIDGETS.md`, `docs/images` (the widgets' pictures), `README.md`, this document.
* `tools/branches`, `.github/workflows/branches.yml`: EsiaGUI's branches per graphics API (`directx`, `opengl`,
  `vulkan`, `metal`).

Everything else is Esia's.

## Where a change goes

* The core, the renderer, the text system, the RHI and the backends, the platform layers, `glass_window`, the tests
  outside `tests/ui`, the build, CI: in Esia, then merged here.
* The widgets, the showcase, the workbench, their documents and pictures: here.
* A change that needs both (a core function the widgets will use): the core part in Esia first, merged here, then the
  widget part here.

## Merging Esia

```
git remote add upstream https://github.com/0xDEADBEEFC0DEBABE/Esia.git   # once per clone
git fetch upstream
git merge upstream/main
```

`README.md` and the documents in `docs/` that both repositories have may conflict: keep EsiaGUI's text and take what
Esia changed in substance. The split itself (Esia's commit that deleted the widgets, examples and documents) was
merged into EsiaGUI with `git merge -s ours`: EsiaGUI recorded it without applying it, so a merge brings only what
changed in Esia after the split.
