#include "graphics.h"
#include "game.h"

/*
 * 
 */
#define TEXT_DRAW_SS_INGAME_FUNC                (0x004D8420)
#define TEXT_GETWIDTH_INGAME_FUNC               (0x004D8130)

#define BOX_DRAW_SS_INGAME_FUNC                 (0x005BE610)

//--------------------------------------------------------
int gfxWorldSpaceToScreenSpace(VECTOR position, int * x, int * y)
{
    int output = 0;

    asm __volatile__ (
        "addiu      $sp, $sp, -0x40     \n"
        "sq         $ra, 0x00($sp)      \n"
        "sq         $s0, 0x10($sp)      \n"
        "sq         $s1, 0x20($sp)      \n"
        "swc1       $f20, 0x30($sp)     \n"

        "move       $s0, %0             \n"
        "move       $s1, %1             \n"
        "li.s       $f20, 0.0625        \n"
        
        "lq		    $a0, 0x00(%2)	    \n"
        "jal		0x004BFCA8	        \n"
        "lui		$v1, 0x001D	        \n"
        "mtc1		$v0, $f01   	    \n"
        "addiu		$a0, $v1, 0x3F40    \n"
        "lwc1		$f02, 0x10($a0)	    \n"
        "cvt.s.w	$f02, $f02	        \n"
        "prot3w		$v1, $v0	        \n"
        "mtc1		$v1, $f00	        \n"
        "lwc1		$f03, 0x14($a0)	    \n"
        "cvt.s.w	$f03, $f03	        \n"
        "lw		    $v0, 0x18($a0)	    \n"
        "sub.s		$f01, $f01, $f02    \n"
        "addiu		$v0, $v0, 0x40	    \n"
        "sub.s		$f00, $f00, $f03    \n"
        "mul.s		$f01, $f01, $f20    \n"
        "mul.s		$f00, $f00, $f20    \n"
        "cvt.w.s	$f02, $f01  	    \n"
        "mfc1       $t0, $f02           \n"
        "sw		    $t0, 0x00($s0)	    \n"
        "cvt.w.s	$f01, $f00	        \n"
        "mfc1       $t1, $f01           \n"
        "sw		    $t1, 0x00($s1)	    \n"

        "lw         $v0, 0x18($a0)      \n"
        "addiu      $v0, $v0, 0x40      \n"
        "slt        $v1, $v0, $t0       \n"
        "bne        $v1, $0, exit       \n"
        "slti       $v1, $t0, -0x40     \n"
        "bne        $v1, $0, exit       \n"
        "lw         $v0, 0x1C($a0)      \n"
        "addiu      $v0, $v0, 0x40      \n"
        "slt        $v1, $v0, $t1       \n"
        "bne        $v1, $0, exit       \n"
        "slti       $v1, $t1, -0x40     \n"
        "bne        $v1, $0, exit       \n"
        "li         %3, 1              \n"

        "exit:                          \n"
        "lq         $ra, 0x00($sp)      \n"
        "lq         $s0, 0x10($sp)      \n"
        "lq         $s1, 0x20($sp)      \n"
        "lwc1       $f20, 0x30($sp)     \n"
        "addiu      $sp, $sp, 0x40      \n"
        : : "r" (x), "r" (y), "r" (position), "r" (output)
    );

    return output;
}

//--------------------------------------------------------
int gfxScreenSpaceText(int x, int y, float scaleX, float scaleY, u32 color, const char * string, int length)
{
    if (isInGame())
    {
        // draw
        ((int (*)(u32,const char*,long,u64,u64,u64,float,float,float,float,float,float))TEXT_DRAW_SS_INGAME_FUNC)(color, string, length, 1, 0, 0x80000000, (float)x, (float)y, scaleX, scaleY, 0, 0);

        // return x + width
        return x + ((int (*)(const char*,long,float))TEXT_GETWIDTH_INGAME_FUNC)(string, length, scaleX);
    }

    return x;
}

//--------------------------------------------------------
void gfxScreenSpaceBox(RECT * rect, u32 colorTL, u32 colorTR, u32 colorBL, u32 colorBR)
{
    u32 buffer[11];

    if (isInGame())
    {
        buffer[0] = 8;
        buffer[1] = 0;
        buffer[2] = 0x005C97AC;
        buffer[3] = 0;
        buffer[4] = 0;
        buffer[5] = 0xBE130000;
        buffer[6] = colorTL;
        buffer[7] = colorTR;
        buffer[8] = colorBL;
        buffer[9] = colorBR;
        buffer[10] = 2;

		((void (*)(void *, void *))BOX_DRAW_SS_INGAME_FUNC)(rect, buffer);
    }
}
