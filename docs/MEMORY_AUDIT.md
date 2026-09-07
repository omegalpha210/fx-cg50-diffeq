> Historical beta.1 memory audit. Current beta.2 section sizes and validation are in [ACCEPTANCE.md](ACCEPTANCE.md).

# Target memory boundary

The validated source baseline has SH data 704 bytes, BSS 62,736 bytes, Document
2,920 bytes and App 13,960 bytes. TRACE reuses its existing 44,756-byte fixed scratch:
20,660 sample cache + 9,504 one-bit raster mask + 14,592 footer. No second full
framebuffer or trajectory clone was introduced for invalid-region support.

The largest individual application frame in the source baseline is 1,932 bytes,
below the 3,072-byte warning threshold. Frames exclude callees. Gint's default
single VRAM is 177,408 bytes and its stack reservation is 16 KiB. ELF BSS/data do
not measure runtime OS/library allocation or combined stack high-water.

Recheck with `sh-elf-size build-cg/diffeq`, `sh-elf-nm -S --size-sort build-cg/diffeq`
and the compiler-generated `.su` files. Release-specific validation is recorded in
[ACCEPTANCE.md](ACCEPTANCE.md). Physical runtime margins remain HARDWARE TEST REQUIRED.
