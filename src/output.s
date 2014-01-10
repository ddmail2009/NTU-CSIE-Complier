.text
main:
#[32m register swap, register '$ra' save to addr: 0($sp)[m
sw	$ra, 0($sp)
#[32m register swap, register '$fp' save to addr: -4($sp)[m
sw	$fp, -4($sp)
li	$s0, -4
add	$fp, $sp, $s0
li	$s1, -8
add	$sp, $sp, $s1
lw	$s2, _framesize_main
sub	$sp, $sp, $s2
#[32m register swap, register '$s0' save to addr: 4($sp)[m
sw	$s0, 4($sp)
#[32m register swap, register '$s1' save to addr: 8($sp)[m
sw	$s1, 8($sp)
#[32m register swap, register '$s2' save to addr: 12($sp)[m
sw	$s2, 12($sp)
#[32m register swap, register '$s3' save to addr: 16($sp)[m
sw	$s3, 16($sp)
#[32m register swap, register '$s4' save to addr: 20($sp)[m
sw	$s4, 20($sp)
#[32m register swap, register '$s5' save to addr: 24($sp)[m
sw	$s5, 24($sp)
#[32m register swap, register '$s6' save to addr: 28($sp)[m
sw	$s6, 28($sp)
#[32m register swap, register '$s7' save to addr: 32($sp)[m
sw	$s7, 32($sp)
_begin_main:
li.s	$f0, 3.200000
s.s	$f0, -16($fp)
li	$s3, 3
not	$t0, $s3
li	$s4, 1
add	$t0, $t0, $s4
move	$s5, $t0
li	$s6, 3
move	$s7, $s6
mul	$t1, $s5, $s7
move	$s0, $t1
l.s	$f1, -16($fp)
mtc1	$s5, $f2
cvt.s.w	$f2, $f2
c.lt.s	$f1, $f2
bc1f _False0
li	$t2, 0
j	End_False0
_False0:
li	$t2, 1
End_False0:
li	$v0, 1
move	$a0, $t2
syscall
#[32m register swap, register '$s0' save to addr: -12($fp)[m
sw	$s0, -12($fp)
la	$s0, _Str_main0
li	$v0, 4
move	$a0, $s0
syscall
mtc1	$s5, $f3
cvt.s.w	$f3, $f3
c.lt.s	$f1, $f3
bc1f _False1
li	$t3, 0
j	End_False1
_False1:
li	$t3, 1
End_False1:
beqz	$t3, _TrueNot0
li	$t4, 0
j	_EndNot0
_TrueNot0:
li	$t4, 1
_EndNot0:
li	$v0, 1
move	$a0, $t4
syscall
la	$s1, _Str_main1
li	$v0, 4
move	$a0, $s1
syscall
div	$t5, $s7, $s5
#[32m register swap, register '$s5' save to addr: -4($fp)[m
sw	$s5, -4($fp)
li	$s5, 0
slt	$t6, $t5, $s5
lw	$s0, -12($fp)
beqz	$s0, _TrueNot1
li	$t7, 0
j	_EndNot1
_TrueNot1:
li	$t7, 1
_EndNot1:
and	$t8, $t6, $t7
li	$v0, 1
move	$a0, $t8
syscall
la	$s5, _Str_main2
li	$v0, 4
move	$a0, $s5
syscall
lw	$s6, -4($fp)
mtc1	$s6, $f4
cvt.s.w	$f4, $f4
c.lt.s	$f1, $f4
bc1f _False2
li	$t9, 0
j	End_False2
_False2:
li	$t9, 1
End_False2:
beqz	$t9, _TrueNot2
li	$t5, 0
j	_EndNot2
_TrueNot2:
li	$t5, 1
_EndNot2:
div	$t7, $s7, $s6
#[32m register swap, register '$s6' save to addr: -4($fp)[m
sw	$s6, -4($fp)
li	$s6, 0
slt	$t6, $t7, $s6
beqz	$s0, _TrueNot3
li	$t8, 0
j	_EndNot3
_TrueNot3:
li	$t8, 1
_EndNot3:
and	$t0, $t6, $t8
or	$t3, $t5, $t0
beqz	$t3, _IfBranch_15_9
#[32m register swap, register '$s7' save to addr: -8($fp)[m
sw	$s7, -8($fp)
la	$s7, _Str_main3
li	$v0, 4
move	$a0, $s7
syscall
j	_End_IF_15_9
_IfBranch_15_9:
la	$s4, _Str_main4
li	$v0, 4
move	$a0, $s4
syscall
_End_IF_15_9:
li	$s6, 0
move	$v0, $s6
j	_end_main
# epilogue sequence
_end_main:
lw	$s0, 4($sp)
lw	$s1, 8($sp)
lw	$s2, 12($sp)
lw	$s3, 16($sp)
lw	$s4, 20($sp)
lw	$s5, 24($sp)
lw	$s6, 28($sp)
lw	$s7, 32($sp)
lw	$ra, 4($fp)
#[32m register swap, register '$s0' save to addr: -12($fp)[m
sw	$s0, -12($fp)
li	$s0, 4
add	$sp, $fp, $s0
lw	$fp, 0($fp)
li	$v0, 10
syscall
.data
_framesize_main: .word 16
_Str_main0: .asciiz "\n"
_Str_main1: .asciiz "\n"
_Str_main2: .asciiz "\n"
_Str_main3: .asciiz "True\n"
_Str_main4: .asciiz "False\n"
