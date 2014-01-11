.data
_delta:	.float 0.0
.text
__InlineSubRoutine_Start_0:
li.s	$f0, 1.000000
s.s	$f0, _delta
j __InlineSubRoutine_End_1
.text
floor:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$t0, _framesize_floor
sub	$sp, $sp, $t0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_floor:
l.s	$f1, 8($fp)
mov.s	$f2, $f1
cvt.w.s	$f2, $f2
mfc1	$s0, $f2
move	$v0, $s0
j	_end_floor
# epilogue sequence
_end_floor:
lw	$s0, -4($fp)
lw	$s1, -8($fp)
lw	$s2, -12($fp)
lw	$s3, -16($fp)
lw	$s4, -20($fp)
lw	$s5, -24($fp)
lw	$s6, -28($fp)
lw	$s7, -32($fp)
lw	$ra, 4($fp)
addi	$sp, $fp, 4
lw	$fp, 0($fp)
jr	$ra
.data
_framesize_floor: .word 36
.text
ceil:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$t0, _framesize_ceil
sub	$sp, $sp, $t0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_ceil:
l.s	$f0, 8($fp)
li	$s0, 0
mtc1	$s0, $f1
cvt.s.w	$f1, $f1
c.lt.s	$f0, $f1
bc1f _False0
li	$t1, 0
j	End_False0
_False0:
li	$t1, 1
End_False0:
j	End_LogicalShort_ceil4
End_LogicalShort_ceil4:
beqz	$t1, __IfBranch_ceil3
l.s	$f2, _delta
add.s	$f3, $f0, $f2
j	End_LogicalShort_ceil5
End_LogicalShort_ceil5:
mov.s	$f4, $f3
cvt.w.s	$f4, $f4
mfc1	$s1, $f4
j	_End__IF_ceil2
__IfBranch_ceil3:
sub.s	$f4, $f0, $f2
j	End_LogicalShort_ceil6
End_LogicalShort_ceil6:
mov.s	$f5, $f4
cvt.w.s	$f5, $f5
mfc1	$s1, $f5
_End__IF_ceil2:
move	$v0, $s1
j	_end_ceil
# epilogue sequence
_end_ceil:
lw	$s0, -4($fp)
lw	$s1, -8($fp)
lw	$s2, -12($fp)
lw	$s3, -16($fp)
lw	$s4, -20($fp)
lw	$s5, -24($fp)
lw	$s6, -28($fp)
lw	$s7, -32($fp)
lw	$ra, 4($fp)
addi	$sp, $fp, 4
lw	$fp, 0($fp)
jr	$ra
.data
_framesize_ceil: .word 40
.text
main:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$t0, _framesize_main
sub	$sp, $sp, $t0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_main:
j __InlineSubRoutine_Start_0
__InlineSubRoutine_End_1:
.data
__STR_main7: .asciiz "Enter number :"
.text
la	$s0, __STR_main7
li	$v0, 4
move	$a0, $s0
syscall
li	$v0, 6
syscall
mov.s	$f0, $f0
mov.s	$f1, $f0
s.s	$f1, -36($fp)
addi	$sp, $sp, -4
l.s	$f2, -36($fp)
s.s	$f2, 4($sp)
jal	ceil
addi	$sp, $sp, 4
move	$s1, $v0
li	$v0, 1
move	$a0, $s1
syscall
.data
__STR_main8: .asciiz "\n"
.text
la	$s2, __STR_main8
li	$v0, 4
move	$a0, $s2
syscall
l.s	$f0, -36($fp)
s.s	$f0, -36($fp)
addi	$sp, $sp, -4
l.s	$f1, -36($fp)
s.s	$f1, 4($sp)
jal	floor
addi	$sp, $sp, 4
move	$s3, $v0
li	$v0, 1
move	$a0, $s3
syscall
la	$s4, __STR_main8
li	$v0, 4
move	$a0, $s4
syscall
l.s	$f0, -36($fp)
s.s	$f0, -36($fp)
addi	$sp, $sp, -4
l.s	$f1, -36($fp)
s.s	$f1, 4($sp)
jal	ceil
addi	$sp, $sp, 4
move	$s5, $v0
l.s	$f0, -36($fp)
s.s	$f0, -36($fp)
addi	$sp, $sp, -4
l.s	$f1, -36($fp)
s.s	$f1, 4($sp)
jal	floor
addi	$sp, $sp, 4
move	$s6, $v0
add	$t0, $s5, $s6
j	End_LogicalShort_main9
End_LogicalShort_main9:
li.s	$f0, 2.000000
mtc1	$t0, $f1
cvt.s.w	$f1, $f1
div.s	$f1, $f1, $f0
j	End_LogicalShort_main10
End_LogicalShort_main10:
mov.s	$f2, $f1
li	$v0, 2
mov.s	$f12, $f2
syscall
la	$s7, __STR_main8
li	$v0, 4
move	$a0, $s7
syscall
li	$s5, 0
move	$v0, $s5
j	_end_main
# epilogue sequence
_end_main:
lw	$s0, -4($fp)
lw	$s1, -8($fp)
lw	$s2, -12($fp)
lw	$s3, -16($fp)
lw	$s4, -20($fp)
lw	$s5, -24($fp)
lw	$s6, -28($fp)
lw	$s7, -32($fp)
lw	$ra, 4($fp)
addi	$sp, $fp, 4
lw	$fp, 0($fp)
li	$v0, 10
syscall
.data
_framesize_main: .word 40
