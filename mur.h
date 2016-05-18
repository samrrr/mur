#include <list>
#include <time.h>

class vec{
public:
	float x, y;
	friend vec operator+(vec _a, vec _b);
	friend vec operator-(vec _a, vec _b);
	void norm()
	{
		float r = sqrt(x*x + y*y);
		if (r == 0)
		{
			int c = rand() % 360;
			dd(&x,&y,c,1);
		}
		else
		{
			x /= r;
			y /= r;
		}
	}
};
vec operator+(vec _a, vec _b)
{
	vec c;
	c.x = _a.x + _b.x;
	c.y = _a.y + _b.y;

	return c;
};
vec operator-(vec _a, vec _b)
{
	vec c;
	c.x = _a.x - _b.x;
	c.y = _a.y - _b.y;

	return c;
};

typedef struct{
	int food;
	float z[5];
	int t;//1-grass 2-food point 3-food store 4-stone
}poiint;

struct MUR{
	float x, y, c;
	int t;
	char role[20];
	int food;
	int stay;
	int last_stay_ti;
	union
	{
		char buff[1];//can use buff[0] [1] [2] ....
		struct{
			vec napr[5];
			vec pos[5];
			vec zap[5];
			int to;
			int type;//0-marking 1-go home
			float mark_level;//1-near base 0.5 - not near...
			float cn;
		}base;
		struct{
			int to;//ticks to back home
			int type;//0-marking 1-go home
			float mark_level;//1-near base 0.5 - not near...
			int stuck_timer;
			float stuck_level;
		}food;
		struct{
			int to;//ticks to back home
			int type;//0-marking 1-go home
			float mark_level;//1-near food 0.5 - not near...
		}home;
	}info;
};

string i_to_s(int i)
{
	char buff[345];
	itoa(i, buff, 10);
	return buff;
}

typedef struct
{
	int stones;
	int foods;
	int food_pixel;
	int stone_size;
	int mur_sco;
	int mur_fooders;
}GDATA;

class MU_ALG
{
private:
	poiint **pole;
	int size_x, size_y;
	bool is_init;
	//unsigned char *color_data;
	int mur_posx;
	int mur_posy;
	int ti;
	int MAX_SCO;
	int MAX_FOO;

public:

	MUR mur[10000];

	MU_ALG()
	{
		is_init = 0;
		size_x = 0;
		size_y = 0;
		pole = NULL;
		ti = 0;
	}

	void init(int _x, int _y,GDATA gen)
	{
		if (_x < 20 || _x > 1000 || _y < 20 || _y > 1000)
		{
			out.put("POLE: init error");
			return;
		}

		MAX_SCO = gen.mur_sco;
		MAX_FOO = gen.mur_fooders;

		time_t t_str;
		int i, r, o, j;

		time(&t_str);

		for (i = 0; i < t_str % 10000; i++)
			rand();

		for (i = 0; i < 10000; i++)
			mur[i].t = 0;

		mur_posx = rand() % (_x - 20) / 2 + 10 + (_x - 20) / 4;
		mur_posy = rand() % (_y - 20) / 2 + 10 + (_y - 20) / 4;

		size_x = _x;
		size_y = _y;

		is_init = 1;


		pole = (poiint**)malloc(sizeof(poiint*)*size_x);
		for (r = 0; r < size_x; r++)
			pole[r] = (poiint*)malloc(sizeof(poiint)*size_y);

		//color_data = (unsigned char *)malloc(sizeof(char)*size_x*size_y * 3 * 6);

		for (r = 0; r < size_x; r++)
			for (o = 0; o < size_y; o++)
			{
				pole[r][o].t = 1;
				pole[r][o].food = 0;
				pole[r][o].z[0] = 0;
				pole[r][o].z[1] = 0;
				pole[r][o].z[2] = 0;
				pole[r][o].z[3] = 0;
				pole[r][o].z[4] = 0;
			}

		for (i = 0; i < (size_x + 200) * (size_y + 200)*gen.stones / 100000; i++)
		{
			int x, y,rad;
			x = rand() % (size_x + 200) - 100;
			y = rand() % (size_y + 200) - 100;
			rad = (rand() % 40 + 80)/100.0*gen.stone_size;	

			for (r = -rad; r < rad; r++)
				for (o = -rad; o < rad; o++)
					if (x + r >= 0 && y + o >= 0 && x + r < size_x && y + o < size_y)
						if ((r + ((y + o) % 2 ? 0 : 0.5))*(r + ((y + o) % 2 ? 0 : 0.5)) + (o)*(o) <= rad* rad)
					{
						pole[x + r][y + o].t = 4;
					}
		}

		
		for (i = 0; i < (size_x + 200) * (size_y + 200)*gen.foods / 10000000; i++)
		{

			int x, y, rad;
			x = rand() % (size_x + 200) - 100;
			y = rand() % (size_y + 200) - 100;
			rad = 5.5;

			for (r = -rad; r < rad; r++)
				for (o = -rad; o < rad; o++)
					if (x + r >= 0 && y + o >= 0 && x + r < size_x && y + o < size_y)
						if ((r + ((y + o) % 2 ? 0 : 0.5))*(r + ((y + o) % 2 ? 0 : 0.5)) + (o)*(o) <= rad*rad)
						{
							if (pole[x + r][y + o].t != 4)
								pole[x + r][y + o].food = gen.food_pixel;
						}



		}

		for (i = -10; i <= 10; i++)
			for (r = -10; r <= 10; r++)
				if ((i + ((r + mur_posy) % 2 ? 0 : 0.5))*(i + ((r + mur_posy) % 2 ? 0 : 0.5)) + (r)*(r)<90)
				{
					pole[mur_posx + i][mur_posy + r].t = 1;
				}
		//

	}
	
	poiint get_front(MUR &m)
	{
		int x, y;

		y = m.y + ddy(m.c, 0.5);

		if (y % 2 == 0)
			x = m.x + ddx(m.c, 0.5);
		else
			x = m.x + ddx(m.c, 0.5) + 0.5;

		if (x >= 0 && y >= 0 && x < size_x && y < size_y)
		{
			return pole[x][y];
		}

		poiint c;

		c.food = 0;
		c.t = 4;
		c.z[0] = 0;
		c.z[1] = 0;
		c.z[2] = 0;
		c.z[3] = 0;
		c.z[5] = 0;

		return c;
	}
	
	poiint* get_po(MUR &m)
	{
		int x, y;

		y = m.y;

		if (y % 2 == 0)
			x = m.x;
		else
			x = m.x+0.5;

		if (x >= 0 && y >= 0 && x < size_x && y < size_y)
		{
			return &pole[x][y];
		}
		return NULL;


	}
	void add_mur(int _x, int _y, int _t)
	{
		int i, r, o, j;
		r = rand() % 9998+2;
		for (i = r; i != r - 1; i++)
		{
			if (i >= 10000)
				i = 0;

			if (mur[i].t == 0)
			{
				if (_t == 1)
				{
					mur[i].stay = 1;
					mur[i].t = 1;
					mur[i].x = _x;
					mur[i].y = _y;
					mur[i].c = (rand() % 360) / 1.0;
					strcpy(mur[i].role, "base");

					for (r = 0; r < 5; r++)
					{
						mur[i].info.base.napr[r].x = 0;
						mur[i].info.base.napr[r].y = 0;
						mur[i].info.base.pos[r].x = 0;
						mur[i].info.base.pos[r].y = 0;
						mur[i].info.base.napr[r].x = 0;
						mur[i].info.base.napr[r].y = 0;
					}

					mur[i].info.base.type = 0;
					mur[i].info.base.cn = rand() % 360;
					mur[i].info.base.mark_level = 1;
					mur[i].last_stay_ti = 0;
				}

				if (_t == 2)
				{
					mur[i].stay = 1;
					mur[i].t = 1;
					mur[i].x = _x;
					mur[i].y = _y;
					mur[i].c = (rand() % 360) / 1.0;
					strcpy(mur[i].role, "food");

					mur[i].info.food.to = 500;
					mur[i].info.food.type = 0;
					mur[i].info.food.mark_level = 0; 
					mur[i].info.food.stuck_timer = 0;
					mur[i].info.food.stuck_level = 0;
					mur[i].last_stay_ti = 0;
				}

				return;
			}
		}
	}

	void addfood(GDATA gen)
	{
		int i,r,o,x, y, rad;
		x = rand() % (size_x + 200) - 100;
		y = rand() % (size_y + 200) - 100;
		rad = 5.5;

		for (r = -rad; r < rad; r++)
			for (o = -rad; o < rad; o++)
				if (x + r >= 0 && y + o >= 0 && x + r < size_x && y + o < size_y)
					if ((r + ((y + o) % 2 ? 0 : 0.5))*(r + ((y + o) % 2 ? 0 : 0.5)) + (o)*(o) <= rad*rad)
					{
						if (pole[x + r][y + o].t != 4)
							pole[x + r][y + o].food = gen.food_pixel;
					}
	}

	void upd()
	{
		ti++;

		if (!is_init)
			return;

		int i, r, o, j;
		int sizex, sizey;

		sizex = size_x * 3;
		sizey = size_y * 3;


		for (i = -2; i <= 2; i++)
			for (r = -2; r <= 2; r++)
				pole[mur_posx + i][mur_posy + r].z[0] = 1;

		for (i = 0; i < size_x; i++)
			for (r = 0; r < size_y; r++)
			{
				if (ti % 10 == 0)
					pole[i][r].z[0] *= 0.999;
				if (ti % 2 == 0)
					pole[i][r].z[1] *= 0.998;
				pole[i][r].z[1] -= 0.00005;
				if (pole[i][r].z[1] < 0)
					pole[i][r].z[1] = 0;
			}

//		if (rand() % 1 == 0)
//			add_mur(mur_posx + 1, mur_posy + 1, 1);

//		if (rand() % 1 == 0)
//			add_mur(mur_posx + 1, mur_posy + 1, 2);


		int mu_sco = 0;
		int mu_foo = 0;
		int mu_all = 0;

		for (i = 0; i < 10000; i++)
			if (mur[i].t != 0)
			{
				mu_all++;
				if (strcmp(mur[i].role, "base") == 0)
				{
					mu_sco++;
				}
				if (strcmp(mur[i].role, "food") == 0)
				{
					mu_foo++;
				}
			}

		out.set(15, "scouts: " + i_to_s(mu_sco));
		out.set(16, "fooders:" + i_to_s(mu_foo));
		out.set(17, "all:    " + i_to_s(mu_all));

		if (mu_sco < MAX_SCO)
			for (; mu_sco < MAX_SCO && mu_all < MAX_FOO + MAX_SCO; mu_sco++, mu_all++)
				add_mur(mur_posx + 1, mur_posy + 1, 1);

		if (mu_foo < MAX_FOO)
			for (; mu_foo < MAX_FOO && mu_all < MAX_FOO + MAX_SCO; mu_foo++, mu_all++)
				add_mur(mur_posx + 1, mur_posy + 1, 2);


		for (i = 0; i < 10000; i++)
			if (mur[i].t != 0)
			{

				// MUR BRAIN

				if (strcmp(mur[i].role, "base") == 0)
				{

i = i;

MUR &m = mur[i];
poiint *po = get_po(m);
float u, v;
float MARK_ADD = 0.30;

m.stay = 0;
if (m.c - m.info.base.cn > 0 && m.c - m.info.base.cn > 180)
	m.c += rand() % 41 - 20-4;
else
	m.c += rand() % 41 - 20+4;

m.info.base.cn += (rand() % 3) - 1;

u = po->z[0];
poiint poi = get_front(m);
if (poi.t == 4)
{
	m.c += rand() % 181 - 90;
	m.info.base.cn = (rand() % 3600) / 10.0;
}

if (m.info.base.mark_level > u)
{
	if (u + MARK_ADD > m.info.base.mark_level)
		po->z[0] = m.info.base.mark_level;
	else
		po->z[0] = u + MARK_ADD;
}

m.info.base.mark_level = po->z[0];

m.info.base.mark_level *= 0.99;


if (m.info.base.mark_level < 0.01)
{
	strcpy(m.role, "home");
	m.info.home.mark_level = 0;
	m.info.home.to = 0;
	m.info.home.type = 0;
}

if (po->food > 0)
{
	strcpy(m.role, "home");
	m.info.home.mark_level = 1;
	m.info.home.to = 0;
	m.info.home.type = 0;
}


i = i;

				}

				if (strcmp(mur[i].role, "home") == 0)
				{


i = i;



MUR &m = mur[i];
poiint *po = get_po(m);
float u, v;
float MARK_ADD = 0.30;

u = po->z[1];

if (m.info.home.mark_level > u)
{
	if (u + MARK_ADD > m.info.home.mark_level)
		po->z[1] = m.info.home.mark_level;
	else
		po->z[1] = u + MARK_ADD;
}

m.info.home.mark_level = po->z[1];

m.info.home.mark_level *= 0.995;

m.stay = 1;
u = po->z[0];
if (u < 0.003)
{
	m.stay = 0;
	m.c += rand() % 41 - 22;
}
else
{
	m.c += rand() % 5 - 2;

	poiint poi = get_front(m);

	if (poi.z[0] < u)
	{
		m.c += 45;
		poi = get_front(m);
		if (poi.z[0] < u)
		{
			m.c -= 90;
			poi = get_front(m);
			if (poi.z[0] < u)
			{
				m.c += 45;
				m.c += rand() % 181 - 90;
			}
			else
				m.stay = 0;
		}
		else
			m.stay = 0;
	}
	else
	{
		m.stay = 0;
	}


}

if (u >= 1)
	m.t = 0;


i = i;

				}

				if (strcmp(mur[i].role, "food") == 0)
				{

i = i;


MUR &m = mur[i];
poiint *po = get_po(m);
float u, v;
float MARK_ADD = 0.10;
poiint poi = get_front(m);

u = po->z[0];

if (m.info.food.mark_level > u)
{
	if (u + MARK_ADD > m.info.food.mark_level)
		po->z[0] = m.info.food.mark_level;
	else
		po->z[0] = u + MARK_ADD;
}

m.info.food.mark_level = po->z[0];

m.info.food.mark_level *= 0.99;

m.stay = 1;

m.info.food.to--;

u = po->z[0];

if (po->z[1] == 0 || m.info.food.to >= 200)
if (u > 0.003)
{
	m.stay = 0;
	m.c += rand() % 41 - 22;
	if (poi.t == 4)
		m.c += rand() % 181 - 90;
}
else
{
	m.stay = 0;
	m.c += rand() % 181 - 90;
}

if (m.info.food.to < 200)
if (po->z[1] > 0)
{
	u = po->z[1];

	m.info.food.to = 100;

	m.stay = 1;
	m.info.food.stuck_timer++;

	if (m.last_stay_ti>5)
		m.info.food.stuck_timer = 0;

	if (m.info.food.stuck_level < poi.z[1])
	{
		m.info.food.stuck_level = poi.z[1];
		m.info.food.stuck_timer = 0;
	}
	if (m.info.food.stuck_timer > 50)
		m.info.food.to = 300;

	if (poi.z[1] < u && poi.food == 0)
	{
		m.c += 45;
		poi = get_front(m);
		if (poi.z[1] < u)
		{
			m.c -= 90;
			poi = get_front(m);
			if (poi.z[1] < u)
			{
				m.c += 45;
				m.c += rand() % 181 - 90;
				/** /
				po->z[1] -= 0.001;
				if (po->z[1] < 0)
					po->z[1] = 0;
				/**/

			}
			else
				m.stay = 0;
		}
		else
			m.stay = 0;
	}
	else
	{
		m.stay = 0;
	}

}

if (po->food > 0)
{
	u = po->z[0];

	if (m.info.food.mark_level > u)
	{
		if (u + MARK_ADD > m.info.food.mark_level)
			po->z[0] = m.info.food.mark_level;
		else
			po->z[0] = u + MARK_ADD;
	}

	m.info.food.mark_level = po->z[0];

	m.info.food.mark_level *= 0.99;

	po->food--;
	po->z[1]=1;
	strcpy(m.role, "home");
	m.info.home.mark_level = 0;
	m.info.home.to = 0;
	m.info.home.type = 0;
}
if (m.info.food.to <= 0)
{
	strcpy(m.role, "home");
	m.info.home.mark_level = 0;
	m.info.home.to = 0;
	m.info.home.type = 0;
}


i = i;

				}

				//UPD MUR


				if (mur[i].stay == 0)
				{
					mur[i].last_stay_ti++;
					mur[i].x += ddx(mur[i].c, 0.5);
					mur[i].y += ddy(mur[i].c, 0.5);
					
					if (get_po(mur[i])->t == 4)
					{
						mur[i].x -= ddx(mur[i].c, 0.5);
						mur[i].y -= ddy(mur[i].c, 0.5);
						mur[i].last_stay_ti = 0;
					}
				}
				else
					mur[i].last_stay_ti = 0;


				if (mur[i].x < 0 + 5 || mur[i].y < 0 + 5 || mur[i].x >= size_x - 5 || mur[i].y >= size_y - 5)
					mur[i].t = 0;
			}
	}

	void tex_data_full(char* _data, int _x, int _y)
	{
		if (!is_init)
			return;
		int i, r, o;
		int stx;
		int cr[5], cg[5], cb[5];

		stx = -1;
		while (_x > 0)
		{
			_x >>= 1;
			stx++;
		}

		for (r = 0; r < size_x; r++)
			for (o = 0; o < size_y; o++)
			{


				switch (pole[r][o].t)
				{
				case 1:
					cr[0] = 0;
					cg[0] = 170;
					cb[0] = 0;

					if (pole[r][o].food > 0)
					{
						cr[0] = 0;
						cg[0] = 255;
						cb[0] = 255;
					}

					break;
				case 4:
					cr[0] = 170;
					cg[0] = 170;
					cb[0] = 170;
					break;
				default:
					cr[0] = 0;
					cg[0] = 0;
					cb[0] = 0;
					break;
				}

				cr[1] = pole[r][o].z[1] * 255;
				cg[1] = pole[r][o].z[0] * 255;
				cb[1] = pole[r][o].z[1] * 255;

				if ((r - mur_posx)*(r - mur_posx) + (o - mur_posy)*(o - mur_posy))
				{

				}


				if (o % 2)
				{
					_data[((r << 1) + (o << stx)) * 3 + 0] = (cr[0] + cr[1]) >> 1;
					_data[((r << 1) + (o << stx)) * 3 + 1] = (cg[0] + cg[1]) >> 1;
					_data[((r << 1) + (o << stx)) * 3 + 2] = (cb[0] + cb[1]) >> 1;

					_data[((r << 1) + 1 + (o << stx)) * 3 + 0] = (cr[0] + cr[1]) >> 1;
					_data[((r << 1) + 1 + (o << stx)) * 3 + 1] = (cg[0] + cg[1]) >> 1;
					_data[((r << 1) + 1 + (o << stx)) * 3 + 2] = (cb[0] + cb[1]) >> 1;
				}
				else
				{
					_data[((r << 1)+2 + (o << stx)) * 3 + 0] = (cr[0] + cr[1]) >> 1;
					_data[((r << 1)+2 + (o << stx)) * 3 + 1] = (cg[0] + cg[1]) >> 1;
					_data[((r << 1)+2 + (o << stx)) * 3 + 2] = (cb[0] + cb[1]) >> 1;

					_data[((r << 1) + 1 + (o << stx)) * 3 + 0] = (cr[0] + cr[1]) >> 1;
					_data[((r << 1) + 1 + (o << stx)) * 3 + 1] = (cg[0] + cg[1]) >> 1;
					_data[((r << 1) + 1 + (o << stx)) * 3 + 2] = (cb[0] + cb[1]) >> 1;
				}

			}
		/** /
		for (i = 0; i < 10000; i++)
			if (mur[i].t != 0)
			{
				int x1, y1;
				x1 = mur[i].x;
				y1 = mur[i].y;
				if (x1 >= 0 && y1 >= 0 && x1 < size_x && y1 < size_y)
				{
					_data[(x1 + y1 * _x) * 3 + 0] = 0;
					_data[(x1 + y1 * _x) * 3 + 1] = 0;
					_data[(x1 + y1 * _x) * 3 + 2] = 0;
				}
			}
		/**/



	}

	bool IS_init()
	{
		return is_init;
	}
};
