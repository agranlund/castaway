

    .bss
    .even
    .balign 4
_RasterList:    ds.l    1

    .globl _RasterList

    .text
    .even
    .globl _VecRaster16
    .globl _VecRasterEnd


    .balign 4
_VecRaster16:
    clr.b   0xFA1B.w                // counter stop
    move.l  a0,-(sp)                // save regs
    move.l  _RasterList,a0
    move.l  (a0)+,0x8240.w
    move.l  (a0)+,0x8244.w
    move.l  (a0)+,0x8248.w
    move.l  (a0)+,0x824C.w
    move.l  (a0)+,0x8250.w
    move.l  (a0)+,0x8254.w
    move.l  (a0)+,0x8258.w
    move.l  (a0)+,0x825C.w

    move.l  (a0)+,0x0120.w          // next func
    move.w  (a0)+,0xFA20.w          // next counter
    move.l  a0,_RasterList

    move.l  (sp)+,a0                // restore regs
    move.b  #8,0xFA1B.w             // counter start
    bclr	#0,0xFA0F.w		        // TimerB done
    rte

_VecRasterEnd:
    bclr    #0,0xFA07.w             // TimerB disable
    bclr	#0,0xFA0F.w		        // TimerB done
    rte
