
typedef struct{
	float time;
	char* date;
}record;

#ifdef __cplusplus
extern "C"{
#endif
int compare_record(const void* a, const void* b);//Function used by sort function


//loads and sorts records from a file
//returns the amount of records
int load_record(ALLEGRO_FILE* file, record** records, 
		_Bool ret_date /*when false will read only time not date*/);

void show_record(TRACK_DATA* track, char* filename);
#ifdef __cplusplus
}
#endif
// vim: cc=100
