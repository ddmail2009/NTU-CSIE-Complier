.data
_a:	.space 24
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

# local array get
    # get the offset
    li $s7, 4
    # get the base address of b which is -12($fp)
    add $s1, $fp, -12
    # add the offset to the address
    add $s1, $s1, $s7
    # load the word in that address
    lw  $s2, 0($s1)

# global array get
    # get the offset
    li $s7, 4
    # get the base address of a which is stored in global field
    la $s1, _a
    # add the offset to the address
    add $s1, $s1, $s7
    # load the word in that address
    lw $s2, 0($s1)

li	$v0, 1
move	$a0, $s2
syscall
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
_framesize_main: .word 32
