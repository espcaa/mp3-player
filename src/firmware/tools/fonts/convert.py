import glob
import os

from bdfparser import Font  # pyright: ignore

ASCII_START = 32
ASCII_END = 126

def get_glyph(font, code):
    char = chr(code)
    try:
        glyph = font.glyph(char)
        if glyph is None:
            glyph = font.glyphbycode(32)
    except Exception:
        glyph = font.glyphbycode(32)
    return glyph

def convert_font(bdf_path):
    font = Font(bdf_path)
    font_name = os.path.splitext(os.path.basename(bdf_path))[0].replace("-", "_")
    output_path = os.path.join(os.path.dirname(bdf_path), f"font_{os.path.splitext(os.path.basename(bdf_path))[0]}.c")

    with open(output_path, "w") as f:
        f.write("#include <stdint.h>\n")
        f.write('#include "../app/gfx.h"\n\n')
        f.write(f"// Font generated from {os.path.basename(bdf_path)}\n\n")
        f.write(f"// --- {font_name} Character Data ---\n")

        for code in range(ASCII_START, ASCII_END + 1):
            glyph = get_glyph(font, code)
            bitmap = glyph.draw(mode=1)
            bitmap_list = bitmap.bytepad().todata(1)

            hex_rows = []
            for row in bitmap_list:
                for i in range(0, len(row), 8):
                    hex_rows.append(f"0x{int(row[i : i + 8], 2):02X}")
            if not hex_rows:
                hex_rows = ["0x00"]

            f.write(f"const uint8_t {font_name}_char_{code}[] = {{\n    ")
            f.write(", ".join(hex_rows))
            f.write("\n};\n\n")

        f.write(f"// --- {font_name} Lookup Table ---\n")
        f.write(f"const Glyph {font_name}_Table[] = {{\n")

        for code in range(ASCII_START, ASCII_END + 1):
            char = chr(code)
            glyph = get_glyph(font, code)
            bitmap = glyph.draw(mode=1)
            width = bitmap.width()
            height = bitmap.height()
            advance = int(glyph.meta.get("dwx0", width))
            yoff = int(glyph.meta.get("bbyoff", 0))

            if code == 92:
                char_label = "'\\\\'"
            elif code == 39:
                char_label = "'\\''"
            else:
                char_label = f"'{char}'"

            f.write(
                f"    {{ {width}, {height}, {advance}, {yoff}, {font_name}_char_{code} }}, // ASCII {code}: {char_label}\n"
            )

        f.write("};\n")

        f.write(f"\n// --- {font_name} Font Metadata ---\n")
        f.write(f"const gfx_font_t {font_name}_font = {{\n")
        f.write(f"    .glyphs = {font_name}_Table,\n")
        f.write(f"    .start_char = {ASCII_START},\n")
        f.write(f"    .end_char = {ASCII_END},\n")
        f.write(f"}};\n")

    print(f"Success! Font data written to '{output_path}'.")

bdf_files = glob.glob("*.bdf")
if not bdf_files:
    print("No .bdf files found in the current directory.")
else:
    for bdf_file in bdf_files:
        convert_font(bdf_file)
