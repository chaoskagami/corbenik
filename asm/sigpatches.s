.arm.little

.create "sig1.to", 0
.thumb
mov r0, #0
.close

.create "sig2.to", 0
.thumb
mov r0, #0
bx lr
.close
