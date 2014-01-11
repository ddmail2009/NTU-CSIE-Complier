.text
is_prime:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$s0, _framesize_is_prime
sub	$sp, $sp, $s0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_is_prime:
lw	$s1, 8($fp)
li	$s2, 2
seq	$t0, $s1, $s2
li	$s3, 1
move	$v0, $s3
j	_end_is_prime
li	$s4, 2
div	$t1, $s1, $s4
li	$s5, 2
mul	$t2, $t1, $s5
sub	$t3, $s1, $t2
li	$s6, 0
seq	$t4, $t3, $s6
li	$s7, 0
move	$v0, $s7
j	_end_is_prime
li	$s7, 2
div	$t5, $s1, $s7
sw	$s1, 8($fp)
move	$s1, $t5
li	$s7, 3
move	$s2, $s7
sw	$s1, -40($fp)
__FOR_is_prime2:
lw	$s1, -40($fp)
sle	$t6, $s2, $s1
beqz	$t6, _End__FOR_is_prime2
lw	$s7, 8($fp)
div	$t7, $s7, $s2
mul	$t8, $t7, $s2
sub	$t9, $s7, $t8
sw	$s1, -40($fp)
li	$s1, 0
seq	$t4, $t9, $s1
li	$s6, 0
move	$v0, $s6
j	_end_is_prime
li	$s3, 2
add	$t2, $s2, $s3
move	$s2, $t2
j	__FOR_is_prime2
_End__FOR_is_prime2:
li	$s4, 1
move	$v0, $s4
j	_end_is_prime
sw	$s2, -36($fp)
# epilogue sequence
_end_is_prime:
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
_framesize_is_prime: .word 40
.text
main:
sw	$ra, 0($sp)
sw	$fp, -4($sp)
addi	$fp, $sp, -4
addi	$sp, $sp, -8
lw	$s0, _framesize_main
sub	$sp, $sp, $s0
sw	$s0, -4($fp)
sw	$s1, -8($fp)
sw	$s2, -12($fp)
sw	$s3, -16($fp)
sw	$s4, -20($fp)
sw	$s5, -24($fp)
sw	$s6, -28($fp)
sw	$s7, -32($fp)
_begin_main:
.data
__STR_main4: .asciiz "enter a range, for example, 5<ENTER> 23<ENTER>:"
.text
la	$s1, __STR_main4
li	$v0, 4
move	$a0, $s1
syscall
li	$v0, 5
syscall
move	$s2, $v0
move	$s3, $s2
li	$v0, 5
syscall
move	$s4, $v0
move	$s5, $s4
move	$s6, $s3
__FOR_main5:
slt	$t0, $s6, $s5
beqz	$t0, _End__FOR_main5
addi	$sp, $sp, -4
lw	$s7, -36($fp)
sw	$s7, 4($sp)
jal	is_prime
addi	$sp, $sp, 4
sw	$s7, -36($fp)
move	$s7, $v0
lw	$s2, -36($fp)
li	$v0, 1
move	$a0, $s2
syscall
sw	$s5, -44($fp)
.data
__STR_main7: .asciiz "\n"
.text
la	$s5, __STR_main7
li	$v0, 4
move	$a0, $s5
syscall
li	$s1, 1
add	$t1, $s2, $s1
move	$s2, $t1
j	__FOR_main5
_End__FOR_main5:
li	$s4, 0
move	$v0, $s4
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
_framesize_main: .word 44
