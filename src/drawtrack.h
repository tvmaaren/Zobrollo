#define STRAIGHT false
#define CIRCLE true

#define RIGHT false
#define LEFT true


typedef struct{
	float x1;
	float y1;
	float x2;
	float y2;
}LINE;

typedef struct{
	float start_angle;
	float delta_angle;
	float midx;
	float midy;
	float r_inner;
	float r_outer;
	float r_mid;
}CIRCLE_SEGMENT;

typedef struct{
	LINE inner;
	LINE outer;
	float length;
	float angle;
}LINE_SEGMENT;

typedef struct{
	void** segments;
	_Bool* segment_types;
	int n_segments;
	
	float trackwidth;
	float border_width;
	
	_Bool milestones;//When false milestones will be disabled
	float milestone_interval;
	float milestone_size;
	
	ALLEGRO_COLOR border_color;
	float max_min[4];//0: max x, 1: max y, 2: min x, 3: min y

}TRACK_DATA;
#define track_min_y 3
#define track_min_x 2
#define track_max_y 1
#define track_max_x 0

void add_element(void** list, int *required, int *available, size_t element_size);
int GetSimpleSegments(void*** segments, _Bool** segment_types, float* trackwidth, 
		ALLEGRO_FILE *file);
void drawtrack(TRACK_DATA *track_data, float scale);
void loadtrack(ALLEGRO_FILE* file, TRACK_DATA* track_data);
void cart2pol(float x, float y, float *angle, float *dist);
int get_cur_segment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA *track_data);
// vim: cc=100
