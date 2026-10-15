/*
 * csi_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: csi controller driver (system adaptive)
 *
 * Revision History:
 * -----------------
 */

#include "sdrv-cam-os-def.h"
#include "vdev_defs.h"
#include "csi_hw.h"
#include "csi_drv.h"
#include "sd_errcode.h"

#define MAX_CSI_INST CONFIG_CSI_NUM
#define MAX_CSI_CHANNEL CONFIG_CSI_CH_NUM
#define MAX_BUF_CNT 16
#define MAX_QLEN 1
#define SKIP_FRAME_NUM (1 << 0)

enum channel_status {
	STOPPED,
	RUNNING,
	IDLE,
};

int cam_log[4] = {
	SSDK_ERR, // err
	SSDK_WARNING, // warn
	SSDK_NOTICE, // info
	SSDK_DEBUG, // debug
};

#define CHECK_CSI_ID_CH(id, ch)                                                \
	do {                                                                   \
		if ((id >= MAX_CSI_INST) || (ch >= MAX_CSI_CHANNEL)) {         \
			cam_err("%s: error id %d\n", __func__, id);            \
			return -1;                                             \
		}                                                              \
	} while (0)

struct camera_device;

struct camera_buffer {
	struct list_node list;
	uint32_t buffer_idx;
	uint32_t frm_cnt;
	uint64_t timestamp;
	dma_addr_t paddr[3];
};

struct cam_queue {
	int qlen;
	struct list_node qhead;
};
static struct camera_buffer s_buffers[MAX_CSI_INST][MAX_CSI_CHANNEL]
				     [MAX_BUF_CNT];

struct camera_channel {
	struct camera_device *host;
	uint32_t ch_id;
	uint32_t enable;
	uint32_t status;
	uint32_t frm_cnt;
	uint32_t update_cnt;

	struct camera_buffer *bufs;
	struct list_node empty;

	struct list_node in_bufq;
	struct list_node out_bufq;
	struct cam_queue active_q;
	struct camera_buffer *next_buf;
	completion_t completion;
	spinlock_t bufq_lock;

	struct mem_range cam_mem[3];

	struct csi_ch_params cfg;

	uint64_t last_timestamp;
	uint64_t frame_interval;
	uint32_t frame_loss_cnt;
};

struct camera_device {
	uint32_t host_id; /* host ip id */
	uint32_t inited;
	struct csi_device ex_dev;

	struct csi_hw_dev *csi_hw;
	uint32_t csi_irq;
	reg_addr_t csi_base;
	uint32_t bt; /* bus type */
	uint32_t sync;
	struct camera_channel cam_ch[MAX_CSI_CHANNEL];
};

struct csi_channel_ops ch_ops;
static struct camera_device s_cam_devices[MAX_CSI_INST];

static uint32_t get_irq(uint32_t id)
{
	if (id == CSI_HOST_0)
		return CSI1_INTERRUPT0_NUM;
	else if (id == CSI_HOST_1)
		return CSI2_INTERRUPT0_NUM;
	else if (id == CSI_HOST_2)
		return CSI3_INTERRUPT0_NUM;
	else
		return CSI4_INTERRUPT0_NUM;
}

static reg_addr_t get_reg_base(uint32_t id)
{
	if (id == CSI_HOST_0)
		return APB_CSI1_BASE;
	else if (id == CSI_HOST_1)
		return APB_CSI2_BASE;
	else if (id == CSI_HOST_2)
		return APB_CSI3_BASE;
	else
		return APB_CSI4_BASE;
}

static int camera_int_handler(uint32_t irq, void *data)
{
	struct camera_device *cam_dev = (struct camera_device *)data;
	cam_debug("camera_int_handler:  enter irq: %d\n", irq);
	cam_dev->csi_hw->ops.csi_irq_handle(cam_dev->csi_hw);
	return 0;
}

static uint32_t to_vdev_pixfmt(uint32_t csi_pix_fmt)
{
	switch (csi_pix_fmt) {
	case CSI_FMT_YUYV:
		return VDEV_PIX_FMT_YUYV;

	case CSI_FMT_UYVY:
		return VDEV_PIX_FMT_UYVY;

	case CSI_FMT_NV16:
		return VDEV_PIX_FMT_NV16;

	case CSI_FMT_NV61:
		return VDEV_PIX_FMT_NV61;

	case CSI_FMT_YUV422P:
		return VDEV_PIX_FMT_YUV422P;

	case CSI_FMT_RGB24:
		return VDEV_PIX_FMT_RGB24;

	case CSI_FMT_BGR24:
		return VDEV_PIX_FMT_BGR24;

	case CSI_FMT_RGB16:
		return VDEV_PIX_FMT_RGB16;

	default:
		return VDEV_PIX_FMT_UYVY;
	}

	return VDEV_PIX_FMT_UYVY;
}

static uint32_t to_csi_pixfmt(uint32_t vdev_pix_fmt)
{
	switch (vdev_pix_fmt) {
	case VDEV_PIX_FMT_YUYV:
		return CSI_FMT_YUYV;

	case VDEV_PIX_FMT_UYVY:
		return CSI_FMT_UYVY;

	case VDEV_PIX_FMT_NV16:
		return CSI_FMT_NV16;

	case VDEV_PIX_FMT_NV61:
		return CSI_FMT_NV61;

	case VDEV_PIX_FMT_YUV422P:
		return CSI_FMT_YUV422P;

	case VDEV_PIX_FMT_RGB24:
		return CSI_FMT_RGB24;

	case VDEV_PIX_FMT_BGR24:
		return CSI_FMT_BGR24;

	case VDEV_PIX_FMT_RGB16:
		return CSI_FMT_RGB16;

	default:
		return CSI_FMT_UYVY;
	}

	return CSI_FMT_UYVY;
}

static int camera_frame_done(void *caller, uint32_t ch)
{
	struct camera_channel *pch = (struct camera_channel *)caller;
	struct camera_device *cam_dev = pch->host;
	struct list_node *node;
	struct camera_buffer *pbuf;
	unsigned long flags = 0;

#ifdef TEST_CAM_FPS
	static uint64_t time_last, time_cur;
	time_cur = current_time_hires();
	if (pch->frm_cnt == 0)
		time_last = time_cur;
	if ((pch->frm_cnt & 0x1f) == 0x1f) {
		uint64_t fps_1_us = 32000000000000ul / (time_cur - time_last);
		cam_info("frm_cnt %d, cam_fps: %llu.%06llu\n", pch->frm_cnt,
				fps_1_us / 1000000, fps_1_us % 1000000);
		time_last = time_cur;
	}
#endif

	if (pch->status == STOPPED) {
		pch->frm_cnt++;
		return 0;
	}


	if (pch->active_q.qlen == 0) {
		if (pch->status == RUNNING) {
			cam_info("csi%d img%d, active_q empty\n",
				 cam_dev->host_id, ch);
		}
		pch->frm_cnt++;
		return 0;
	}

	if (pch->active_q.qlen > MAX_QLEN)
		cam_info("csi%d img%d, active_q full\n", cam_dev->host_id, ch);

	spin_lock_irqsave(&pch->bufq_lock, flags);

	node = list_peek_head(&pch->active_q.qhead);
	list_delete(node);
	pch->active_q.qlen--;

	if (pch->frm_cnt < SKIP_FRAME_NUM) {
		list_add_tail(&pch->in_bufq, node);
		spin_unlock_irqrestore(&pch->bufq_lock, flags);
		pch->frm_cnt++;
		return 0;
	}

	pbuf = containerof(node, struct camera_buffer, list);
	pbuf->timestamp = current_time_hires();
	pch->frame_interval = pbuf->timestamp - pch->last_timestamp;
	pch->last_timestamp = pbuf->timestamp;
	pbuf->frm_cnt = pch->frm_cnt;
	list_add_tail(&pch->out_bufq, &pbuf->list);
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	completion_done(&pch->completion);

	cam_debug("%s, csi%d img%d, frm_cnt %d, buf_idx %d\n", __func__,
		  cam_dev->host_id, ch, pch->frm_cnt, pbuf->buffer_idx);

	pch->frm_cnt++;
	return 0;
}

static int camera_update_buf(void *caller, uint32_t ch)
{
	struct camera_channel *pch = (struct camera_channel *)caller;
	struct camera_device *cam_dev = pch->host;
	struct list_node *node;
	struct camera_buffer *pbuf;
	struct csi_img_buf buf;
	unsigned long flags = 0;

	pch->update_cnt++;
	if (pch->status == STOPPED)
		return 0;

	spin_lock_irqsave(&pch->bufq_lock, flags);
	node = list_peek_head(&pch->in_bufq);
	if (node == NULL) {
		spin_unlock_irqrestore(&pch->bufq_lock, flags);
		if (pch->status == RUNNING) {
			pch->status = IDLE;
			pch->frame_loss_cnt++;
			cam_debug("ch %d vbuf_list is empty\n", ch);
		}
		return 0;
	}
	pch->status = RUNNING;

	if (pch->active_q.qlen >= MAX_QLEN) {
		spin_unlock_irqrestore(&pch->bufq_lock, flags);
		cam_err("csi%d img%d, active_q overflow\n", cam_dev->host_id,
			ch);
		return 0;
	}

	if (pch->next_buf) {
		pbuf = pch->next_buf;
		list_add_tail(&pch->active_q.qhead, &pbuf->list);
		pch->active_q.qlen++;
		pch->next_buf = NULL;
	}

	list_delete(node);
	pbuf = containerof(node, struct camera_buffer, list);
	pch->next_buf = pbuf;
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	buf.paddr[0] = pbuf->paddr[0];
	buf.paddr[1] = pbuf->paddr[1];
	buf.paddr[2] = pbuf->paddr[2];
	cam_dev->csi_hw->ops.csi_cfg_img_buf(cam_dev->csi_hw, ch, &buf);

	cam_debug("%s, csi%d img%d, done\n", __func__, cam_dev->host_id, ch);
	return 0;
}

static int camera_error_handle(void *caller, uint32_t ch)
{
	struct camera_channel *pch = (struct camera_channel *)caller;
	struct camera_device *cam_dev = pch->host;

	cam_err("%s, csi%d img%d, error\n", __func__, cam_dev->host_id, ch);
	return 0;
}

struct csi_device *camera_dev_init(uint32_t id)
{
	int i;
	struct camera_device *cam_dev;

	if (id >= MAX_CSI_INST) {
		cam_err("%s: error id %d\n", __func__, id);
		return NULL;
	}
	cam_dev = &s_cam_devices[id];
	if (cam_dev->inited)
		goto exit;

	cam_dev->host_id = id;
	cam_dev->csi_irq = get_irq(id);
	cam_dev->csi_base = (reg_addr_t)get_reg_base(id);
	cam_dev->bt = 1;
	cam_dev->sync = 0;
	cam_err("csi%d init: irq %d,  reg_base 0x%08x\n", id, cam_dev->csi_irq,
		(uint32_t)cam_dev->csi_base);

	cam_dev->csi_hw = (void *)csi_hw_init(cam_dev->csi_base, id);
	if (cam_dev->csi_hw == NULL) {
		cam_err("%s: faile to init csi_hw\n", __func__);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CSI_MOD,ERROR_CSI_INIT_FAIL,0);
		return NULL;
	}

	cam_info("enalbe irq: %d\n", cam_dev->csi_irq);
	for (i = 0; i < 4; i++) {
		register_int_handler(cam_dev->csi_irq + i, camera_int_handler, cam_dev);
		unmask_interrupt(cam_dev->csi_irq + i);
	}

	for (i = 0; i < MAX_CSI_CHANNEL; i++) {
		cam_dev->cam_ch[i].ch_id = i;
		cam_dev->cam_ch[i].host = cam_dev;
		spin_lock_init(&cam_dev->cam_ch[i].bufq_lock);
	}

	cam_dev->ex_dev.ops = ch_ops;

exit:
	cam_info("%s: done\n", __func__);
	return &cam_dev->ex_dev;
}

int camera_dev_sync(uint32_t id, uint32_t sync)
{
	struct camera_device *cam_dev;

	if (id >= MAX_CSI_INST) {
		cam_err("%s: error id %d\n", __func__, id);
		return -1;
	}
	cam_dev = &s_cam_devices[id];
	cam_dev->sync = sync;
	cam_dev->csi_hw->ops.csi_cfg_sync(cam_dev->csi_hw, cam_dev->sync);
	cam_info("%s: csi%d sync %d\n", __func__, id, sync);

	return 0;
}

static int channel_open(uint32_t id, uint32_t ch)
{
	int i;
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct camera_buffer *pbuf;
	struct csi_callback_t cb;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	cam_dev->csi_hw->ops.csi_reset_img(
		cam_dev->csi_hw, ch, CSI_RESET_IMG_CTX | CSI_RESET_IMG_HW);

	list_initialize(&pch->empty);
	pch->bufs = &s_buffers[id][ch][0];
	memset(pch->bufs, 0, sizeof(struct camera_buffer) * MAX_BUF_CNT);
	for (i = 0, pbuf = pch->bufs; i < MAX_BUF_CNT; i++, pbuf++) {
		list_add_tail(&pch->empty, &pbuf->list);
	}

	list_initialize(&pch->out_bufq);
	list_initialize(&pch->in_bufq);
	list_initialize(&pch->active_q.qhead);
	pch->active_q.qlen = 0;
	pch->next_buf = NULL;
	completion_init(&pch->completion);

	pch->cfg.bus_type = CSI_BUS_PARALLEL2;
	pch->cfg.bus_fmt = CSI_MBUS_UYVY8_2X8;
	pch->cfg.width = 640;
	pch->cfg.height = 480;
	pch->cfg.crop_en = 0;
	pch->cfg.set_pix_fmt = CSI_FMT_UYVY;
	pch->cfg.pix_fmt_num = 1;
	pch->cfg.pix_fmt[0] = CSI_FMT_UYVY;

	memset(&cb, 0, sizeof(struct csi_callback_t));
	cb.caller = (void *)pch;
	cb.csi_frame_done = camera_frame_done;
	cb.csi_update_buf = camera_update_buf;
	cb.csi_error_handle = camera_error_handle;
	cam_info("csi_register_callback: caller %p, done %p\n", cb.caller,
		 cb.csi_frame_done);

	cam_dev->csi_hw->ops.csi_register_callback(cam_dev->csi_hw, ch, &cb);

	pch->enable = 1;
	pch->status = STOPPED;
	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_close(uint32_t id, uint32_t ch)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];
	pch->enable = 0;
	pch->status = STOPPED;
	completion_destroy(&pch->completion);

	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_cfg_bus_type(uint32_t id, uint32_t ch,
				struct csi_ch_params *param)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	switch (param->bus_type) {
	case VDEV_MBUS_CSI2:
		pch->cfg.bus_type = CSI_BUS_MIPICSI2;
		break;
	case VDEV_MBUS_PARALLEL:
	case VDEV_MBUS_CSI1:
		pch->cfg.bus_type = CSI_BUS_PARALLEL4;
		break;
	case VDEV_MBUS_PARALLEL2:
		pch->cfg.bus_type = CSI_BUS_PARALLEL2;
		break;
	case VDEV_MBUS_BT656:
		pch->cfg.bus_type = CSI_BUS_BT656;
		break;
	default:
		cam_err("%s: error vdev bus type %d\n", __func__,
			param->bus_type);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CSI_MOD,ERROR_CSI_UNSUPPORTED_BUS_TYPE,param->bus_type);
		break;
	}

	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_cfg_bus_fmt(uint32_t id, uint32_t ch,
			       struct csi_ch_params *param)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	switch (param->bus_fmt) {
	case VDEV_MBUS_FMT_UYVY:
		pch->cfg.bus_fmt = CSI_MBUS_UYVY8_2X8;
		break;

	case VDEV_MBUS_FMT_YUYV:
		pch->cfg.bus_fmt = CSI_MBUS_YUYV8_2X8;
		break;

	case VDEV_MBUS_FMT_RGB888:
		pch->cfg.bus_fmt = CSI_MBUS_RGB888_1X24;
		break;

	case VDEV_MBUS_FMT_RGB565:
		pch->cfg.bus_fmt = CSI_MBUS_RGB565_1X16;
		break;

	default:
		cam_err("%s: error vdev bus fmt %d\n", __func__,
			param->bus_fmt);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CSI_MOD,ERROR_CSI_UNSUPPORTED_OUTFMT,param->bus_fmt);
		break;
	}

	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_enum_pixfmt(uint32_t id, uint32_t ch,
			       struct csi_ch_params *param)
{
	int ret;
	uint32_t i;
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct csi_bus_info bus_info;
	struct csi_outfmt_info res;

	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	bus_info.bus_type = pch->cfg.bus_type;
	bus_info.mbus_fmt = pch->cfg.bus_fmt;
	bus_info.bus_flag = 0;

	ret = cam_dev->csi_hw->ops.csi_cfg_bus(cam_dev->csi_hw, ch, &bus_info);
	ret |= cam_dev->csi_hw->ops.csi_query_outfmts(cam_dev->csi_hw, ch,
						      &res);

	if (ret == 0 && res.count > 0) {
		param->pix_fmt_num = res.count;
		for (i = 0; i < res.count; i++) {
			param->pix_fmt[i] = to_vdev_pixfmt(res.fmts[i]);
			if (i == 0)
				pch->cfg.set_pix_fmt = res.fmts[i];
		}
	} else {
		param->pix_fmt_num = 1;
		param->pix_fmt[0] = VDEV_PIX_FMT_UYVY;
	}

	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_cfg_pixfmt(uint32_t id, uint32_t ch,
			      struct csi_ch_params *param)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];
	pch->cfg.set_pix_fmt = to_csi_pixfmt(param->set_pix_fmt);

	cam_info("%s: ch %d done,\n", __func__, ch);
	return 0;
}

static int channel_cfg_size(uint32_t id, uint32_t ch,
			    struct csi_ch_params *param)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];
	if (param->field_type == VDEV_FIELD_INTERLACED) {
		pch->cfg.field_type = CSI_FIELD_INTERLACED;
		pch->cfg.width_even = param->width_even;
		pch->cfg.height_even = param->height_even;
	} else {
		pch->cfg.field_type = CSI_FIELD_NONE;
	}
	pch->cfg.width = param->width;
	pch->cfg.height = param->height;

	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_cfg_crop(uint32_t id, uint32_t ch,
			    struct csi_ch_params *param)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct csi_img_crop crop[2];
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	pch->cfg.crop_en = 1;
	pch->cfg.crop_x = param->crop_x;
	pch->cfg.crop_y = param->crop_y;
	pch->cfg.crop_w = param->crop_w;
	pch->cfg.crop_h = param->crop_h;

	if (pch->status == RUNNING) {
		memset(&crop[0], 0, sizeof(crop));
		crop[0].x = pch->cfg.crop_x;
		crop[0].y = pch->cfg.crop_y;
		crop[0].w = pch->cfg.crop_w;
		crop[0].h = pch->cfg.crop_h;
		cam_dev->csi_hw->ops.csi_cfg_crop(cam_dev->csi_hw, ch,
						  &crop[0]);
	}
	cam_info("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_stream(uint32_t id, uint32_t ch, int on)
{
	int ret = 0;
	uint32_t outfmt = CSI_FMT_YUYV;
	struct csi_bus_info bus_info;
	struct csi_field_info field;
	struct csi_img_size size[2];
	struct csi_img_crop crop[2];
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct list_node *node;
	unsigned long flags = 0;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	if ((on && (pch->status != STOPPED)) ||
	    (!on && (pch->status == STOPPED))) {
		cam_err("%s: csi %d img %d already stream %s\n", __func__, id,
			ch, (on ? "on" : "off"));
		return 0;
	}

	if (on == 0) {
		goto stream_off;
	}

	pch->status = IDLE;
	bus_info.bus_type = pch->cfg.bus_type;
	bus_info.mbus_fmt = pch->cfg.bus_fmt;
	bus_info.bus_flag = 0;
	ret = cam_dev->csi_hw->ops.csi_cfg_bus(cam_dev->csi_hw, ch, &bus_info);

	outfmt = pch->cfg.set_pix_fmt;
	ret |= cam_dev->csi_hw->ops.csi_cfg_outfmt(cam_dev->csi_hw, ch, outfmt);

	field.field_type = pch->cfg.field_type;
	ret |= cam_dev->csi_hw->ops.csi_cfg_field(cam_dev->csi_hw, ch, &field);

	memset(&size[0], 0, sizeof(size));
	size[0].w = pch->cfg.width;
	size[0].h = pch->cfg.height;
	size[1].w = pch->cfg.width_even;
	size[1].h = pch->cfg.height_even;
	ret |= cam_dev->csi_hw->ops.csi_cfg_size(cam_dev->csi_hw, ch, &size[0]);

	if (pch->cfg.crop_en == 1) {
		memset(&crop[0], 0, sizeof(crop));
		crop[0].x = pch->cfg.crop_x;
		crop[0].y = pch->cfg.crop_y;
		crop[0].w = pch->cfg.crop_w;
		crop[0].h = pch->cfg.crop_h;
		ret |= cam_dev->csi_hw->ops.csi_cfg_crop(cam_dev->csi_hw, ch,
							 &crop[0]);
	}

	ret |= camera_update_buf(pch, ch);

	ret |= cam_dev->csi_hw->ops.csi_stream_on(cam_dev->csi_hw, ch);
	if (ret) {
		cam_info("%s: ch %d error\n", __func__, ch);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CSI_MOD,ERROR_CSI_CFG_FAIL,ch);
		return -1;
	}

	pch->frm_cnt = 0;
	pch->update_cnt = 0;
	pch->status = RUNNING;
	pch->frame_loss_cnt = 0;

	cam_info("%s: ch %d stream on done\n", __func__, ch);
	return 0;

stream_off:
	pch->status = STOPPED;
	completion_done(&pch->completion);

	cam_dev->csi_hw->ops.csi_stream_off(cam_dev->csi_hw, ch);

	/* clear all buffer queue here */
	spin_lock_irqsave(&pch->bufq_lock, flags);
	do {
		node = list_peek_head(&pch->out_bufq);
		if (node == NULL)
			break;
		list_delete(node);
		list_add_tail(&pch->empty, node);
	} while (1);

	do {
		node = list_peek_head(&pch->in_bufq);
		if (node == NULL)
			break;
		list_delete(node);
		list_add_tail(&pch->empty, node);
	} while (1);

	do {
		node = list_peek_head(&pch->active_q.qhead);
		if (node == NULL)
			break;
		list_delete(node);
		list_add_tail(&pch->empty, node);
	} while (1);
	pch->active_q.qlen = 0;

	if (pch->next_buf) {
		list_add_tail(&pch->empty, &pch->next_buf->list);
		pch->next_buf = NULL;
	}
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	cam_info("%s: ch %d stream off done\n", __func__, ch);
	return 0;
}

static int channel_qbuf(uint32_t id, uint32_t ch, void *buf)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct camera_buffer *pbuf;
	struct list_node *node;
	struct vdev_buffer *in_buf = (struct vdev_buffer *)buf;
	unsigned long flags = 0;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	spin_lock_irqsave(&pch->bufq_lock, flags);
	node = list_peek_head(&pch->empty);
	if (node == NULL) {
		spin_unlock_irqrestore(&pch->bufq_lock, flags);
		cam_err("%s: ch %d failed\n", __func__, ch);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CSI_MOD,ERROR_CSI_INVALID_ID,ch);
		return -1;
	}
	list_delete(node);
	pbuf = containerof(node, struct camera_buffer, list);
	pbuf->buffer_idx = in_buf->buffer_idx;
	pbuf->paddr[0] = (dma_addr_t)in_buf->paddr[0];
	pbuf->paddr[1] = (dma_addr_t)in_buf->paddr[1];
	pbuf->paddr[2] = (dma_addr_t)in_buf->paddr[2];
	list_add_tail(&pch->in_bufq, &pbuf->list);
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	return 0;
}

static int channel_dqbuf(uint32_t id, uint32_t ch, void *buf)
{
	struct camera_device *cam_dev;
	struct camera_channel *pch;
	struct camera_buffer *pbuf;
	struct list_node *node;
	struct vdev_buffer *out_buf = (struct vdev_buffer *)buf;
	uint32_t timeout_ms;
	unsigned long flags = 0;
	CHECK_CSI_ID_CH(id, ch);

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];

	spin_lock_irqsave(&pch->bufq_lock, flags);
	node = list_peek_head(&pch->out_bufq);
	if (node != NULL) {
		cam_debug("%s: ch %d get node\n", __func__, ch);
		goto get_node;
	}
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	timeout_ms =
		(out_buf->timeout_ms > 0) ? out_buf->timeout_ms : (10 * 1000);
	cam_debug("%s: ch %d wait timeout %d ms\n", __func__, ch, timeout_ms);
	if (wait_completion_timeout(&pch->completion, timeout_ms) < 0) {
		cam_err("%s: ch %d failed for timeout %d ms\n", __func__, ch,
			timeout_ms);
		return -1;
	}

	spin_lock_irqsave(&pch->bufq_lock, flags);
	node = list_peek_head(&pch->out_bufq);
	if (node == NULL) {
		spin_unlock_irqrestore(&pch->bufq_lock, flags);
		cam_debug("%s: ch %d node null\n", __func__, ch);
		return -1;
	}

get_node:
	list_delete(node);
	pbuf = containerof(node, struct camera_buffer, list);
	out_buf->buffer_idx = pbuf->buffer_idx;
	out_buf->frm_cnt = pbuf->frm_cnt;
	out_buf->timestamp = pbuf->timestamp;
	out_buf->timestamp_h = (pbuf->timestamp >> 32) & 0xFFFFFFFF;
	out_buf->paddr[0] = (void *)pbuf->paddr[0];
	out_buf->paddr[1] = (void *)pbuf->paddr[1];
	out_buf->paddr[2] = (void *)pbuf->paddr[2];

	memset(pbuf, 0, sizeof(struct camera_buffer));
	list_add_tail(&pch->empty, &pbuf->list);
	spin_unlock_irqrestore(&pch->bufq_lock, flags);

	cam_debug("%s: ch %d done\n", __func__, ch);
	return 0;
}

static int channel_debug(uint32_t id, uint32_t ch)
{
	struct camera_channel *pch;
	struct camera_device *cam_dev;
	int inq_cnt, outq_cnt;
	unsigned long flags = 0;

	cam_dev = &s_cam_devices[id];
	pch = &cam_dev->cam_ch[ch];
	cam_info("%7s%7s%5s%3s%6s%7s%7s%8s%9s\n", "host_id", "inited", "sync",
		 "bt", "ch_id", "enable", "status", "frm_cnt", "interval");

	cam_info("%7d", cam_dev->host_id);
	cam_info("%7d", cam_dev->inited);
	cam_info("%5d", cam_dev->sync);

	cam_info("%3d", cam_dev->bt);
	cam_info("%6d", pch->ch_id);
	cam_info("%7d", pch->enable);

	cam_info("%7d", pch->status);
	cam_info("%8d", pch->frm_cnt);

	cam_info("%5lld.%03lld\n", pch->frame_interval / 1000,
		 pch->frame_interval % 1000);

	cam_info("%8s%6s%7s%8s%9s\n", "loss_cnt", "width", "height", "inq_cnt",
		 "outq_cnt");

	cam_info("%8d", pch->frame_loss_cnt);

	cam_info("%6d", pch->cfg.width);
	cam_info("%7d", pch->cfg.height);

	spin_lock_irqsave(&pch->bufq_lock, flags);
	inq_cnt = list_length(&pch->in_bufq);
	outq_cnt = list_length(&pch->out_bufq);
	spin_unlock_irqrestore(&pch->bufq_lock, flags);
	cam_info("%8d", inq_cnt);
	cam_info("%9d\n", outq_cnt);

	return 0;
}

struct csi_channel_ops ch_ops = {
	.channel_open = channel_open,
	.channel_close = channel_close,
	.channel_stream = channel_stream,
	.channel_cfg_bus_type = channel_cfg_bus_type,
	.channel_cfg_bus_fmt = channel_cfg_bus_fmt,
	.channel_cfg_size = channel_cfg_size,
	.channel_cfg_crop = channel_cfg_crop,
	.channel_enum_pixfmt = channel_enum_pixfmt,
	.channel_cfg_pixfmt = channel_cfg_pixfmt,

	.channel_qbuf = channel_qbuf,
	.channel_dqbuf = channel_dqbuf,
	.channel_show_info = channel_debug,
};
