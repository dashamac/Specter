#pragma once
#include <ntddk.h>
#include <intrin.h>
#include <limits.h>
#include <cstdint>

#include "windows_structs.hpp"
#include "core/status_codes.hpp"

// We like nice declarations
typedef signed char        int8_t;        /**< Signed 8-bit integer */
typedef short              int16_t;       /**< Signed 16-bit integer */
typedef int                int32_t;       /**< Signed 32-bit integer */
typedef long long          int64_t;       /**< Signed 64-bit integer */
typedef unsigned char      uint8_t;       /**< Unsigned 8-bit integer */
typedef unsigned short     uint16_t;      /**< Unsigned 16-bit integer */
typedef unsigned int       uint32_t;      /**< Unsigned 32-bit integer */
typedef unsigned long long uint64_t;      /**< Unsigned 64-bit integer */

 /**
  * @brief Logging macro for error messages.
  *
  * This macro logs error messages with the file name and line number.
  *
  * @param fmt Format string for the message.
  * @param ... Arguments for the format string.
  */
#define project_log_error(fmt, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-] " "[%s:%d] " fmt "\n",  __LINE__, ##__VA_ARGS__)

  /**
   * @brief Logging macro for warning messages.
   *
   * This macro logs warning messages with the file name and line number.
   *
   * @param fmt Format string for the message.
   * @param ... Arguments for the format string.
   */
#define project_log_warning(fmt, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[~] " "[%s:%d] " fmt "\n",  __LINE__, ##__VA_ARGS__)

   /**
    * @brief Logging macro for success messages.
    *
    * This macro logs success messages with the file name and line number.
    *
    * @param fmt Format string for the message.
    * @param ... Arguments for the format string.
    */
#define project_log_success(fmt, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[+] " "[%s:%d] " fmt "\n",  __LINE__, ##__VA_ARGS__)

    /**
     * @brief Logging macro for informational messages.
     *
     * This macro logs informational messages with the file name and line number.
     *
     * @param fmt Format string for the message.
     * @param ... Arguments for the format string.
     */
#define project_log_info(fmt, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[*] " "[%s:%d] " fmt "\n",  __LINE__, ##__VA_ARGS__)

     /**
      * @brief Wrapper function to get the physical address of a virtual address.
      *
      * This function returns the physical address corresponding to the given virtual address.
      *
      * @param virtual_address The virtual address to convert.
      * @return The physical address corresponding to the given virtual address.
      */
inline uint64_t win_get_physical_address(void* virtual_address) {
    return MmGetPhysicalAddress(virtual_address).QuadPart;
}

/**
 * @brief Wrapper function to get the virtual address corresponding to a physical address.
 *
 * This function returns the virtual address corresponding to the given physical address.
 *
 * @param physical_address The physical address to convert.
 * @return The virtual address corresponding to the given physical address.
 */
inline uint64_t win_get_virtual_address(uint64_t physical_address) {
    PHYSICAL_ADDRESS phys_addr = { 0 };
    phys_addr.QuadPart = physical_address;

    return (uint64_t)(MmGetVirtualForPhysical(phys_addr));
}

/**
 * @brief Wrapper function to sleep for a specified number of milliseconds.
 *
 * This function puts the calling thread to sleep for a specified duration in milliseconds.
 *
 * @param milliseconds The duration in milliseconds to sleep.
 */
inline void sleep(LONG milliseconds) {
    LARGE_INTEGER interval;

    // Convert milliseconds to 100-nanosecond intervals
    interval.QuadPart = -((LONGLONG)milliseconds * 10000);

    KeDelayExecutionThread(KernelMode, false, &interval);
}

/**
 * @brief External declaration for a function to get the process number.
 *
 * This function retrieves the process number.
 *
 * @return The process number.
 */
extern "C" uint32_t get_proc_number(void);

/**
 * @brief External declaration for an assembly handler function.
 *
 * This function serves as a handler for assembly-level operations.
 */
extern "C" void asm_handler(void);

/**
 * @brief External declaration for the `KeStackAttachProcess` function.
 *
 * This function attaches the current thread to the specified process's address space.
 *
 * @param PROCESS A pointer to the process to attach to.
 * @param ApcState A pointer to the APC state for the thread.
 */
extern "C" NTKERNELAPI VOID KeStackAttachProcess(PRKPROCESS PROCESS, PKAPC_STATE ApcState);

/**
 * @brief External declaration for the `KeUnstackDetachProcess` function.
 *
 * This function detaches the current thread from the address space of the attached process.
 *
 * @param ApcState A pointer to the APC state for the thread.
 */
extern "C" NTKERNELAPI VOID KeUnstackDetachProcess(PKAPC_STATE ApcState);

/**
 * @brief External declaration for the PsLoadedModuleList symbol.
 *
 * This symbol provides access to the list of loaded modules in the system.
 */
extern "C" PLIST_ENTRY PsLoadedModuleList;

/**
 * @brief Global variable for the base address of the driver.
 *
 * This variable holds the base address of the loaded driver in memory.
 */
inline void* g_driver_base;

/**
 * @brief Global variable for the size of the driver.
 *
 * This variable holds the size of the loaded driver.
 */
inline uint64_t g_driver_size;

/**
 * @brief Session ID of the process that loaded the driver.
 *
 * Captured at driver_entry from the loader's process. Used to find the winlogon
 * process in that session for win32k data-pointer hook installation, since
 * win32k data is session-private.
 */
inline ULONG g_target_session_id = 0;


// Define MAXULONG32 and MAXULONG64 if not already defined (WDK compatibility)
#ifndef MAXULONG32
#define MAXULONG32 ((uint32_t)0xFFFFFFFFUL)
#endif

#ifndef MAXULONG64
#define MAXULONG64 ((uint64_t)0xFFFFFFFFFFFFFFFFULL)
#endif

// Kernel API declarations
extern "C" NTKERNELAPI ULONGLONG KeQueryInterruptTime(void);
