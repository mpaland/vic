# Changelog

## [2.1.0](https://github.com/mpaland/vic/compare/v2.0.0...v2.1.0) (2026-11-10)


### Features

* **head:** Added the X11 head for Linux ([2119fd9](https://github.com/mpaland/vic/commit/2119fd9e8af79e716d7e55700b793619f12e6650))


### Bug Fixes

* **gpr:** Fixed fill() on heads with reduced color depth, spans via line_horz() ([4d3b9c3](https://github.com/mpaland/vic/commit/4d3b9c34e713343149fa269542c9e978156036f9))
* **gpr:** Fixed fill() with color changing shaders like gradient ([2fd0b4b](https://github.com/mpaland/vic/commit/2fd0b4b1ca34f79ac55310bdcb9e08ed2ade58d7))
* **head:** Fixed the head discrepancies found in the datasheet review ([8acf585](https://github.com/mpaland/vic/commit/8acf585b82135b5d66813211c6619f76cf2f01d6))
* **txr:** Added missing cast ([921539c](https://github.com/mpaland/vic/commit/921539cdd7ca3fddc79532e3fe4d66210d645500))


### Performance

* **gpr:** Anti aliased circles and sectors only test the pixels of the ring ([f67a4ee](https://github.com/mpaland/vic/commit/f67a4ee9798c44eca8422fe2b8251445fe2a9823))
* **gpr:** Anti aliased convex shapes only test the ends of a row ([3dfeecd](https://github.com/mpaland/vic/commit/3dfeecd52193ac7c947fc51d51a02bfd86b9ee93))
* **gpr:** Solid triangle computes the span of a row instead of searching it ([4b54086](https://github.com/mpaland/vic/commit/4b54086f2e5a6713d22a456af8aab2cf928a767a))


### Refactoring

* **gpr:** Removed the ring predicate, it's a sector with equal radii ([6ff1856](https://github.com/mpaland/vic/commit/6ff18561b4f3ede9512881773e76905ad72278ca))


### Documentation

* **head:** Added datasheet summaries, removed the PDFs from git ([5360813](https://github.com/mpaland/vic/commit/53608132b426bf09ad0e6c09fdc9093233173494))
* **sprite:** Update sprite header documentation ([a0ff6d7](https://github.com/mpaland/vic/commit/a0ff6d7a5a4a94bc7c86e20c6143489e2d6481f0))

## [2.0.0](https://github.com/mpaland/vic/compare/v1.2.0...v2.0.0) (2026-11-06)


### ⚠ BREAKING CHANGES

* **font:** Added support for scaling vector fonts

### Features

* **font:** Added support for scaling vector fonts ([b9e9ca7](https://github.com/mpaland/vic/commit/b9e9ca7af0960c669bf162e2d1c59ab2ea766489))
* **font:** Added the Google Doto (LED) vector font ([f7681d3](https://github.com/mpaland/vic/commit/f7681d34aa89c927302dde7c40260636180bfe4f))

## [1.2.0](https://github.com/mpaland/vic/compare/v1.1.0...v1.2.0) (2026-10-29)


### Features

* **font:** Added Michroma font ([a23ee32](https://github.com/mpaland/vic/commit/a23ee32ac1e52c0cde07a1fd878491a3cb1ffa9d))


### Bug Fixes

* **framebuffer:** Added missing display_xxx() functions ([c4997bf](https://github.com/mpaland/vic/commit/c4997bfd6ea9e916a398d795ca16d54f41095e67))
* **test,demo:** Use the Michroma font ([8197b38](https://github.com/mpaland/vic/commit/8197b38b6e6766083d2c49e3eca935babbafccef))
