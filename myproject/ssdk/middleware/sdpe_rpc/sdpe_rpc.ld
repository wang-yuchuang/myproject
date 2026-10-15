/*
 * Link script patch for sdpe rpc buffer sections.
 */
SECTIONS {
    .sdpe_rpc_buf SDPE_RPC_BUFFER_BASE (NOLOAD) : {
        . = ALIGN(4);
        __sdpe_rpc_buf_start = .;
        *(.sdpe_rpc_buf)
        __sdpe_rpc_buf_end = .;
    }

    ASSERT(. < SDPE_RPC_BUFFER_BASE + SDPE_RPC_BUFFER_SIZE, "RPC buffer too large")
}
INSERT BEFORE .ext.mem;
