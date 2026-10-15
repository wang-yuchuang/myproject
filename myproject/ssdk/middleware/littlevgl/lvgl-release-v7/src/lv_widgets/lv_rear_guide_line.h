/**
 * @file lv_rear_guide_line.h
 *
 */

#ifndef LV_REAR_GUIDE_LINE_H
#define LV_REAR_GUIDE_LINE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_conf_internal.h"
#include "../lv_core/lv_indev.h"

#if LV_USE_LINE != 0

#include "../lv_core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
     C        D
       *     *
      **     **
     *         *
    ***       ***
   *             *
  *               *
 A                 B
*
*/

typedef enum
{
    STATIC_LINE,
    DYNAMIC_LINE
} rear_guide_mode;

typedef struct {
    lv_obj_t obj;
    uint16_t rotation;
    uint32_t ellipse_a;//x^2/a^2+y^2/b^2 = 1  this is a
    uint32_t ellipse_b;//x^2/a^2+y^2/b^2 = 1  this is b
    rear_guide_mode mode;
}lv_rear_guide_t;

/**********************
 * GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_rear_guide_line_create(lv_obj_t * parent,lv_obj_t * copy,int mode);

/*=====================
 * Setter functions
 *====================*/


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the y inversion attribute
 * @param obj       pointer to a line object
 * @return          true: y inversion is enabled, false: disabled
 */

void lv_rgl_set_focus(lv_point_t a_point,lv_point_t b_point);
void lv_obj_set_rear_angle(void * obj,int32_t v);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LINE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_REAR_GUIDE_LINE_H*/
