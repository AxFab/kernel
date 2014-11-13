#include <kernel/cpu.h>
// #include <prv/format.h>

static int fontW = 6;
static int fontH = 9;

int kTty_offsetIn = 0; // TODO remove this lame input

// ===========================================================================


kTty_t screen;
// ===========================================================================


// ===========================================================================
void kTty_Putc (char ch);
kTty_t screen;
// ---------------------------------------------------------------------------
/**
 * As we need to log early during initialization we need to a temporary tty
 */
int kTty_PreSystem (uint32_t* base, int width, int height, int depth)
{
  int lg;


  screen._ptr = base;
  screen._mode = ((unsigned)screen._ptr == 0x000B8000) ? 0 : 1;
  screen._width = width;
  screen._height = height;
  screen._depth = depth;


  screen._length = screen._width * screen._height * screen._depth;
  screen._cursorX = 0;
  screen._cursorY = 0;

  if (screen._mode) {
    screen._column = width / fontW;
    screen._row = height / fontH;
    screen._color = 0x787878;
    screen._bkground = 0x323232;

    lg = width * height;
    while (lg > 0) screen._ptr[--lg] = screen._bkground;

  } else {
    screen._column = screen._width;
    screen._row = screen._height;
    screen._color = 0x0700;
    memset (screen._ptr, 0, screen._length);
  }

  return __noerror();
}


// ===========================================================================

// ---------------------------------------------------------------------------

// typedef struct kStrBuild kStrBuild_t;
// struct kStrBuild
// {
//   char*  _buf;
//   int     _curs;
//   int     _limit;
// };

// int strPutc (int c, kStrBuild_t* sb)
// {
//   if (sb->_curs+1 >= sb->_limit)
//     return -1;
//   sb->_buf [ sb->_curs++] = c;
//   return 0;
// }

// int snprintf (char* buf, size_t limit, const char* str, ...)
// {
//   const char** args = &str;
//   kStrBuild_t sb;
//   sb._curs = 0;
//   sb._buf = buf;
//   sb._limit = limit;
//   int res = format ((_putc_f)strPutc, &sb, str, ++args);
//   sb._buf [ sb._curs++] = 0;
//   return res;
// }

// ===========================================================================


void kTty_MoveCursor (int pos)
{
    outb(0x3d4, 0x0f);
    outb(0x3d5, pos & 0xff);
    outb(0x3d4, 0x0e);
    outb(0x3d5, (pos >> 8) & 0xff);
}

void kTty_ShowCursor (void)
{
    kTty_MoveCursor(screen._cursorX + screen._cursorY * screen._column);
}

void kTty_HideCursor (void)
{
    kTty_MoveCursor(0xffff);
}


static int CtrL_enable;
static int CtrR_enable;
static int AltL_enable;
static int AltR_enable;
static int SftL_enable;
static int SftR_enable;
static int HstL_enable;
static int HstR_enable;

static int Caps_enable;
static int Nums_enable;
static int Scrl_enable;
static int Insr_enable;


/*
void kTty_Update (void)
{
	if (Caps_enable) {
    	kTty_Buffer [0 - 4] = 'C' | 0x0800;
    	kTty_Buffer [0 - 3] = 'a' | 0x0800;
    	kTty_Buffer [0 - 2] = 'p' | 0x0800;
    } else {
    	kTty_Buffer [0 - 4] = ' ' | 0x0800;
    	kTty_Buffer [0 - 3] = ' ' | 0x0800;
    	kTty_Buffer [0 - 2] = ' ' | 0x0800;
    }

	if (Nums_enable) {
    	kTty_Buffer [0 - 8] = 'N' | 0x0800;
    	kTty_Buffer [0 - 7] = 'u' | 0x0800;
    	kTty_Buffer [0 - 6] = 'm' | 0x0800;
    } else {
    	kTty_Buffer [0 - 8] = ' ' | 0x0800;
    	kTty_Buffer [0 - 7] = ' ' | 0x0800;
    	kTty_Buffer [0 - 6] = ' ' | 0x0800;
    }

	if (Scrl_enable) {
    	kTty_Buffer [0 - 12] = 'S' | 0x0800;
    	kTty_Buffer [0 - 11] = 'c' | 0x0800;
    	kTty_Buffer [0 - 10] = 'l' | 0x0800;
    } else {
    	kTty_Buffer [0 - 12] = ' ' | 0x0800;
    	kTty_Buffer [0 - 11] = ' ' | 0x0800;
    	kTty_Buffer [0 - 10] = ' ' | 0x0800;
    }

	if (Insr_enable) {
    	kTty_Buffer [0 - 16] = 'I' | 0x0800;
    	kTty_Buffer [0 - 15] = 'n' | 0x0800;
    	kTty_Buffer [0 - 14] = 's' | 0x0800;
    } else {
    	kTty_Buffer [0 - 16] = ' ' | 0x0800;
    	kTty_Buffer [0 - 15] = ' ' | 0x0800;
    	kTty_Buffer [0 - 14] = ' ' | 0x0800;
    }
}

*/


enum Special_Key {

	Null = '\0',
	BkSp = 0x07,
	Tabl = '\t',
	Entr = '\n',
	Retr = '\r',

	Home = 0x80,
	End	 = 0x81,
	PgUp = 0x82,
	PgDw = 0x83,
	Insr = 0x84,
	Delt = 0x85,

	ArrU = 0x88,
	ArrD = 0x89,
	ArrL = 0x8a,
	ArrR = 0x8b,

	CtrL = 0xc0,
	CtrR = 0xc1,
	AltL = 0xc2,
	AltR = 0xc3,
	SftL = 0xc4,
	SftR = 0xc5,
	HstL = 0xc6,
	HstR = 0xc7,

	Caps = 0xe0,
	Nums = 0xe1,
	Scrl = 0xe2,

	Escp = 0xe8,
	Menu = 0xe9,

	Fn01 = 0xf1,
	Fn02 = 0xf2,
	Fn03 = 0xf3,
	Fn04 = 0xf4,
	Fn05 = 0xf5,
	Fn06 = 0xf6,
	Fn07 = 0xf7,
	Fn08 = 0xf8,
	Fn09 = 0xf9,
	Fn10 = 0xfa,
	Fn11 = 0xfb,
	Fn12 = 0xfc,
};

	unsigned char qwerty_US[2][0x80] = { {
		Null, Escp, '1' , '2' , '3' , '4' , '5' , '6' ,	// 00
		'7' , '8' , '9' , '0' , '-' , '=' , BkSp, Tabl, // 08
		'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , // 10
		'o' , 'p' , '[' , ']' , Entr, CtrL, 'a' , 's' , // 18
		'd' , 'f' , 'g' , 'h' , 'j' , 'k' , 'l' , ';' , // 20
		'\'', '`' , SftL, '\\', 'z' , 'x' , 'c' , 'v' , // 28
		'b' , 'n' , 'm' , ',' , '.' , '/' , SftR, '*' , // 30
		AltL, ' ' , Caps, Fn01, Fn02, Fn03, Fn04, Fn05, // 38
		Fn06, Fn07, Fn08, Fn09, Fn10, Nums, Scrl, '7' , // 40
		'8' , '9' , '-' , '4' , '5' , '6' , '+' , '1' , // 48
		'2' , '3' , '0' , '.' , Null, Null, '\\', Fn11, // 50
		Fn12, Null, Null, HstL, HstR, Menu, Null, Null, // 58
		Null, Null, Null, Null, Null, Null, Null, Null, // 60
		Null, Null, Null, Null, Null, Null, Null, Null, // 68
		Null, Null, Null, Null, Null, Null, Null, Null, // 70
		Null, Null, Null, Null, Null, Null, Null, Null, // 78
	}, {
		Null, Escp, '!' , '@' , '#' , '$' , '%' , '^' ,	// 00
		'&' , '*' , '(' , ')' , '_' , '+' , BkSp, Tabl, // 08
		'Q' , 'W' , 'E' , 'R' , 'T' , 'Y' , 'U' , 'I' , // 10
		'O' , 'P' , '{' , '}' , Retr, CtrL, 'A' , 'S' , // 18
		'D' , 'F' , 'G' , 'H' , 'J' , 'K' , 'L' , ':' , // 20
		'"' , '~' , SftL, '|' , 'Z' , 'X' , 'C' , 'V' , // 28
		'B' , 'N' , 'M' , '<' , '>' , '?' , SftR, '*' , // 30
		AltL, ' ' , Caps, Fn01, Fn02, Fn03, Fn04, Fn05, // 38
		Fn06, Fn07, Fn08, Fn09, Fn10, Nums, Scrl, Home, // 40
		ArrU, PgUp, '-' , ArrL, '5' , ArrR, '+' , End , // 48
		ArrD, PgDw, Insr, Delt, Null, Null, '\\', Fn11, // 50
		Fn12, Null, Null, HstL, HstR, Menu, Null, Null, // 58
		Null, Null, Null, Null, Null, Null, Null, Null, // 60
		Null, Null, Null, Null, Null, Null, Null, Null, // 68
		Null, Null, Null, Null, Null, Null, Null, Null, // 70
		Null, Null, Null, Null, Null, Null, Null, Null, // 78
	}};

void kTty_KeyPress (int c)
{
	int k = (SftL_enable || SftR_enable) ? 1 : 0;
	if ((c < 0x37 && Caps_enable) || (c >= 0x37 && Nums_enable))
    	k = (k == 0 ? 1 : 0);

    c = qwerty_US[k][c];

 	if (c >= 0x80) {
 		switch (c) {
 			case CtrL: CtrL_enable = 1; break;
 			case CtrR: CtrR_enable = 1; break;
 			case AltL: AltL_enable = 1; break;
 			case AltR: AltR_enable = 1; break;
 			case SftL: SftL_enable = 1; break;
 			case SftR: SftR_enable = 1; break;
 			case HstL: HstL_enable = 1; break;
 			case HstR: HstR_enable = 1; break;

 			case Caps: Caps_enable = !Caps_enable; break;
 			case Nums: Nums_enable = !Nums_enable; break;
 			case Scrl: Scrl_enable = !Scrl_enable; break;
 			case Insr: Insr_enable = !Insr_enable; break;

 			case Home: kTty_offsetIn = 0; break;
 			case End: kTty_offsetIn = 0; break; // strlen (input);
 			case PgUp: break;
 			case PgDw: break;

 			case BkSp: break;
 			case Delt: break;

 			case ArrU: break;
 			case ArrD: break;
 			case ArrL: if (kTty_offsetIn > 0) kTty_offsetIn--; break;
 			case ArrR: if (kTty_offsetIn < 0) kTty_offsetIn++; break; // strlen (input);

 		}

 		// kTty_Update ();
 		return;
 	}
 	kTty_Putc (c & 0xff);
}

void kTty_KeyUp (int c)
{
	int k = (SftL_enable || SftR_enable) ? 1 : 0;
	if ((c < 0x37 && Caps_enable) || (c >= 0x37 && Nums_enable))
    	k = (k == 0 ? 1 : 0);

    c = qwerty_US[k][c];

 	if (c >= 0x80) {
 		switch (c) {
 			case CtrL: CtrL_enable = 0; break;
 			case CtrR: CtrR_enable = 0; break;
 			case AltL: AltL_enable = 0; break;
 			case AltR: AltR_enable = 0; break;
 			case SftL: SftL_enable = 0; break;
 			case SftR: SftR_enable = 0; break;
 			case HstL: HstL_enable = 0; break;
 			case HstR: HstR_enable = 0; break;
 		}
 		return;
	}
}

int kTty_Initialize (void)
{
	SftL_enable = SftR_enable = 0;
	Caps_enable = 0;
	Nums_enable = 1;


  /*
	kTty_Buffer = (short*)0xB80a0;
	kTty_offsetX = kTty_offsetY = 0;
	kTty_style = Regular;

	kTty_Update();

  */
  return __noerror ();
}

void kTty_Write (const char *str)
{
	while (*str) {
		kTty_Putc (*str);
		str++;
	}

}

/*
void kTty_Putc (char c)
{
	if (c == '\n') {
		kTty_Eol ();
	} else {
		if (c < 0)
			c = '?';
		else if (c < 0x20)
			c = 0x20;
		kTty_Buffer[kTty_offsetX + kTty_offsetY] = (c & 0xff) | kTty_style;
		kTty_offsetX++;
		if (kTty_offsetX >= 38)
			kTty_Eol ();
	}
}
*/



// =========
/*
short kTty_Font[] = {
  0x0000, 0x2092, 0x002d, 0x0000, 0x0000, 0x0000, 0x0000, 0x0024, // 20-27
  0x4494, 0x1491, 0x0000, 0x0000, 0x2400, 0x0000, 0x2000, 0x1294, // 28-2f
  0x7b6f, 0x4924, 0x73e7, 0x79a7, 0x49fd, 0x79af, 0x7baf, 0x492f, // 30-37
  0x7bef, 0x79ef, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 38-3f
  0x0000, 0x5bea, 0x3aeb, 0x724f, 0x3b6b, 0x72cf, 0x12cf, 0x7b4f, // 40-47
  0x5bed, 0x7497, 0x3497, 0x5aed, 0x7249, 0x5b7d, 0xd635, 0xab6a, // 48-4f H
  0x13ef, 0x0000, 0x5aef, 0x79cf, 0x2497, 0x7b6d, 0x2b6d, 0x0000, // 50-57 P
  0x5aad, 0x0000, 0x788f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 58-5f X
  0x0000, 0x5bea, 0x3aeb, 0x724f, 0x3b6b, 0x72cf, 0x12cf, 0x7b4f, // 60-67
  0x5bed, 0x7497, 0x3497, 0x5aed, 0x7249, 0x5b7d, 0xd635, 0xab6a, // 68-6f H
  0x13ef, 0x0000, 0x5aef, 0x79cf, 0x2497, 0x7b6d, 0x2b6d, 0x0000, // 70-77 P
  0x5aad, 0x0000, 0x788f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 78-7f X
};*/


uint64_t kTty_Font[] = {
  0x00000000000000, 0x00004004104104, 0x0000000000028a, 0x0000a7ca51e500, // 20-23
  0x0010f30f1453c4, 0x0004d3821072c8, 0x00016255085246, 0x00000000000104, // 24-27
  0x08104104104108, 0x04208208208204, 0x00000284384000, 0x0000411f104000, // 28-2b
  0x00084180000000, 0x0000000e000000, 0x00006180000000, 0x00082104208410, // 2c-2f

  0x0000f24924924f, 0x00004104104106, 0x0000f0413c820f, 0x0000f20838820f, // 30-33 - 0
  0x0000820f249249, 0x0000f2083c104f, 0x0000f24924f04f, 0x0000208210420f, // 34-37 - 4
  0x0000f2493c924f, 0x0000f20f24924f, 0x00006180186000, 0x00084180186000, // 38-3b - 8
  0x00018181198000, 0x00000380380000, 0x00003310303000, 0x0000400410844e, // 3c-3f

  0x1f05d55d45f000, 0x000114517d145f, 0x0001f4517c924f, 0x0000f04104104f, // 40-23
  0x0000f45145144f, 0x0000f0411c104f, 0x000010411c104f, 0x0001f45174104f, // 44-27 - D
  0x000114517d1451, 0x00002082082082, 0x00007208208208, 0x00009143043149, // 48-2b - H
  0x0000f041041041, 0x0001155555555f, 0x0001145145145f, 0x0001f45145145f, // 4c-2f - L

  0x000010417d145f, 0x0041f55145145f, 0x000114517c924f, 0x0000f2083c104f, // 50-23 - P
  0x0000410410411f, 0x0001f451451451, 0x0000410a291451, 0x0001b555555451, // 54-27 - T
  0x0001144a10a451, 0x000041047d1451, 0x0000f04108420f, 0x0c10410410410c, // 58-2b - X
  0x00410208104082, 0x06104104104106, 0x00000000011284, 0x0003f000000000, // 5c-2f

  0x00000000000204, 0x0000f24f20f000, 0x0000f24924f041, 0x0000f04104f000, // 60-23
  0x0000f24924f208, 0x0000f04f24f000, 0x00001041043047, 0x0f20f24924f000, // 64-27 - d
  0x0000924924f041, 0x00002082082002, 0x000c2082082002, 0x00009143149041, // 68-2b - h
  0x00007041041041, 0x0001555555f000, 0x0000924924f000, 0x0000f24924f000, // 6c-2f - l

  0x0104f24924f000, 0x0820f24924f000, 0x0000104124f000, 0x0000f20f04f000, // 70-23 - p
  0x00007041043041, 0x0000f249249000, 0x0000428a451000, 0x0001f555555000, // 74-27 - t
  0x00009246249000, 0x0f20f249249000, 0x0000f04210f000, 0x18104103104118, // 78-2b - x
  0x04104104104104, 0x03104118104103, 0x00000000352000, 0x00000000000000, // 7c-2f
};

void kTty_EOL ()
{
  screen._cursorX = 0;
  screen._cursorY++;
  if (screen._cursorY >= 58 /* screen._row */) {
    screen._cursorY -= 1;

    memcpy (screen._ptr, &screen._ptr[(fontH+1) * screen._width ], screen._width * (screen._height - fontH-1)*4);
    // memset (screen._ptr, &screen._ptr[600 - (fontH+1) * 800 ], (fontH+1) * 800 *4);
  }
}

/*
void kTty_Eol ()
{
  kTty_offsetY += 80;
  kTty_offsetX = 0;
  if (kTty_offsetY > 80 * 22 + 40)
    kTty_Scroll ();
}

void kTty_Scroll ()
{
  kTty_offsetY = 40;
  kTty_offsetX = 0;
}
*/

void kTty_Putc (char ch)
{
  int i, j;
  uint64_t vl;
  int c = ch;
  int k;

  if (c == '\n') {
    kTty_EOL();
    return;
  }

  if (c < 0x20 || c >= 0x80) c = 0x20;


  if (screen._mode == 1) {
    k = 1 + screen._width + screen._cursorY * (fontH+1) * screen._width +
        screen._cursorX * (fontW);
    vl = (uint64_t)kTty_Font[c - 0x20];
    for (j=0; j<fontH; ++j) {
      for (i=0; i<fontW; ++i) {
        screen._ptr[k + j * screen._width + i] = (vl & 1) ? screen._color : screen._bkground;
        vl = vl >> 1;
      }
    }
  } else {
    ((uint16_t*)screen._ptr)[screen._cursorX + screen._cursorY * 80] = (c & 0xff) | screen._color;
  }

  screen._cursorX++;
  if (screen._cursorX >= screen._column) {
    kTty_EOL ();
  }
}



void kTty_HexChar (unsigned int value, int size)
{
	value = value << ((8 - size) * 4);
	while (size-- > 0) {
		unsigned int digit = (value >> 28) & 0xf;
		kTty_Putc (digit < 10 ? digit + '0' : digit - 10 + 'a');
		value = value << 4;
	}
}

void kTty_HexDump (unsigned char* ptr, int length)
{
	int i;
	while (length > 0) {
    kTty_Write ("0x");
		kTty_HexChar ((unsigned int)ptr, 8);
		kTty_Write (" ::  ");
		for (i=0; i<16; i++) {
      kTty_HexChar ((unsigned int)ptr[i], 2);
      kTty_Putc (' ');
    }

    for (i=0; i<16; i++) {
      if (ptr[i] < 0x20)
        kTty_Putc ('.');
      else if (ptr[i] > 0x80)
        kTty_Putc ('_');
      else
        kTty_Putc (ptr[i]);
    }

		kTty_Putc ('\n');
		length -= 16;
		ptr += 16;
	}
	kTty_Putc ('\n');
}






typedef struct kTtyTerm kTtyTerm_t;

struct kTtyTerm
{
  char*         buffer_;
  size_t        length_;
  int           offset_;
  int           attribute_;
  kTtyTerm_t*   next_;
};


kTtyTerm_t klog = {
  (char*)0x7000,
  0x10000 - 0x7000,
  // 0x200,
  0,
  0,
  NULL
};


kTtyTerm_t* term = &klog;


void kTty_Update ()
{
  char *str;
  int lg;

  while (1) {
    str = &term->buffer_[term->offset_];
    int max = term->length_ - term->offset_;
    lg = 0;

    while (str[lg] != '\0' && str[lg] != '\n' && lg < max) ++lg;

    if (str[lg] == '\0') break;
    if (str[lg] == '\n') lg++;

    term->offset_ += lg;
    int i;

    for (i=0; i<lg; ++i)
      kTty_Putc (str[i]);


    if ((size_t)term->offset_ >= term->length_) {
      term->offset_ = 0;
    }
  }
}

void kTty_NewTerminal (uintptr_t base, size_t limit)
{
  kTtyTerm_t* neo = kalloc(sizeof(kTtyTerm_t));
  neo->next_ = term;
  neo->buffer_ = (char*)0x7000;
  neo->length_ = limit;
  term = neo;
  kTty_Update ();
}


