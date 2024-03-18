._start
    STO 0 #42
    JSR .is_prime
    STO 1 #ff
    BRN ._halt
.is_prime ; input in 0, output bool in 1
    ;todo
._halt
    HLT