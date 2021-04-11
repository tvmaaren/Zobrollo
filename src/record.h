
typedef struct{
	float time;
	char* date;
}record;
int compare_record(const void* a, const void* b);//Function used by sort function


//loads and sorts records from a file
//returns the amount of records
int load_record(ALLEGRO_FILE* file, record** records, 
		_Bool ret_date /*when false will read only time not date*/);

void show_record(TRACK_DATA* track, char* filename, CONFIG* config, ALLEGRO_DISPLAY* disp, 
		PATHS* paths, ALLEGRO_EVENT *event, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);
// vim: cc=100
