#!/usr/bin/env python3
"""
generate_font.py
-----------------
Offline helper that regenerates the glyph-data body of src/ArabicFont.h.

You do NOT need this script to use the ArabicOLED library - the glyph
data is already committed to src/ArabicFont.h. Run this only if you want
to change the font source, target glyph height, or the set of covered
codepoints.

It rasterises every Arabic "Presentation Forms-B" codepoint (U+FE70-
U+FEFC) - i.e. every individual isolated/initial/medial/final letter
shape, already resolved to its correct joined form - directly from a
TrueType font that has native glyphs for that block (e.g. FreeSerif,
part of the GNU FreeFont project, commonly available on Linux at
/usr/share/fonts/truetype/freefont/FreeSerif.ttf). Because each shape
already has its own dedicated Unicode codepoint, no font-shaping engine
(HarfBuzz/Raqm) is required at generation time - we just render each
codepoint like any other character. The runtime shaping logic (deciding
*which* codepoint a given letter needs) lives in ArabicOLED.cpp, not
here.

Requirements: Python 3 + Pillow (`pip install pillow`), and a TTF font
with Arabic Presentation Forms-B coverage.

Usage:
    python3 tools/generate_font.py --font /path/to/FreeSerif.ttf \
        --height 24 --out /tmp/font_body.h

Then splice the output between the struct definition and the lookup
function in src/ArabicFont.h.
"""
import argparse
import sys

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("This script requires Pillow: pip install pillow")

# base_codepoint: (isolated, initial, medial, final)  -- 0 = not applicable
# Derived from the Unicode Arabic Presentation Forms-B block's own glyph
# names (e.g. U+FEE3 == "ARABIC LETTER MEEM INITIAL FORM"), not hand-typed.
ARABIC_LETTERS = {
    0x0621: (0xFE80, 0, 0, 0),                # HAMZA
    0x0622: (0xFE81, 0, 0, 0xFE82),           # ALEF MADDA
    0x0623: (0xFE83, 0, 0, 0xFE84),           # ALEF HAMZA ABOVE
    0x0624: (0xFE85, 0, 0, 0xFE86),           # WAW HAMZA
    0x0625: (0xFE87, 0, 0, 0xFE88),           # ALEF HAMZA BELOW
    0x0626: (0xFE89, 0xFE8B, 0xFE8C, 0xFE8A), # YEH HAMZA
    0x0627: (0xFE8D, 0, 0, 0xFE8E),           # ALEF
    0x0628: (0xFE8F, 0xFE91, 0xFE92, 0xFE90), # BEH
    0x0629: (0xFE93, 0, 0, 0xFE94),           # TEH MARBUTA
    0x062A: (0xFE95, 0xFE97, 0xFE98, 0xFE96), # TEH
    0x062B: (0xFE99, 0xFE9B, 0xFE9C, 0xFE9A), # THEH
    0x062C: (0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E), # JEEM
    0x062D: (0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2), # HAH
    0x062E: (0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6), # KHAH
    0x062F: (0xFEA9, 0, 0, 0xFEAA),           # DAL
    0x0630: (0xFEAB, 0, 0, 0xFEAC),           # THAL
    0x0631: (0xFEAD, 0, 0, 0xFEAE),           # REH
    0x0632: (0xFEAF, 0, 0, 0xFEB0),           # ZAIN
    0x0633: (0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2), # SEEN
    0x0634: (0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6), # SHEEN
    0x0635: (0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA), # SAD
    0x0636: (0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE), # DAD
    0x0637: (0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2), # TAH
    0x0638: (0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6), # ZAH
    0x0639: (0xFEC9, 0xFECB, 0xFECC, 0xFECA), # AIN
    0x063A: (0xFECD, 0xFECF, 0xFED0, 0xFECE), # GHAIN
    0x0641: (0xFED1, 0xFED3, 0xFED4, 0xFED2), # FEH
    0x0642: (0xFED5, 0xFED7, 0xFED8, 0xFED6), # QAF
    0x0643: (0xFED9, 0xFEDB, 0xFEDC, 0xFEDA), # KAF
    0x0644: (0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE), # LAM
    0x0645: (0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2), # MEEM
    0x0646: (0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6), # NOON
    0x0647: (0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA), # HEH
    0x0648: (0xFEED, 0, 0, 0xFEEE),           # WAW
    0x0649: (0xFEEF, 0, 0, 0xFEF0),           # ALEF MAKSURA
    0x064A: (0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2), # YEH
}
LAM_ALEF = {
    0x0622: (0xFEF5, 0xFEF6),
    0x0623: (0xFEF7, 0xFEF8),
    0x0625: (0xFEF9, 0xFEFA),
    0x0627: (0xFEFB, 0xFEFC),
}
EXTRA_CODEPOINTS = [0x061F, 0x060C]  # Arabic question mark, Arabic comma


def collect_codepoints():
    cps = set()
    for forms in ARABIC_LETTERS.values():
        cps.update(cp for cp in forms if cp)
    for iso, fin in LAM_ALEF.values():
        cps.add(iso)
        cps.add(fin)
    cps.update(EXTRA_CODEPOINTS)
    return sorted(cps)


def rasterize(font_path, target_h, render_size=200, max_w=32):
    font = ImageFont.truetype(font_path, render_size)
    ascent, descent = font.getmetrics()
    full_h = ascent + descent
    scale = target_h / full_h

    def glyph(cp):
        ch = chr(cp)
        adv_px = max(1, round(font.getlength(ch)))
        img = Image.new("L", (adv_px, full_h), 0)
        d = ImageDraw.Draw(img)
        # Anchor every glyph on the SAME baseline (font ascent) so glyphs
        # line up correctly when placed side by side, regardless of each
        # letter's individual height.
        d.text((0, ascent), ch, font=font, fill=255, anchor="ls")
        w = max(1, min(max_w, round(adv_px * scale)))
        small = img.resize((w, target_h), Image.LANCZOS)
        px = small.load()
        rows = []
        for y in range(target_h):
            rows.append([1 if px[x, y] >= 128 else 0 for x in range(w)])
        return w, rows

    glyphs = {}
    for cp in collect_codepoints():
        w, rows = glyph(cp)
        bytes_per_row = (w + 7) // 8
        data = bytearray()
        for row in rows:
            for b in range(bytes_per_row):
                byte = 0
                for bit in range(8):
                    xi = b * 8 + bit
                    if xi < w and row[xi]:
                        byte |= 0x80 >> bit
                data.append(byte)
        glyphs[cp] = (w, bytes(data))
    return glyphs


def emit_header(glyphs, target_h, out):
    codepoints = sorted(glyphs)
    out.write(f"// Fixed height (in pixels) of every glyph bitmap below. Glyph width varies\n"
              f"// per-letter and is stored in each ArabicGlyph entry's `width` field.\n"
              f"#define ARABIC_GLYPH_HEIGHT {target_h}\n\n")
    for cp in codepoints:
        w, data = glyphs[cp]
        hexbytes = ", ".join(f"0x{b:02X}" for b in data)
        out.write(f"static const uint8_t ARFONT_{cp:04X}[] PROGMEM = {{ {hexbytes} }};\n")
    out.write("\nstatic const ArabicGlyph ARABIC_GLYPH_TABLE[] PROGMEM = {\n")
    for cp in codepoints:
        w, _ = glyphs[cp]
        out.write(f"  {{ 0x{cp:04X}, {w}, ARFONT_{cp:04X} }},\n")
    out.write("};\n")
    out.write(f"static const uint16_t ARABIC_GLYPH_COUNT = {len(codepoints)};\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--font", required=True, help="Path to a .ttf with Arabic Presentation Forms-B coverage")
    ap.add_argument("--height", type=int, default=24, help="Target glyph height in pixels")
    ap.add_argument("--out", default="-", help="Output file (default: stdout)")
    args = ap.parse_args()

    glyphs = rasterize(args.font, args.height)
    out = sys.stdout if args.out == "-" else open(args.out, "w")
    emit_header(glyphs, args.height, out)
    if out is not sys.stdout:
        out.close()
    print(f"Generated {len(glyphs)} glyphs.", file=sys.stderr)


if __name__ == "__main__":
    main()
