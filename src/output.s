.text
main:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
li	$s0, -4
add	$fp, $sp, $s0
li	$s1, -8
add	$sp, $sp, $s1
lw	$s2, _framesize_main
sub	$sp, $sp, $s2
sw	$t0, 4($sp)
sw	$t1, 8($sp)
sw	$t2, 12($sp)
sw	$t3, 16($sp)
sw	$t4, 20($sp)
sw	$t5, 24($sp)
sw	$t6, 28($sp)
sw	$t7, 32($sp)
_begin_main:
li.s	$f0, 3.200000
s.s	$f0, -16($fp)
li	$v0, 5
syscall
move	$s3, $v0
move	$s4, $s3
li	$v0, 5
syscall
move	$s5, $v0
move	$s6, $s5
mul	$t0, $s4, $s6
move	$s7, $t0
l.s	$f1, -16($fp)
mtc1	$s4, $f2
cvt.s.w	$f2, $f2
c.lt.s	$f1, $f2
bc1f _False0
li	$t1, 0
_False0:
li	$t1, 1
sw	$s6, -8($fp)
not	$6889928, $6889928
lw	$s6, -8($fp)
div	$t2, $s6, $s4
li	$s2, 0
slt	$t3, $t2, $s2
sw	$s6, -8($fp)
not	$6889928, $6889928
and	$t4, $t3, $s6
or	$t5, $s6, $t4
beqz	$t5, _IfBranch_9_9
li	$v0, 4
la	$a0, _Str_main0
syscall
j	_End_IF_9_9
_IfBranch_9_9:
li	$v0, 4
la	$a0, _Str_main1
syscall
_End_IF_9_9:
li	$s6, 0
move	$v0, $s6
j	_end_main
# epilogue sequence
_end_main:
lw	$t0, 4($sp)
lw	$t1, 8($sp)
lw	$t2, 12($sp)
lw	$t3, 16($sp)
lw	$t4, 20($sp)
lw	$t5, 24($sp)
lw	$t6, 28($sp)
lw	$t7, 32($sp)
lw	$ra, 4($fp)
sw	$s4, -4($fp)
li	$s4, 4
add	$sp, $fp, $s4
lw	$fp, 0($fp)
li	$v0, 10
.data
_framesize_main: .word 16
_Str_main0: .asciiz "True\n"
_Str_main1: .asciiz "False\n"
