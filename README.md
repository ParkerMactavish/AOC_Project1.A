# AOC_Project1.A
DMAC and SRAM design and test
***
【DMAC腳位說明】
1. addr_src是DRAM讀或寫的起始位址(單位byte)
2. addr_dst是SRAM讀或寫的起始位址(單位byte)
3. size是要讀寫的資料有幾個byte
4. d2s是指DRAM準備讓DMAC搬資料到SRAM了沒
5. start是enable
---
【Note】
1. 正常讀寫是以1個byte為單位(0~0x800000)
2. mem是以4個byte為單位(0~0x200000)
