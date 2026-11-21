import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAP = os.path.join(ROOT, "serial_capture.bin")
W, H = 160, 120
LINE = W * 2


def rgb(p):
    return (((p >> 11) & 0x1F) * 255 // 31,
            ((p >> 5) & 0x3F) * 255 // 63,
            (p & 0x1F) * 255 // 31)


def cdiff(a, b):
    return (abs(((a >> 11) & 0x1F) - ((b >> 11) & 0x1F)) +
            abs(((a >> 5) & 0x3F) - ((b >> 5) & 0x3F)) +
            abs((a & 0x1F) - (b & 0x1F)))


class Frame:
    def __init__(self, combo, s, line_bytes, line_count, cap, payload):
        self.combo = combo
        self.s = s
        self.line_bytes = line_bytes
        self.line_count = line_count
        self.cap = cap
        self.payload = payload

    def px(self, x, y):
        o = y * LINE + x * 2
        return self.payload[o] | (self.payload[o + 1] << 8)

    def row(self, y):
        return [self.px(x, y) for x in range(W)]

    def nonzero(self):
        return sum(1 for b in self.payload if b)

    def edges(self, y, limit=12, thr=60):
        r = self.row(y)
        e = []
        for x in range(1, W):
            if cdiff(r[x - 1], r[x]) > thr:
                e.append(x)
                if len(e) >= limit:
                    break
        return e

    def runs(self, y, min_len=2):
        r = self.row(y)
        out = []
        start = 0
        for x in range(1, W + 1):
            if x == W or r[x] != r[start]:
                if x - start >= min_len:
                    out.append((start, x - 1, r[start]))
                start = x
        return out

    def best_shift(self, y, ref=0, max_shift=80):
        a = self.row(ref)
        b = self.row(y)
        best = None
        for shift in range(-max_shift, max_shift + 1):
            total = 0
            count = 0
            for x in range(max_shift, W - max_shift):
                xx = x + shift
                if 0 <= xx < W:
                    total += cdiff(a[x], b[xx])
                    count += 1
            score = total / max(1, count)
            if best is None or score < best[0]:
                best = (score, shift)
        return best

    def energy(self):
        h = 0
        v = 0
        prev = self.row(0)
        for y in range(H):
            cur = self.row(y)
            for x in range(1, W):
                h += cdiff(cur[x - 1], cur[x])
            if y > 0:
                for x in range(W):
                    v += cdiff(prev[x], cur[x])
            prev = cur
        return h, v


def parse_packets(data):
    packets = []
    pos = 0
    while True:
        i = data.find(b"\xA5\x5A\x02", pos)
        if i < 0 or i + 14 > len(data):
            break
        combo = data[i + 3]
        s = data[i + 4]
        lb = data[i + 5] | (data[i + 6] << 8)
        lc = data[i + 7] | (data[i + 8] << 8)
        cap = data[i + 9]
        length = struct.unpack_from("<I", data, i + 10)[0]
        payload = data[i + 14:i + 14 + length]
        if len(payload) != length:
            break
        packets.append(Frame(combo, s, lb, lc, cap, payload))
        pos = i + 14 + length
    return packets


def write_bmp(path, width, height, get_pixel, scale=1):
    out_w = width * scale
    out_h = height * scale
    row_bytes = (out_w * 3 + 3) & ~3
    image_size = row_bytes * out_h
    header = struct.pack("<2sIHHI", b"BM", 54 + image_size, 0, 0, 54)
    dib = struct.pack("<IiiHHIIiiII", 40, out_w, out_h, 1, 24, 0,
                      image_size, 2835, 2835, 0, 0)
    with open(path, "wb") as f:
        f.write(header)
        f.write(dib)
        for y in range(height - 1, -1, -1):
            row = bytearray()
            for x in range(width):
                r, g, b = get_pixel(x, y)
                row += bytes((b, g, r)) * scale
            row += b"\x00" * (row_bytes - len(row))
            for _ in range(scale):
                f.write(row)


def frame_pixel(frame, x, y):
    return rgb(frame.px(x, y))


def main():
    if not os.path.exists(CAP):
        sys.exit("missing " + CAP)
    data = open(CAP, "rb").read()
    print("capture bytes:", len(data))

    packets = parse_packets(data)
    if not packets:
        sys.exit("no version 2 packets found")

    print("packets:", len(packets))
    print()
    print("combo  S  L      R      cap  nonzero  hEnergy  vEnergy  edges@y0 / y60 / y119           shift@1  shift@60")
    print("-" * 125)

    for f in packets:
        h, v = f.energy()
        e0 = f.edges(0)
        e60 = f.edges(60)
        e119 = f.edges(119)
        s1 = f.best_shift(1)
        s60 = f.best_shift(60)
        print("  %02X  %02X  %04X  %04X   %02X  %6d  %7d  %7d  %s / %s / %s  %+4d  %+4d"
              % (f.combo, f.s, f.line_bytes, f.line_count, f.cap,
                 f.nonzero(), h, v, e0[:6], e60[:6], e119[:6], s1[1], s60[1]))

        raw = os.path.join(ROOT, "frame_c%d.raw" % f.combo)
        open(raw, "wb").write(f.payload)
        bmp = os.path.join(ROOT, "frame_c%d.bmp" % f.combo)
        write_bmp(bmp, W, H, lambda x, y, ff=f: frame_pixel(ff, x, y), scale=3)

    print()
    for f in packets:
        print("combo %d row runs:" % f.combo)
        for y in (0, 1, 60, 119):
            runs = f.runs(y)
            print("  y=%3d: %s" % (y, " ".join("x%d-%d:%04X" % r for r in runs[:10])))
        print()

    valid = [f for f in packets if f.nonzero() > 0]
    if valid:
        best = min(valid, key=lambda f: (abs(f.s - 8), abs(f.best_shift(60)[1])))
        print("recommended combo:", best.combo,
              "S=%02X shift@60=%+d" % (best.s, best.best_shift(60)[1]))

        contact = os.path.join(ROOT, "sweep_contact.bmp")
        by_combo = {f.combo: f for f in packets}

        def contact_pixel(x, y):
            cell_w = W * 2
            combo = x // cell_w
            sx = (x % cell_w) // 2
            sy = y // 2
            f = by_combo.get(combo)
            return rgb(f.px(sx, sy)) if f else (0, 0, 0)

        write_bmp(contact, 8 * W, H, contact_pixel, scale=2)
        print("saved", contact)

        s60 = best.best_shift(60)
        shift = s60[1]
        if abs(shift) >= 2:
            shear = os.path.join(ROOT, "frame_best_shear.bmp")

            def shear_pixel(x, y):
                sx = x - shift * y // 60
                if 0 <= sx < W:
                    return rgb(best.px(sx, y))
                return (0, 0, 0)

            write_bmp(shear, W, H, shear_pixel, scale=3)
            print("saved", shear)

        for name, fn in (
            ("raw", lambda x, y: rgb(best.px(x, y))),
            ("rot90", lambda x, y: rgb(best.px(y, H - 1 - x))),
            ("rot180", lambda x, y: rgb(best.px(W - 1 - x, H - 1 - y))),
            ("rot270", lambda x, y: rgb(best.px(W - 1 - y, x))),
        ):
            path = os.path.join(ROOT, "frame_best_%s.bmp" % name)
            if name in ("rot90", "rot270"):
                write_bmp(path, H, W, fn, scale=3)
            else:
                write_bmp(path, W, H, fn, scale=3)
            print("saved", path)


if __name__ == "__main__":
    main()
