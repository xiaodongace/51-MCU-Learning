#ifndef RTX51TNY_IDE_COMPAT_H
#define RTX51TNY_IDE_COMPAT_H

/*
 * CLion/Clang index-only compatibility header for Keil RTX51 Tiny.
 *
 * Keil C51 obtains the real RTX51TNY.H from its Windows installation.
 * This project-local copy exists solely because CLion runs on macOS and
 * cannot search Keil's Windows system include directory.  It is found via
 * cmake/ide_include, which is intentionally absent from Keil's IncludePath.
 */

typedef unsigned char rtx51_task_id_t;

/* RTX51 Tiny wait-event flags. */
#define K_SIG 0x01u
#define K_TMO 0x02u
#define K_IVL 0x80u

/* Task control APIs. */
void os_create_task(rtx51_task_id_t task_id);
void os_delete_task(rtx51_task_id_t task_id);
void os_send_signal(rtx51_task_id_t task_id);
void os_clear_signal(rtx51_task_id_t task_id);

/* Waiting APIs: wait for an event, optionally with a timeout interval. */
void os_wait1(unsigned char wait_event);
void os_wait2(unsigned char wait_event, unsigned char interval);

#endif
