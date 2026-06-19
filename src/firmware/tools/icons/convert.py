"""Convert PNGs in ./png into a single embedded icon sheet (icons.c / icons.h).

Each pixel becomes one bit (1 = drawn). Color is applied at draw time by
gfx_draw_icon, so the source color does not matter -- only the shape mask:
  - if the PNG has transparency, alpha >= 128 is "on"
  - otherwise, luminance < 128 (dark pixels) is "on"

Run with:  uv run convert.py
"""

from pathlib import Path

from PIL import Image

HERE = Path(__file__).parent
PNG_DIR = HERE / "png"
OUT_DIR = HERE / "../../src/ui/icons"
OUT_C = OUT_DIR / "icons.c"
OUT_H = OUT_DIR / "icons.h"


def icon_name(png_path: Path) -> str:
    return png_path.stem.replace("-", "_").replace(" ", "_")


def pack(img: Image.Image) -> tuple[int, int, list[int]]:
    rgba = img.convert("RGBA")
    w, h = rgba.size
    px = rgba.load()

    has_alpha = any(px[x, y][3] < 255 for y in range(h) for x in range(w))

    def is_on(x: int, y: int) -> bool:
        r, g, b, a = px[x, y]
        if has_alpha:
            return a >= 128
        return (r * 299 + g * 587 + b * 114) // 1000 < 128

    bytes_per_row = (w + 7) // 8
    data: list[int] = []
    for y in range(h):
        for byte in range(bytes_per_row):
            value = 0
            for bit in range(8):
                x = byte * 8 + bit
                if x < w and is_on(x, y):
                    value |= 1 << (7 - bit)
            data.append(value)
    return w, h, data


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pngs = sorted(PNG_DIR.glob("*.png"))

    with OUT_H.open("w") as h:
        h.write('#include "gfx.h"\n\n')
        for png in pngs:
            h.write(f"extern const gfx_icon_t icon_{icon_name(png)};\n")

    with OUT_C.open("w") as c:
        c.write('#include "icons.h"\n')
        c.write("#include <stdint.h>\n\n")
        for png in pngs:
            name = icon_name(png)
            w, ht, data = pack(Image.open(png))
            rows = ", ".join(f"0x{b:02X}" for b in data)
            c.write(f"static const uint8_t {name}_data[] = {{ {rows} }};\n")
            c.write(
                f"const gfx_icon_t icon_{name} = {{ {w}, {ht}, {name}_data }};\n\n"
            )

    print(f"wrote {len(pngs)} icon(s) to {OUT_C} and {OUT_H}")


if __name__ == "__main__":
    main()
