/*********************
 *      INCLUDES
 *********************/
#include "lv_rear_guide_line.h"
#include <math.h>
#include "lv_line.h"

/*********************
 *      DEFINES
 *********************/
#define DYNAMIC_GUIDE_LINE 8
#define STATIC_GUIDE_LINE 3
#define POINT_NUM 200
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define abs(x) ((x)>0 ? (x) : -(x))

void initStyle(void);
void init_rear_guide_line(lv_obj_t * obj);
void init_left_carline(void);
lv_point_t get_ellipse_point(int angle);
lv_point_t get_v_line_left_point(lv_point_t p,int angle);
lv_point_t get_v_line_right_point(lv_point_t p,int angle);
lv_point_t* get_point_list_line(lv_point_t* list_a,lv_point_t* list_b,int index,lv_point_t line[]);
lv_point_t* get_point_list(lv_point_t a, lv_point_t b,lv_point_t* list,int d,int v);
lv_point_t* get_left_short_line(lv_point_t *point_line,lv_point_t *target,float percentage,int angle);
lv_point_t* get_right_short_line(lv_point_t *point_line,lv_point_t *target,float percentage,int angle);

static lv_point_t a = {600,720};
static lv_point_t b = {1320,720};
static lv_point_t c = {0,0};
static lv_point_t d = {0,0};

static lv_point_t left_1[8];
static lv_point_t left_2[5];
static lv_point_t left_3[5];

static lv_point_t right_1[8];
static lv_point_t right_2[5];
static lv_point_t right_3[5];

float list_weight[5]={1,6.0/21,11.0/21,18.0/21,20.0/21};
static int eclipse_a;
static int eclipse_b;
#if 1
static lv_point_t top_line_points[2];
static bool flag = false;
static int line_angle = 0;
static lv_point_t ellipse_point;
static lv_point_t left_line_points[POINT_NUM];
static lv_point_t right_line_points[POINT_NUM];
static lv_point_t line_points1[2];
static lv_point_t line_points2[2];
static lv_point_t line_points3[2];
static lv_point_t line_points4[2];
static lv_point_t line_points1_right[2];
static lv_point_t line_points2_right[2];
static lv_point_t line_points3_right[2];
static lv_point_t line_points4_right[2];
#endif
static lv_style_t default_style_line;
static lv_obj_t * top_dynamic_line;

static lv_obj_t * static_guide_line_left[3];
static lv_obj_t * static_guide_line_right[3];

static int rear_mode;

static lv_style_t style_greenline;
static lv_style_t style_redline;
static lv_style_t style_yellowline;
static lv_obj_t * dynamic_guide_line_hor[8];
static lv_obj_t * dynamic_guide_line_left_normal;
static lv_obj_t * dynamic_guide_line_right_normal;
static lv_obj_t * dynamic_guide_line_left[3];//left guide green-yellow-red color line
static lv_obj_t * dynamic_guide_line_right[3];//right guide green-yellow-red color line
static lv_design_cb_t ancestor_design;
static lv_signal_cb_t ancestor_signal;
lv_obj_t * line;
lv_style_t style_line;
static float scale = 0;

static void lv_rgl_draw_line(lv_obj_t * rgl, const lv_area_t * clip_area)
{
#if 1
    if(!flag){
        init_rear_guide_line(rgl);
        flag = true;
    }

    ellipse_point = get_ellipse_point(line_angle);
    top_line_points[0] = get_v_line_left_point(ellipse_point,line_angle);
    top_line_points[1] = get_v_line_right_point(ellipse_point,line_angle);
    if(rear_mode == DYNAMIC_LINE){
        if(line_angle<0){
            lv_point_t * line_point_4 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[4],line_points1);
            lv_point_t * line_point_5 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[3],line_points2);
            lv_point_t * line_point_6 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[2],line_points3);
            lv_point_t * line_point_7 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[1],line_points4);

            lv_point_t * line_point_right_4 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[4],line_points1_right);
            lv_point_t * line_point_right_5 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[3],line_points2_right);
            lv_point_t * line_point_right_6 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[2],line_points3_right);
            lv_point_t * line_point_right_7 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM*list_weight[1],line_points4_right);

            lv_point_t * temp = get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle);
            lv_line_set_points(dynamic_guide_line_left[0], temp, (int)(POINT_NUM*1.0*2.0/9));
            lv_line_set_points(dynamic_guide_line_left[1], &temp[(int)(POINT_NUM*1.0*2.0/9)], (int)(POINT_NUM*1.0*3.0/9));
            lv_line_set_points(dynamic_guide_line_left[2], &temp[(int)(POINT_NUM*1.0*5.0/9)], (int)(POINT_NUM*1.0*4.0/9));
            lv_point_t * temp_right = get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle);
            lv_line_set_points(dynamic_guide_line_right[0], temp_right, (int)(POINT_NUM*1.0*4.0/9));
            lv_line_set_points(dynamic_guide_line_right[1], &temp_right[(int)(POINT_NUM*1.0*4.0/9)], (int)(POINT_NUM*1.0*3.0/9));
            lv_line_set_points(dynamic_guide_line_right[2], &temp_right[(int)(POINT_NUM*1.0*7.0/9)], (int)(POINT_NUM*1.0*2.0/9));

            lv_line_set_points(dynamic_guide_line_hor[0],get_left_short_line(line_point_4,line_points1,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[1],get_left_short_line(line_point_5,line_points2,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[2],get_left_short_line(line_point_6,line_points3,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[3],get_left_short_line(line_point_7,line_points4,0.23,line_angle),2);

            lv_line_set_points(dynamic_guide_line_hor[4],get_right_short_line(line_point_right_4,line_points1_right,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[5],get_right_short_line(line_point_right_5,line_points2_right,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[6],get_right_short_line(line_point_right_6,line_points3_right,0.23,line_angle),2);
            lv_line_set_points(dynamic_guide_line_hor[7],get_right_short_line(line_point_right_7,line_points4_right,0.23,line_angle),2);

        }
        else{
            lv_point_t * line_point_4 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[4],line_points1);
            lv_point_t * line_point_5 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[3],line_points2);
            lv_point_t * line_point_6 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[2],line_points3);
            lv_point_t * line_point_7 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[1],line_points4);

            lv_point_t * line_point_right_4 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[4],line_points1_right);
            lv_point_t * line_point_right_5 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[3],line_points2_right);
            lv_point_t * line_point_right_7 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[2],line_points3_right);
            lv_point_t * line_point_right_8 = get_point_list_line(get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle), get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM*list_weight[1],line_points4_right);
            lv_point_t * temp = get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle);
            lv_line_set_points(dynamic_guide_line_left[0], temp, (int)POINT_NUM*1.0*2.0/9);
            lv_line_set_points(dynamic_guide_line_left[1], &temp[(int)(POINT_NUM*1.0*2.0/9)], (int)(POINT_NUM*1.0*3.0/9));
            lv_line_set_points(dynamic_guide_line_left[2], &temp[(int)(POINT_NUM*1.0*5.0/9)], (int)(POINT_NUM*1.0*4.0/9));
            lv_point_t * temp_right = get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle);
            lv_line_set_points(dynamic_guide_line_right[0], temp_right, (int)(POINT_NUM*1.0*4.0/9));
            lv_line_set_points(dynamic_guide_line_right[1], &temp_right[(int)(POINT_NUM*1.0*4.0/9)], (int)(POINT_NUM*1.0*3.0/9));
            lv_line_set_points(dynamic_guide_line_right[2], &temp_right[(int)(POINT_NUM*1.0*7.0/9)], (int)(POINT_NUM*1.0*2.0/9));

            lv_line_set_points(dynamic_guide_line_hor[0],get_left_short_line(line_point_4,line_points1,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[1],get_left_short_line(line_point_5,line_points2,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[2],get_left_short_line(line_point_6,line_points3,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[3],get_left_short_line(line_point_7,line_points4,0.23,line_angle*scale),2);

            lv_line_set_points(dynamic_guide_line_hor[4],get_right_short_line(line_point_right_4,line_points1_right,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[5],get_right_short_line(line_point_right_5,line_points2_right,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[6],get_right_short_line(line_point_right_7,line_points3_right,0.23,line_angle*scale),2);
            lv_line_set_points(dynamic_guide_line_hor[7],get_right_short_line(line_point_right_8,line_points4_right,0.23,line_angle*scale),2);

        }
    }else{
        if(line_angle<0){
            lv_line_set_points(dynamic_guide_line_left_normal, get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*2*scale),line_angle),POINT_NUM);
            lv_line_set_points(dynamic_guide_line_right_normal, get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM);
        }
        else{
            lv_line_set_points(dynamic_guide_line_left_normal, get_point_list(get_v_line_left_point(ellipse_point,line_angle),a,left_line_points,abs(line_angle*scale)*4,line_angle),POINT_NUM);
            lv_line_set_points(dynamic_guide_line_right_normal, get_point_list(b,get_v_line_right_point(ellipse_point,line_angle),right_line_points,abs(line_angle*2*scale),line_angle), POINT_NUM);
        }
       
        lv_line_set_points(top_dynamic_line, top_line_points, 2);
    }
#endif

}

static lv_design_res_t lv_rgl_design(lv_obj_t * rear_guide_line, const lv_area_t * clip_area, lv_design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == LV_DESIGN_COVER_CHK) {
        return LV_DESIGN_RES_NOT_COVER;
    }
    /*Draw the object*/
    else if(mode == LV_DESIGN_DRAW_MAIN) {
        ancestor_design(rear_guide_line, clip_area, mode);
        lv_rgl_draw_line(rear_guide_line, clip_area);
    }
    /*Post draw when the children are drawn*/
    else if(mode == LV_DESIGN_DRAW_POST) {
        ancestor_design(rear_guide_line, clip_area, mode);
    }
    return LV_DESIGN_RES_OK;
}

/**
 * Signal function of the rgl
 * @param rgl pointer to a rgl object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_rgl_signal(lv_obj_t * rgl, lv_signal_t sign, void * param)
{
    lv_res_t res;
    res = ancestor_signal(rgl, sign, param);
    if(res != LV_RES_OK) return res;
    return res;
}

/**
 * Create a guide line object
 * @param par pointer to an object, it will be the parent of the new guide line
 * mode -> rear_guide_mode(STATIC_LINE/DYNAMIC_LINE)
 * @return pointer to the created guide line
 */
lv_obj_t * lv_rear_guide_line_create(lv_obj_t * parent,lv_obj_t * copy,int mode)
{
    LV_LOG_INFO("begin")
    
    lv_obj_t * rear_guide_line = lv_obj_create(parent, NULL);
    lv_rear_guide_t * ext = lv_obj_allocate_ext_attr(rear_guide_line, sizeof(lv_rear_guide_t));
    LV_ASSERT_MEM(ext);
    if(ext == NULL) {
        lv_obj_del(rear_guide_line);
        return NULL;
    }
    if(copy == NULL) {
        lv_obj_set_size(rear_guide_line, parent->coords.x2 - parent->coords.x1, parent->coords.y2 - parent->coords.y1);
        lv_obj_clean_style_list(rear_guide_line, 0);
    }
    // if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(rear_guide_line);
    if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(rear_guide_line);
    // lv_obj_set_signal_cb(rear_guide_line, lv_rgl_design);
    lv_obj_set_design_cb(rear_guide_line, lv_rgl_design);
    rear_mode = mode;   
    lv_rgl_set_focus(a,b);
    return rear_guide_line;
}

void lv_rgl_set_focus(lv_point_t a_point,lv_point_t b_point)
{
    a.x = a_point.x;
    a.y = a_point.y;
    b.x = b_point.x;
    b.y = b_point.y;
    int dx = abs(b.x - a.x);
    int dy = b.y;
    
    c = (lv_point_t){(a.x+b.x)/2-dx/16*3,b.y-dx/2*sqrt(3)};
    d = (lv_point_t){(a.x+b.x)/2+dx/16*3,c.y}; 
    scale = dy * 1.0/500; 

    eclipse_a = abs(b.x - a.x);
    eclipse_b = eclipse_a * sqrt(3)/2;
}

//calculate the gradient by points,y = kx +b
float get_line_k(lv_point_t a,lv_point_t b)
{
    if(b.x == a.x)
        return 0;
    return (b.y - a.y)*1.0/(b.x - a.x);
}

//calculate the const by points, y = kx +b
int get_line_b(lv_point_t a,lv_point_t b)
{
    return a.y - a.x * get_line_k(a,b);
}

//Take the line segment of the line ,percentage (0~1)
lv_point_t * get_left_short_line(lv_point_t *point_line,lv_point_t *target,float percentage,int angle)
{
    int dx = abs(point_line[1].x - point_line[0].x);
    int dy = abs(point_line[1].y - point_line[0].y);
    if(angle >0)
        point_line[1] = (lv_point_t){point_line[0].x+dx*percentage,point_line[0].y+dy*percentage};
    else
        point_line[1] = (lv_point_t){point_line[0].x+dx*percentage,point_line[0].y-dy*percentage};
        
    return point_line;
}

//Take the line segment of the line ,percentage (0~1)
lv_point_t * get_right_short_line(lv_point_t *point_line,lv_point_t *target,float percentage,int angle)
{
    int dx = abs(point_line[1].x - point_line[0].x);
    int dy = abs(point_line[1].y - point_line[0].y);
    if(angle >0)
        point_line[0] = (lv_point_t){point_line[1].x-dx*percentage,point_line[1].y-dy*percentage};
    else
        point_line[0] = (lv_point_t){point_line[1].x-dx*percentage,point_line[1].y+dy*percentage};
return point_line;
}

lv_point_t get_p2_line_right_point(lv_point_t p1,lv_point_t p2,int len,int angle)
{
    int helf_x = (p1.x + p2.x)/2;
    int helf_y = (p1.y + p2.y)/2;
    int dx = abs(p2.x - p1.x);
    int dy = abs(p2.y - p1.y);

    if(dy == 0)
        return (lv_point_t){0,0};
    if(((p1.x==a.x||p2.x==a.x) && (p1.x>p2.x?p1.x:p2.x) >= c.x) || ((p1.x==b.x||p2.x==b.x) && ((p1.x<p2.x?p1.x:p2.x) >= d.x)))
        return (lv_point_t){helf_x - (len)*(dy/sqrt(dx*dx+dy*dy)),helf_y - (len)*(dx/sqrt(dx*dx+dy*dy))};
    else
        return (lv_point_t){helf_x + (len)*(dy/sqrt(dx*dx+dy*dy)),helf_y - (len)*(dx/sqrt(dx*dx+dy*dy))};
}

//获取定位线
lv_point_t* get_point_list_line(lv_point_t* list_a,lv_point_t* list_b,int index,lv_point_t line[])
{
    line[0] = list_a[abs(POINT_NUM-index)];
    line[1] = list_b[abs(index)];
    return line;
}

lv_point_t* get_point_list(lv_point_t a, lv_point_t b,lv_point_t* list,int d,int v)
{
    int p2_x = get_p2_line_right_point(a,b,d,v).x;
    int p2_y = get_p2_line_right_point(a,b,d,v).y;
    float t;
    for(int i=0;i<POINT_NUM;i++){
        t = (i+1) * 1.0/POINT_NUM;
        if(i == 0)
            t = 0;

        int new_x = (1-t)*(1-t)*a.x+2*t*(1-t)*p2_x+t*t*b.x;
        int new_y = (1-t)*(1-t)*a.y+2*t*(1-t)*p2_y+t*t*b.y;
        list[i] = (lv_point_t){new_x ,new_y};
    }
    return list;
}

//get c point
lv_point_t get_v_line_left_point(lv_point_t p,int angle)
{
    int helf = abs(d.x - c.x)/2;
    int dx = helf * _lv_trigo_cos(angle) >> LV_TRIGO_SHIFT;
    int dy = helf * _lv_trigo_sin(angle) >> LV_TRIGO_SHIFT;
    return (lv_point_t){p.x - dx,p.y - dy};
}

//get d point
lv_point_t get_v_line_right_point(lv_point_t p,int angle)
{
    int helf = abs(d.x - c.x)/2;
    int dx = helf * _lv_trigo_cos(angle) >> LV_TRIGO_SHIFT;
    int dy = helf * _lv_trigo_sin(angle) >> LV_TRIGO_SHIFT;
    return (lv_point_t){p.x + dx,p.y + dy};
}

//get ellipse point
lv_point_t get_ellipse_point(int angle)
{
    if(angle == 0){
        return  (lv_point_t){abs(b.x - a.x)+a.x-abs(b.x - a.x)/2,a.y-abs(b.x - a.x)/2.0*sqrt(3)};
    }
    
    else if(angle>0)
    {
        int temp = 90-angle;
        float x = (int)sqrt(eclipse_b*eclipse_b)*eclipse_a/sqrt(((double)eclipse_b*eclipse_b+eclipse_a*eclipse_a*_lv_trigo_tan(temp)*_lv_trigo_tan(temp)));
        float y = x * _lv_trigo_tan(temp);
        return (lv_point_t){x+abs(b.x - a.x)+a.x-abs(b.x - a.x)/2,a.y-y};

    }else{
        int temp = 90-(-angle);
        float x = (int)sqrt(eclipse_b*eclipse_b)*eclipse_a/sqrt(((double)eclipse_b*eclipse_b+eclipse_a*eclipse_a*_lv_trigo_tan(temp)*_lv_trigo_tan(temp)));
        float y = x * _lv_trigo_tan(temp);
        return (lv_point_t){abs(b.x - a.x)-x+a.x-abs(b.x - a.x)/2,a.y-y};
    }
}

void set_style(lv_style_t *style,lv_color_t color,int line_width)
{
    lv_style_init(style);
    lv_style_set_line_width(style,LV_STATE_DEFAULT,line_width);
    lv_style_set_line_color(style, LV_STATE_DEFAULT,color);
    lv_style_set_line_rounded(style, LV_STATE_DEFAULT,true);
}

void initStyle()
{
    set_style(&default_style_line,LV_COLOR_ORANGE,6);
    set_style(&style_greenline,LV_COLOR_GREEN,6);
    set_style(&style_redline,LV_COLOR_RED,6);
    set_style(&style_yellowline,LV_COLOR_YELLOW,6);
}

/*      *   *
       *     *
      **     **
     *         *
    ***       ***
   *             *
  *               *  <--static rear guide line like this*/
void initStaticLine(lv_obj_t * obj)
{
    lv_style_t *right_static_guide_line_color[STATIC_GUIDE_LINE] = {&style_greenline,&style_yellowline,&style_redline};
    for(int i=0;i<STATIC_GUIDE_LINE;i++){
        static_guide_line_right[i] = lv_line_create(obj,NULL);
        lv_obj_add_style(static_guide_line_right[i], 0, right_static_guide_line_color[i]);
    }

    lv_style_t *left_static_guide_line_color[STATIC_GUIDE_LINE] = {&style_greenline,&style_yellowline,&style_redline};
    for(int i=0;i<STATIC_GUIDE_LINE;i++){
        static_guide_line_left[i] = lv_line_create(obj,NULL);
        lv_obj_add_style(static_guide_line_left[i], 0, left_static_guide_line_color[i]);
    }
    lv_style_t *left_dynamic_guide_line_color[3] = {&style_greenline,&style_yellowline,&style_redline};
    for(int i=0;i<3;i++){
        dynamic_guide_line_left[i] = lv_line_create(obj,NULL);
        lv_obj_add_style(dynamic_guide_line_left[i], 0, left_dynamic_guide_line_color[i]);
    }

    lv_style_t *right_dynamic_guide_line_color[3] = {&style_redline,&style_yellowline,&style_greenline};
    for(int i=0;i<3;i++){
        dynamic_guide_line_right[i] = lv_line_create(obj,NULL);
        lv_obj_add_style(dynamic_guide_line_right[i], 0, right_dynamic_guide_line_color[i]);
    }

    top_dynamic_line = lv_line_create(obj,NULL);
    lv_obj_add_style(top_dynamic_line, 0, &default_style_line);

    dynamic_guide_line_left_normal = lv_line_create(obj,NULL);
    lv_obj_add_style(dynamic_guide_line_left_normal, 0, &default_style_line);

    dynamic_guide_line_right_normal = lv_line_create(obj,NULL);
    lv_obj_add_style(dynamic_guide_line_right_normal, 0, &default_style_line);

    lv_style_t *hor_dynamic_guide_line_color[DYNAMIC_GUIDE_LINE] = {&style_greenline,&style_greenline,&style_yellowline,&style_redline,
                                                                    &style_greenline,&style_greenline,&style_yellowline,&style_redline};
    for(int i=0;i<DYNAMIC_GUIDE_LINE;i++){
        dynamic_guide_line_hor[i] = lv_line_create(obj,NULL);
        lv_obj_add_style(dynamic_guide_line_hor[i], 0, hor_dynamic_guide_line_color[i]);
    }
}

void init_left_carline()
{
    int line_len = 30*scale;
    int left_1_temp_x = (int)c.x-2.0/9*(c.x-a.x);
    int left_1_temp_y = (int)c.y+2.0/9*(a.y-c.y);
    left_1[0] = (lv_point_t){left_1_temp_x,left_1_temp_y};
    left_1[1] = (lv_point_t){(int)left_1_temp_x+1.0/3*(c.x-left_1_temp_x),(int)left_1_temp_y-1.0/3*(left_1_temp_y-c.y)};
    left_1[2] = (lv_point_t){(int)left_1_temp_x+1.0/3*(c.x-left_1_temp_x)+line_len,(int)left_1_temp_y-1.0/3*(left_1_temp_y-c.y)};
    left_1[3] = (lv_point_t){(int)left_1_temp_x+1.0/3*(c.x-left_1_temp_x),(int)left_1_temp_y-1.0/3*(left_1_temp_y-c.y)};
    left_1[4] = (lv_point_t){(int)left_1_temp_x+2.0/3*(c.x-left_1_temp_x),(int)left_1_temp_y-2.0/3*(left_1_temp_y-c.y)};
    left_1[5] = (lv_point_t){(int)left_1_temp_x+2.0/3*(c.x-left_1_temp_x)+line_len,(int)left_1_temp_y-2.0/3*(left_1_temp_y-c.y)};
    left_1[6] = (lv_point_t){(int)left_1_temp_x+2.0/3*(c.x-left_1_temp_x),(int)left_1_temp_y-2.0/3*(left_1_temp_y-c.y)};
    left_1[7] = (lv_point_t){c.x,c.y};
    int left_2_temp_x = (int)c.x-5.0/9*(c.x-a.x);
    int left_2_temp_y = (int)c.y+5.0/9*(a.y-c.y);
    left_2[0] = (lv_point_t){(int)c.x-2.0/9*(c.x-a.x),(int)c.y+2.0/9*(a.y-c.y)};
    left_2[1] = (lv_point_t){(int)left_2_temp_x+1.0/3*(left_1_temp_x-left_2_temp_x),(int)left_2_temp_y-1.0/3*(left_2_temp_y-left_1_temp_y)};
    left_2[2] = (lv_point_t){(int)left_2_temp_x+1.0/3*(left_1_temp_x-left_2_temp_x)+line_len,(int)left_2_temp_y-1.0/3*(left_2_temp_y-left_1_temp_y)};
    left_2[3] = (lv_point_t){(int)left_2_temp_x+1.0/3*(left_1_temp_x-left_2_temp_x),(int)left_2_temp_y-1.0/3*(left_2_temp_y-left_1_temp_y)};
    left_2[4] = (lv_point_t){left_2_temp_x,left_2_temp_y};

    left_3[0] = (lv_point_t){left_2_temp_x,left_2_temp_y};
    left_3[1] = (lv_point_t){(int)left_2_temp_x-1.0/3*(left_2_temp_x-a.x),(int)left_2_temp_y+1.0/3*(a.y-left_2_temp_y)};
    left_3[2] = (lv_point_t){(int)left_2_temp_x-1.0/3*(left_2_temp_x-a.x)+line_len,(int)left_2_temp_y+1.0/3*(a.y-left_2_temp_y)};
    left_3[3] = (lv_point_t){(int)left_2_temp_x-1.0/3*(left_2_temp_x-a.x),(int)left_2_temp_y+1.0/3*(a.y-left_2_temp_y)};
    left_3[4] = (lv_point_t){a.x,a.y};

    lv_line_set_points(static_guide_line_left[0], left_1, 8);
    lv_line_set_points(static_guide_line_left[1], left_2, 5);
    lv_line_set_points(static_guide_line_left[2], left_3, 5);
}

void init_right_carline(void)
{
    int line_len = 30*scale;
    int right_1_temp_x = (int)d.x+2.0/9*(b.x-d.x);
    int right_1_temp_y = (int)d.y+2.0/9*(b.y-d.y);
    right_1[0] = (lv_point_t){right_1_temp_x,right_1_temp_y};
    right_1[1] = (lv_point_t){(int)right_1_temp_x-1.0/3*(right_1_temp_x - d.x),(int)right_1_temp_y-1.0/3*(right_1_temp_y-d.y)};
    right_1[2] = (lv_point_t){(int)right_1_temp_x-1.0/3*(right_1_temp_x - d.x)-line_len,(int)right_1_temp_y-1.0/3*(right_1_temp_y-d.y)};
    right_1[3] = (lv_point_t){(int)right_1_temp_x-1.0/3*(right_1_temp_x - d.x),(int)right_1_temp_y-1.0/3*(right_1_temp_y-d.y)};
    right_1[4] = (lv_point_t){(int)right_1_temp_x-2.0/3*(right_1_temp_x - d.x),(int)right_1_temp_y-2.0/3*(right_1_temp_y-d.y)};
    right_1[5] = (lv_point_t){(int)right_1_temp_x-2.0/3*(right_1_temp_x - d.x)-line_len,(int)right_1_temp_y-2.0/3*(right_1_temp_y-d.y)};
    right_1[6] = (lv_point_t){(int)right_1_temp_x-2.0/3*(right_1_temp_x - d.x),(int)right_1_temp_y-2.0/3*(right_1_temp_y-d.y)};
    right_1[7] = (lv_point_t){d.x,d.y};

    int right_2_temp_x = (int)d.x+5.0/9*(b.x-d.x);
    int right_2_temp_y = (int)d.y+5.0/9*(b.y-d.y);
    right_2[0] = (lv_point_t){right_2_temp_x,right_2_temp_y};
    right_2[1] = (lv_point_t){(int)right_2_temp_x-1.0/3*(right_2_temp_x - right_1_temp_x),(int)right_2_temp_y-1.0/3*(right_2_temp_y-right_1_temp_y)};
    right_2[2] = (lv_point_t){(int)right_2_temp_x-1.0/3*(right_2_temp_x - right_1_temp_x)-line_len,(int)right_2_temp_y-1.0/3*(right_2_temp_y-right_1_temp_y)};
    right_2[3] = (lv_point_t){(int)right_2_temp_x-1.0/3*(right_2_temp_x - right_1_temp_x),(int)right_2_temp_y-1.0/3*(right_2_temp_y-right_1_temp_y)};
    right_2[4] =  (lv_point_t){right_1_temp_x,right_1_temp_y};

    right_3[0] = (lv_point_t){right_2_temp_x,right_2_temp_y};
    right_3[1] = (lv_point_t){(int)right_2_temp_x+1.0/3*(b.x - right_2_temp_x),(int)right_2_temp_y+1.0/3*(b.y-right_2_temp_y)};
    right_3[2] = (lv_point_t){(int)right_2_temp_x+1.0/3*(b.x - right_2_temp_x)-line_len,(int)right_2_temp_y+1.0/3*(b.y-right_2_temp_y)};
    right_3[3] = (lv_point_t){(int)right_2_temp_x+1.0/3*(b.x - right_2_temp_x),(int)right_2_temp_y+1.0/3*(b.y-right_2_temp_y)};
    right_3[4] = (lv_point_t) {b.x,b.y};

    lv_line_set_points(static_guide_line_right[0], right_1, 8);
    lv_line_set_points(static_guide_line_right[1], right_2, 5);
    lv_line_set_points(static_guide_line_right[2], right_3, 5);
}

void initStaticGuideLine(void)
{
    init_left_carline();
    init_right_carline();
}

void lv_obj_set_rear_angle(void * obj,int32_t v)
{
    line_angle = v;
    lv_obj_invalidate((lv_obj_t *)obj);
}

void init_rear_guide_line(lv_obj_t * obj)
{
    initStyle();
    initStaticLine(obj);
    if(!rear_mode)
         initStaticGuideLine();
}


