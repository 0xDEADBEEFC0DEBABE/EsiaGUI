# Test fonts

The FreeType + HarfBuzz text tests (`tests/text/test_ft_text.cpp`, `test_ft_cjk.cpp`), the system font lookup's tests
(`test_system_fonts.cpp`) and the golden images in `tests/text/golden` use these fonts.

* `DroidSans.ttf`: Droid Sans by Steve Matteson, Apache License 2.0 (came with Dear ImGui's `misc/fonts` in WGT).
* `Karla-Regular.ttf`: Karla by Jonathan Pinhorn, SIL Open Font License 1.1 (1000 units per em: the tests use it as
  the fallback font; also from Dear ImGui's `misc/fonts`).
* `NotoSansSC-Subset.otf`: 87 characters of Noto Sans SC Regular 2.004 (Adobe / Google, SIL Open Font License 1.1,
  `NotoSansSC-OFL.txt`): CFF outlines, 1000 units per em. A subset of the region-specific
  `Sans/SubsetOTF/SC/NotoSansSC-Regular.otf` of https://github.com/notofonts/noto-cjk (SHA256
  `faa6c9df652116dde789d351359f3d7e5d2285a2b2a1f04a2d7244df706d5ea9`), made with fontTools 4.66.1:

  ```
  pyftsubset NotoSansSC-Regular.otf --text-file=chars.txt --layout-features='*' --no-hinting --desubroutinize \
      --output-file=NotoSansSC-Subset.otf
  ```

  `chars.txt` holds the CJK characters of `test_ft_cjk.cpp` (the Hangul excepted: the SC subset font has none) plus
  U+2026 and U+3000:

  ```
  …　、。《》「」かなのカ一不与中之书介以会体出单可号名在型字宽对尾居年库度引很态括换排文日时是月本标段汉测液混点版现玻璃界略的省看符繁结與英行語词试语超这长间面首體齐（），：
  ```

  A test that needs another character adds it there and makes the subset again. The subset keeps the font's name
  ("Noto Sans SC"), its copyright and license records: the font declares no Reserved Font Name. Noto is a trademark of
  Google Inc.
