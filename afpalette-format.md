# Affinity `.afpalette` format

Affinity Designer / Photo palette files are a little-endian binary format made of tagged chunks. Ocular reads versions **8–20** and writes **version 8** swatch palettes compatible with Affinity 2.

## File layout

| Region | Size | Description |
|--------|------|-------------|
| Header | 80 bytes | Magic, version, size fields, metadata chunks |
| Body | variable | Palette name, swatch count, color data, swatch names |
| End marker | 5 bytes | `00 FF FF FF FF` |
| `#FAT` footer | 107 bytes | Metadata, size fields, CRC32, `Swatches.dat` path |

The last **115 bytes** on disk are often described as the “footer” in other tools: that span includes any trailing name bytes, the end marker, and the `#FAT` chunk.

## Header (80 bytes)

| Offset | Type | Field |
|--------|------|-------|
| 0 | `uint32` | Magic `0x414BFF00` (`00 FF 4B 41`) |
| 4 | `uint32` | Format version (8 in files written by Ocular) |
| 8 | char[4] | `htwS` |
| 12 | char[4] | `#Inf` |
| 16 | `uint64` | Offset of `#FAT` chunk (patched on save) |
| 24 | `uint64` | Total file size (patched on save) |
| 32 | `uint64` | CRC length: `file_size − 80 − 107` (patched on save) |
| 48 | `uint32` | Unix datestamp placeholder (`0x544E4DEB` in templates) |
| … | … | `Prot`, `#Fil`, `KS` tags (fixed in v8 template) |

## Body

After the header:

1. **`VlaP` wrapper** — `01 00` + `VlaP` + `01 00`
2. **`+NClP`** — tag `2B 4E 43 6C 50`, then `uint32` UTF-8 length and palette name
3. **`B1VlaP`** — byte `0xB1` + `VlaP`, then `uint32` swatch count
4. **Swatch prefix** — `01 00 00 00 00 00` before the first color
5. **Color swatches** — linked list of `SliF` / `1rloC` blocks (see below)
6. **`VNaP` names** — byte `0xAB`, tag `VNaP`, block size, count, then length-prefixed UTF-8 names

### Swatch chaining

The first swatch uses a longer `SliF` / `lliF` / `1rloC` / `ABAL` header. Each following swatch is a shorter `SliF` + `1rloC` block. After every swatch except the last, six bytes link to the next index: `01 NN 00 00 00 01` where `NN = swatch_index × 2`.

### Color encodings (read)

Ocular scans for type tags after `rloC` markers:

| Tag | Location tag | Data |
|-----|----------------|------|
| `ABGR` | `Dloc_` | Three `float32` RGB (0–1) |
| `ABAL` | `<loc_` | Three `uint16` Lab (scaled 0–65535) |
| `KYMC` | `Hloc_` | Four `float32` CMYK |
| `ALSH` | `Dloc_` | Three `float32` HSL |
| `YARG` | `<loc_` | One `float32` gray |

### Color encoding (write)

Swatches are written as **`ABAL`** with Lab values derived from RGB via `rgb2lab`, then a `<loc_` block and suffix `FF FF 00 00`.

## `#FAT` footer (107 bytes)

Starts with tag `#FAT` (`23 46 41 54`). Important patched fields (relative to `#FAT`):

| Offset | Type | Value |
|--------|------|-------|
| 20 | `uint64` | File size |
| 28 | `uint64` | CRC length |
| 72 | `uint64` | CRC length (duplicate) |
| 80 | `uint64` | CRC length (duplicate) |
| 88 | `uint32` | CRC32 checksum |

Ends with a length-prefixed `Swatches.dat` path (`0x0C 00` + `"Swatches.dat"`).

## CRC32

Affinity 2 validates the file checksum.

- **Polynomial:** `0xEDB88320` (standard CRC-32)
- **Range:** byte offset **76** for **`file_size − 80 − 107`** bytes (includes the last four header bytes and the body through the end marker)
- **Stored at:** `#FAT` + 88

```c
crc = CRC32(data[76 .. 76 + crc_length])
```

## Related formats

Gradient-heavy palettes may use **version 11**, a **`#FT4`** footer (115 bytes), and **`ABGR`** floats instead of `ABAL` Lab swatches.

Ocular’s writer targets simple v8 swatch palettes (`#FAT`).

## Implementation

`read_afpalette_palette` and `save_afpalette_palette` in [`lib/palette.c`](../lib/palette.c).
