# Changelog

## [2.3.0](https://github.com/mpaland/vic/compare/v2.2.0...v2.3.0) (2026-11-19)


### Features

* **head:** Added the JD79667 four color e-paper head ([add6a26](https://github.com/mpaland/vic/commit/add6a26d56a438e6c020ae241c32d636fffbb0f5))
* **sprite:** Added canvas_sheet_xy, array restore buffer for sheet ([517f78a](https://github.com/mpaland/vic/commit/517f78ad8d4fe3ab7332fb8f542a622791a1ac53))


### Bug Fixes

* **demo:** Removed the unused wave function ([1c8d2b3](https://github.com/mpaland/vic/commit/1c8d2b377404aacd79a15dd438853b8c2ac3820f))
* **HD44780:** Relaxed timing paramter ([bf6340e](https://github.com/mpaland/vic/commit/bf6340e117bdc77f7bb39b7c4be00753b775b779))
* **shader:** Fixed the uninitialized warning of the gradient distances ([08dd2b5](https://github.com/mpaland/vic/commit/08dd2b5d223a79e7c4206010494b14eda65072ce))
* **shader:** Limited the gradient weighting to the array size for the GCC uninitialized warning ([4664bb9](https://github.com/mpaland/vic/commit/4664bb9a45e7e29380d70d44c9f800369ad3faf4))


### Performance

* **demo:** Used canvas_sheet_xy in the sprite demo ([03fc7a1](https://github.com/mpaland/vic/commit/03fc7a17cc9ce3a2264b455e05d25cdc8c55484a))
* **head:** Added native horizontal and vertical lines to the JD79667 head ([22897b0](https://github.com/mpaland/vic/commit/22897b0cfb950d68645945e51dad4e5c5c7b8f6d))
* **head:** Added native horizontal and vertical lines to the SSD1306 head ([86d0eb8](https://github.com/mpaland/vic/commit/86d0eb8b043b3bed125fedb2503802f4debb1582))
* **head:** Added native horizontal and vertical lines to the SSD1680 head ([cfbc39c](https://github.com/mpaland/vic/commit/cfbc39c42c04f648102ba845396a2d1532adcc43))


### Documentation

* **head:** Added the JD79667 native line functions ([a5105e6](https://github.com/mpaland/vic/commit/a5105e6e91dc348534d29d5c0b4079b17a3c71d4))
* **head:** Added the JD79667 programming summary ([68a6622](https://github.com/mpaland/vic/commit/68a6622386768b25e72612850707d12a450bc9ce))
* **head:** Added the SSD1306 native line functions ([dbbeda3](https://github.com/mpaland/vic/commit/dbbeda30de6ca8c3b9acba379936f208fca6bf3a))
* **head:** Added the SSD1680 native line functions ([e96bc36](https://github.com/mpaland/vic/commit/e96bc36505a6c87374bd017c2fb8f48bb8e5eabd))
* **readme:** Added canvas_sheet_xy ([b655644](https://github.com/mpaland/vic/commit/b6556441293cd3714cc4af07adf9a0e0c8651e96))
* **readme:** Added the docs badge ([1194264](https://github.com/mpaland/vic/commit/119426464d0e11db1910c0fa45eb085cd1ce068e))
* **readme:** Added the JD79667 head ([0ddbfa6](https://github.com/mpaland/vic/commit/0ddbfa65fc07628fb0f20f53b4b4e221c227ea39))
* **site:** Added canvas_sheet_xy and the sprite type comparison ([758301c](https://github.com/mpaland/vic/commit/758301c1238595419c94cc40f8e069a1d9717c3b))
* **site:** Added the GitHub Pages website ([124eed0](https://github.com/mpaland/vic/commit/124eed0878029c3635e6a34e690e191a7856929c))
* **site:** Added the JD79667 head ([30d5708](https://github.com/mpaland/vic/commit/30d5708264e5f4b811b6465e6144bc3b4ecc9ded))
* **site:** Added the JD79667 native line functions ([b502291](https://github.com/mpaland/vic/commit/b502291c096681b4ed975c8c7ae9efd7ea01e6b3))
* **site:** Added the SSD1306 native line functions ([3c03186](https://github.com/mpaland/vic/commit/3c03186ae4e4a3e7ce6861a58bfbbb9727c2646f))
* **site:** Added the SSD1680 native line functions ([862dbae](https://github.com/mpaland/vic/commit/862dbae95e26d0b44d51816220649124e8d89511))
* **site:** Moved the available heads to the top of the heads page ([dbbef6c](https://github.com/mpaland/vic/commit/dbbef6c5b3c634b4fabf2811002d7fadb2626480))
* **site:** Redesigned the landing page with an animated pipeline hero ([61a0397](https://github.com/mpaland/vic/commit/61a03977f46b0336bff0d3e96d66156232e1657f))

## [2.2.0](https://github.com/mpaland/vic/compare/v2.1.0...v2.2.0) (2026-11-10)


### Features

* **cmake:** Added install rules and the package config ([73b7cb7](https://github.com/mpaland/vic/commit/73b7cb777dec468be3f6a13c1125cd00681d6b42))
* **gpr:** Added the pen width for lines and outlines ([23be7aa](https://github.com/mpaland/vic/commit/23be7aa96d7fb52e62ae7388325c4a2597dcb6a6))
* **pio:** Added a PlatformIO manifest ([14cfbc7](https://github.com/mpaland/vic/commit/14cfbc7210b9fc41ee329a6a6cc28283195be93b))


### Bug Fixes

* **gpr:** Anti aliased rounded rectangle had four half covered pixels between arcs and lines ([7cc136c](https://github.com/mpaland/vic/commit/7cc136c8c845e91f6c3d56e1a701d18efb02573e))


### Documentation

* **readme:** Added the CodeQL badge, removed the issues badge ([331ac82](https://github.com/mpaland/vic/commit/331ac824abdc99fc2e8b729c08d619ca4c3dce24))
* **readme:** Corrected the angle direction of the gpr section ([a13dd94](https://github.com/mpaland/vic/commit/a13dd94bec3fb7fbfc2c14352782e4ef8f9da591))

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
