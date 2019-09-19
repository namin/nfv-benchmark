// types
u8, u16, u32, u64, u128
i8, i16, i32, i64, i128
uptr -- (pointer type)
bytes
packet
d32, d64 (float/double)


# type operators
(iX, uX) (+-/*&|^) (iX, uX)

# Cacheable result
def hash(bytes val) pure {
}

# Not cacheable
def test(bytes val) {
}

# Just enough to capture the structure of the code?

