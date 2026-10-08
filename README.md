# MP3 Tag Reader and Editor

A lightweight command-line tool written in C that reads and modifies ID3v2 metadata in standard MP3 audio files. It parses raw binary headers, converts big-endian frame sizes, handles syncsafe integers, and updates tags without damaging the underlying audio stream.

---

## Features

- **Read Metadata:** Parses the 10-byte ID3v2 header and displays embedded frames (Title, Artist, Album, Year, Track, Genre, and Comments).
- **Edit Metadata:** Modifies specific tags directly from the terminal and recalculates frame sizes.
- **Syncsafe Integer Decoding:** Correctly calculates the 28-bit tag size from the ID3 header to locate where the MPEG audio stream begins.
- **Safe File Writing:** Uses a temporary file workflow so the original audio stream is not corrupted if an error occurs mid-write.

---

## Project Structure

```text
├── main.c        # Command-line interface, argument parsing, and menu handling
├── tag.c         # Binary parsing, byte conversion, viewing, and editing logic
├── tag.h         # Function declarations and standard library headers
├── sample.mp3    # Sample audio file for testing
└── README.md     # Project documentation
```

---

## How to Build

Compile the source files using `gcc`:

```bash
gcc main.c tag.c -o mp3_tag_reader
```

---

## Usage

### 1. View Help Menu
```bash
./mp3_tag_reader -h
```

### 2. View Tags
```bash
./mp3_tag_reader -v sample.mp3
```

### 3. Edit Tags
```bash
./mp3_tag_reader -e <flag> "<new_value>" <filename.mp3>
```

#### Supported Edit Flags:
| Flag | Metadata Field | ID3 Frame ID |
|---|---|---|
| `-t` | Song Title | `TIT2` |
| `-T` | Track Number | `TRCK` |
| `-a` | Artist Name | `TPE1` |
| `-A` | Album Name | `TALB` |
| `-y` | Year | `TYER` |
| `-g` | Genre | `TCON` |
| `-c` | Comments | `COMM` |

---

## Examples

### Viewing Metadata
```text
$ ./mp3_tag_reader -v sample.mp3
MP3 Tag Reader & Editor:
------------------------------------------------------------
Version ID: 3.0
------------------------------------------------------------
+------------------------------------+
| TITLE :: Midnight Drive            |
+------------------------------------+
+------------------------------------+
| ARTIST :: John Doe                 |
+------------------------------------+
+------------------------------------+
| ALBUM :: Chill Beats               |
+------------------------------------+
+------------------------------------+
| YEAR :: 2024                       |
+------------------------------------+
```

### Updating a Tag
```text
$ ./mp3_tag_reader -e -t "Night Drive (Remix)" sample.mp3
Updating frame: TIT2
Changes saved successfully to sample.mp3
```

---

## Technical Notes

1. **Big-Endian to Little-Endian:** ID3v2 frame sizes are stored in big-endian byte order. The application converts these 4-byte values to little-endian using bitwise shifts so host architectures can compute accurate buffer sizes.
2. **Syncsafe Integers:** ID3v2 headers clear the most significant bit (MSB) of each size byte to avoid false MPEG sync signals (a sequence of eleven consecutive 1 bits). The program reconstructs the 28-bit value before calculating tag offsets.
3. **Data Integrity:** When editing, untouched frames, header padding, and MPEG audio frames are preserved and streamed sequentially into `temp.mp3` before replacing the source file.

---

## Author

- **P. Nithyanand**
- GitHub: [@NithyanandPuralasetti](https://github.com/NithyanandPuralasetti)
