/**
 * @file    SWD_flash.c
 * @brief   通过SWD协议对MCU的FLASH编程
 */
#include "swd_host.h"
#include "SWD_flash.h"


extern const program_target_t flash_algo;


error_t target_flash_init(uint32_t flash_start) {
  if(0 == swd_set_target_state_hw(RESET_PROGRAM)) {
    return ERROR_RESET;
  }

  // 下载编程算法到目标MCU的SRAM，并初始化
  if(0 == swd_write_memory(flash_algo.algo_start, (uint8_t*)flash_algo.algo_blob, flash_algo.algo_size)) {
    return ERROR_ALGO_DL;
  }

  if(0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.init, flash_start, 0, 0, 0)) {
    return ERROR_INIT;
  }

  return ERROR_SUCCESS;
}

error_t target_flash_uninit(void) {
  swd_set_target_state_hw(RESET_RUN);

  swd_off();
  return ERROR_SUCCESS;
}
/*
Verify
Prototype

U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff)
Function description

Compares a specified number of bytes of a provided data buffer with the content of the device

Parameters

Addr: Start address in memory which should be compared
NumBytes: Number of bytes to be compared
pBuff: Pointer to the data to be compared
Return values

== (Addr + NumBytes): O.K.
!= (Addr + NumBytes): *not* O.K. (ideally the fail address is returned)
Code Example

U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff) {
  unsigned char *pFlash;
  unsigned long r;

  pFlash = (unsigned char *)Addr;
  r = Addr + NumBytes;
  do {
      if (*pFlash != *pBuff) {
        r = (unsigned long)pFlash;
        break;
      }
      pFlash++;
      pBuff++;
  } while (--NumBytes);
  return r;
}
*/
error_t target_flash_verify(uint32_t addr, const uint8_t* buf, uint32_t size) {
  if(!swd_write_memory(flash_algo.program_buffer, (uint8_t*)buf, size)) {
    return ERROR_WRITE;
  }
  {
    DEBUG_STATE state = {{0}, 0};
    // Call flash algorithm function on target and wait for result.
    state.r[0]     = addr;                   // R0: Argument 1
    state.r[1]     = size;                   // R1: Argument 2
    state.r[2]     = flash_algo.program_buffer;                   // R2: Argument 3
    state.r[3]     = 0;                   // R3: Argument 4
    state.r[9]     = flash_algo.sys_call_s.static_base;    // SB: Static Base
    state.r[13]    = flash_algo.sys_call_s.stack_pointer;  // SP: Stack Pointer
    state.r[14]    = flash_algo.sys_call_s.breakpoint;     // LR: Exit Point
    state.r[15]    = flash_algo.verify;                        // PC: Entry Point
    state.xpsr     = 0x01000000;          // xPSR: T = 1, ISR = 0

    if(!swd_write_debug_state(&state)) {
      return ERROR_VERIFY;
    }

    if(!swd_wait_until_halted()) {
      return ERROR_VERIFY;
    }

    if(!swd_read_core_register(0, &state.r[0])) {
      return ERROR_VERIFY;
    }

    // Flash functions return 0 if successful.
    if(state.r[0] == addr  + size) {
      return ERROR_SUCCESS;
    } else {
      return ERROR_VERIFY;
    }
  }
  return ERROR_SUCCESS;
}
error_t target_flash_program_page(uint32_t addr, const uint8_t* buf, uint32_t size) {
  while(size > 0) {
    uint32_t write_size = size > flash_algo.program_buffer_size ? flash_algo.program_buffer_size : size;

    // Write page to buffer
    if(!swd_write_memory(flash_algo.program_buffer, (uint8_t*)buf, write_size)) {
      return ERROR_ALGO_DATA_SEQ;
    }

    // Run flash programming
    if(!swd_flash_syscall_exec(&flash_algo.sys_call_s,
                               flash_algo.program_page,
                               addr,
                               write_size,//flash_algo.program_buffer_size,
                               flash_algo.program_buffer,
                               0)) {
      return ERROR_WRITE;
    }

    addr += write_size;
    buf  += write_size;
    size -= write_size;
  }

  return ERROR_SUCCESS;
}

error_t target_flash_erase_sector(uint32_t addr) {
  if(0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_sector, addr, 0, 0, 0)) {
    return ERROR_ERASE_SECTOR;
  }

  return ERROR_SUCCESS;
}

error_t target_flash_erase_chip(void) {
  error_t status = ERROR_SUCCESS;

  if(0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_chip, 0, 0, 0, 0)) {
    return ERROR_ERASE_ALL;
  }

  return status;
}
