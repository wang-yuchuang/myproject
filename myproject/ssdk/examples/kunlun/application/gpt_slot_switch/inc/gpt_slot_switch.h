/************************************************************************
 * Copyright(c) 2026  Semidrive
 * All rights reserved.
 ************************************************************************/
#ifndef __GPT_SLOT_SWITCH_H__
#define __GPT_SLOT_SWITCH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* GPT A/B slot switch CLI: slot_switch [a|b|status]
 *   a       : mark slot A active (switch GPT active bit to A)
 *   b       : mark slot B active (switch GPT active bit to B)
 *   status  : show current active slot and slot attributes
 */
int gpt_slot_switch_cmd(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __GPT_SLOT_SWITCH_H__ */
