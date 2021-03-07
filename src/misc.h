void add_element(void** list, int *required, int *available, size_t element_size);
void must_init(bool test, const char *description);
char* get_filename(ALLEGRO_FS_ENTRY *e);
float InInterval(float a);
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int PointAndLine(float x, float y, float x1, float y1, float x2, float y2);

// vim: cc=100
