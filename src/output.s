.text
add:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$t0, _framesize_add
sub	$sp, $sp, $t0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_add:
.data
__STR_add0: .asciiz "in add function"
.text
la	$s0, __STR_add0
li	$v0, 4
move	$a0, $s0
syscall
lw	$s1, 8($fp)
li	$v0, 1
move	$a0, $s1
syscall
lw	$s2, 12($fp)
li	$v0, 1
move	$a0, $s2
syscall
add	$t1, $s1, $s2
j	End_LogicalShort_add1
End_LogicalShort_add1:
move	$v0, $t1
j	_end_add
# epilogue sequence
_end_add:
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
_framesize_add: .word 32
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
li	$s0, 3
li	$s1, 5
addi	$sp, $sp, -8
li	$s2, 3
sw	$s2, 4($sp)
li	$s1, 5
sw	$s1, 8($sp)
jal	add
addi	$sp, $sp, 8
move	$s3, $v0
move	$s4, $s3
.data
__STR_main2: .asciiz "main function\n"
.text
la	$s5, __STR_main2
li	$v0, 4
move	$a0, $s5
syscall
li	$v0, 1
move	$a0, $s4
syscall
li	$s6, 0
move	$v0, $s6
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
_framesize_main: .word 36
