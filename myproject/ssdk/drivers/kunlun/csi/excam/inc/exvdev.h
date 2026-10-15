/**
 * @file exvdev.h
 * @brief externel video device header
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __EXVDEV_H__
#define __EXVDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/kunlun/csi/cam_res.h>

#include "sdrv-cam-os-def.h"
#include "vdev_defs.h"

struct vdev_fwnode_endpoint {
	enum vdev_mbus_type bus_type;
	unsigned int flags;
};

/**
 * struct vdev_mbus_framefmt - Media bus format enumeration
 * @index:  for enum index
 * @code:   data format code (from enum vdev_mbus_format)
 * @field:  used interlacing type (from enum vdev_field_fmt)
 * @colorspace: colorspace of the data (from enum vdev_colorspace)
 * @width: frame width
 * @height: frame height
 * @field_w: field width, [0] for odd, [1] for even
 * @field_h: field height, [0] for odd, [1] for even
 */
struct vdev_mbus_framefmt {
	uint32_t index;
	uint32_t code;
	uint32_t field;
	uint32_t colorspace;
	uint32_t width;
	uint32_t height;
	uint32_t field_w[2];
	uint32_t field_h[2];
};

/**
 * struct vdev_frame_size_enum - frame size enumeration
 * @index: format index during enumeration
 * @code: format code (MEDIA_BUS_FMT_ definitions)
 */
struct vdev_frame_size_enum {
	uint32_t index;
	uint32_t code;
	uint32_t min_width;
	uint32_t max_width;
	uint32_t min_height;
	uint32_t max_height;
};

struct vdev_fract {
	uint32_t numerator;
	uint32_t denominator;
};

/**
 * struct vdev_frame_interval_enum - Frame interval enumeration
 * @index: frame interval index during enumeration
 * @code: format code (MEDIA_BUS_FMT_ definitions)
 * @width: frame width in pixels
 * @height: frame height in pixels
 * @interval: frame interval in seconds
 */
struct vdev_frame_interval_enum {
	uint32_t index;
	uint32_t code;
	uint32_t width;
	uint32_t height;
	struct vdev_fract interval;
};

struct vdev_device;

/**
 * struct vdev_dev_ops - vdev operations
 * @s_power: power on/off, (MUST)
 * @get_fmt: get mbus fmt (MUST)
 * @s_stream: stream start/stop (MUST)
 * @close: exit and deinit device (MUST)
 */
struct vdev_dev_ops {
	int (*s_power)(struct vdev_device *vdev, int on);
	int (*enum_mbus_fmt)(struct vdev_device *vdev,
			     struct vdev_mbus_framefmt *fme);
	int (*enum_frame_size)(struct vdev_device *vdev,
			       struct vdev_frame_size_enum *fse);
	int (*enum_frame_interval)(struct vdev_device *vdev,
				   struct vdev_frame_interval_enum *fie);
	int (*g_frame_interval)(struct vdev_device *vdev,
				struct vdev_fract *fi);
	int (*s_frame_interval)(struct vdev_device *vdev, struct vdev_fract fi);
	int (*get_fmt)(struct vdev_device *vdev,
		       struct vdev_mbus_framefmt *fmt);
	int (*set_fmt)(struct vdev_device *vdev, struct vdev_mbus_framefmt fmt);
	int (*s_stream)(struct vdev_device *vdev, int enable);
	int (*close)(struct vdev_device *vdev);
};

struct vdev_device {
	void *priv_data;
	u8     ch_id;
	struct vdev_fwnode_endpoint ep;
	struct vdev_mbus_framefmt fmt;
	struct vdev_fract frame_interval;
	struct vdev_dev_ops ops;
};

struct external_camera_api {
	char name[32];
	struct vdev_device *(*init)(void *init_data);
};

#define v_abs(a) ((a > 0) ? a : (0 - a))

#define vdev_find_nearest_size(array, array_size, width_field, height_field,   \
			       width, height)                                  \
	({                                                                     \
		(__typeof(&(array)[0])) __vdev_find_nearest_size(              \
			(array), array_size, sizeof(*(array)),                 \
			offsetof(__typeof(*(array)), width_field),             \
			offsetof(__typeof(*(array)), height_field), width,     \
			height);                                               \
	})

inline const void *
__vdev_find_nearest_size(const void *array, size_t array_size,
			 size_t entry_size, size_t width_offset,
			 size_t height_offset, uint32_t width, uint32_t height)
{
	uint32_t error, min_error = (1 << 31);
	const void *best = NULL;
	unsigned int i;
	int32_t w = width;
	int32_t h = height;

	if (!array)
		return NULL;

	for (i = 0; i < array_size; i++, array += entry_size) {
		const int32_t *entry_width = array + width_offset;
		const int32_t *entry_height = array + height_offset;

		error = v_abs(*entry_width - w) + v_abs(*entry_height - h);

		if (error > min_error)
			continue;

		min_error = error;
		best = array;

		if (!error)
			break;
	}

	return best;
}

const void *get_excam_api(const char *name);

#ifdef __cplusplus
extern "C" {
#endif

#endif
