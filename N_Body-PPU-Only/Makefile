CC = powerpc-linux-gcc

INCL = /opt/cell/toolchain/include


N_Body-PPU-Only: NBodyPPUOnly.c
			$(CC) NBodyPPUOnly.c -o N_Body-PPU-Only_Bin -I$(INCL) -maltivec -mabi=altivec
