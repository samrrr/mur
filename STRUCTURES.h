typedef struct
{
	int x, y;
	int l_press, l_up, l_down;
	int r_press, r_up, r_down;
}MOUSE;

typedef union
{
	float x, y, z;
	float i[3];
}Vec;
