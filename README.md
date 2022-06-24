# uboot-xburst

U-Boot 2013.07 for Ingenic XBurst SoCs. It's old but usable.

## Compiling

`make ARCH=mips CROSS_COMPILE=mipsel-linux-gnu- <target>`

### Targets
| Name                       | Manufacturer | Product        | SoC    | Boot Media | Kernel Image Type | Remarks |
|----------------------------|--------------|----------------|--------|------------|-------------------|---------|
| x1501-pico_uImage_sfc_nor  | SudoMaker    | X1501 Pico SoM | X1501  | SFC NOR (XT25F16B KGD) | uImage (LZMA only)|         |
| notkia-168x_uImage_sfc_nor  | SudoMaker   | Notkia 168x    | X1000E | SFC NOR (W25Q512JVFQ) | uImage (gzip only)|         |
| cu1000-neo_uImage_sfc_nor  | YSH & ATIL   | CU1000 Neo     | X1000E | SFC NOR    | uImage (gzip only)|         |
| cu1000-neo_uImage_sfc_nand | YSH & ATIL   | CU1000 Neo     | X1000E | SFC NAND   | uImage (gzip only)|         |
| cu1000-neo_uImage_msc0     | YSH & ATIL   | CU1000 Neo     | X1000E | MSC0 | uImage (gzip only)|         |

TODO
