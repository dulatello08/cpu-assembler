; Binary search algorithm, high part of index is always the same,
; so can only operate on contiguous array that is also aligned with hex 100s,
; e.g 0xf1 is high part of index, 0x0-ff(size of array instead of ff) is low part, where algorithm will search,
; logic for carrying over index values when adding, and maintaining different high parts of indexes (low mid hi),
; is postponed due to being overly complex.

; R0 = target_scan_code
; R1 = base_address of the array (high part fixed at 0xF1)
; R2 = low (starting low index, initially 0x00)
; R3 = high (ending low index, initially 0xFF or size of array)
; R4 = mid (calculated mid index)
; R5 = temporary storage for mid_index low part
; R6 = value at array[mid]
; R7 = result (ASCII value or -1)
.search_ff

