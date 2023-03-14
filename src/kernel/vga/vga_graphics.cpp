#include <vga.h>
void write_pixel8(unsigned x, unsigned y, unsigned c) {
	unsigned wd_in_bytes;
	unsigned off;

	wd_in_bytes = 320;
	off = wd_in_bytes * y + x;
	*(uint8_t*)((uint64_t)VGA_MEM-0x18000+off) = c;
}

void write_pixel4p(unsigned x, unsigned y, uint8_t c) {
	*(uint8_t*)((uint64_t)0x34000+x+y*640) = c;
    *(uint8_t*)((uint64_t)0x7f000+y) = 4;
    *(uint8_t*)((uint64_t)0x7f000+480+x) = 4;
}

void copy_plane(void* dst, void* src, int plane) {
    uint8_t pmask = 1 << plane;
    uint64_t wd_in_bytes = 640 / 8;
    for (uint64_t y = 0; y < 480; y++) {
        if (!*(uint8_t*)(0x7f000+y)) { continue; }
        for (uint64_t x = 0; x < 640; x++) {
            if (!*(uint8_t*)(0x7f000+480+x)) { continue; }
            uint64_t off = wd_in_bytes * y + x / 8;
            uint8_t c = *(uint8_t*)((uint64_t)src + (640*y+x));
            uint8_t mask = 0x80 >> (x & 7);
            if(pmask & c)
	        	*(uint8_t*)((uint64_t)dst+off) = *(uint8_t*)((uint64_t)dst+off) | mask;
	        else
	        	*(uint8_t*)((uint64_t)dst+off) = *(uint8_t*)((uint64_t)dst+off) & ~mask;
        }
    }
}

void write_screen() {
    for (int i = 0; i < 4; i++) {
        set_plane(i);
        copy_plane((void*)0xA0000,(void*)0x34000,i);
    }
    memset((void*)0x7f000,0,480+640);
}

void gclear(uint8_t c) {
    memset((void*)0x34000,c,640*480);
    memset((void*)0x7f000,4,480+640);
    set_cursor_pos(0);
    b = c;
}

void fill_rect(uint64_t x0, uint64_t y0, uint64_t width, uint64_t height, uint8_t color) {
    for (uint64_t x = x0; x < x0+width; x++) {
        for (uint64_t y = y0; y < y0+height; y++) {
            write_pixel4p(x,y,color);
        }
    }
}

void draw_font_transparent(uint64_t x, uint64_t y, uint8_t index, uint8_t* font, uint8_t font_height, uint8_t foreground) {
    uint8_t* character = (uint8_t*)((uint64_t)font+font_height*index);
    for (uint8_t font_y = 0; font_y < font_height; font_y++) {
        for (uint8_t draw_x = 0; draw_x < 8; draw_x++) {
            if ((0x80 >> draw_x)&*character) write_pixel4p(x+draw_x,y+font_y,foreground);
        }
        character++;
    }
}

void draw_font(uint64_t x, uint64_t y, uint8_t index, uint8_t* font, uint8_t font_height, uint8_t background, uint8_t foreground) {
    fill_rect(x,y,8,font_height,background);
    draw_font_transparent(x,y,index,font,font_height,foreground);
}

void draw_line(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint8_t color) {
    int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, deltaslowdirection, deltafastdirection, err;
    dx = x2-x1;
    dy = y2-y1;
    incx = sign(dx);
    incy = sign(dy);
    if (dx < 0) dx *= -1;
    if (dy < 0) dy *= -1;
    if (dx > dy) {
        pdx = incx; pdy = 0;
        ddx = incx; ddy = incy;
        deltaslowdirection = dy;   deltafastdirection = dx;
    } else {
        pdx = 0;    pdy = incy;
        ddx = incx; ddy = incy;
        deltaslowdirection = dx;   deltafastdirection = dy;
    }
    x = x1;
    y = y1;
    err = deltafastdirection / 2;
    write_pixel4p(x,y,color);

    for(t = 0; t < deltafastdirection; ++t) {
        err -= deltaslowdirection;
        if(err < 0) {
            err += deltafastdirection;
            x += ddx;
            y += ddy;
        } else {
            x += pdx;
            y += pdy;
        }
        write_pixel4p(x, y, color);
    }
}

void draw_circle(int x0, int y0, int radius,uint8_t color,uint8_t width) {
    if (width == 0) return;
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    write_pixel4p(x0, y0 + radius,color);
    write_pixel4p(x0, y0 - radius,color);
    write_pixel4p(x0 + radius, y0,color);
    write_pixel4p(x0 - radius, y0,color);

    while(x < y)
    {
        if (f >= 0)
        {
            y -= 1;
            ddF_y += 2;
            f += ddF_y;
        }
        x += 1;
        ddF_x += 2;
        f += ddF_x + 1;

        write_pixel4p(x0 + x, y0 + y,color);
        write_pixel4p(x0 - x, y0 + y,color);
        write_pixel4p(x0 + x, y0 - y,color);
        write_pixel4p(x0 - x, y0 - y,color);
        write_pixel4p(x0 + y, y0 + x,color);
        write_pixel4p(x0 - y, y0 + x,color);
        write_pixel4p(x0 + y, y0 - x,color);
        write_pixel4p(x0 - y, y0 - x,color);
    }
}

void draw_ellipse(int xm, int ym, int a, int b, uint8_t color,uint8_t type)
{
    int dx = 0, dy = b;
    long a2 = a*a, b2 = b*b;
    long err = b2-(2*b-1)*a2, e2;

    do
    {
        if (E_3 & type) write_pixel4p(xm + dx, ym + dy, color);
        if (E_4 & type) write_pixel4p(xm - dx, ym + dy, color);
        if (E_1 & type) write_pixel4p(xm - dx, ym - dy, color);
        if (E_2 & type) write_pixel4p(xm + dx, ym - dy, color);
        e2 = 2*err;
        if (e2 <  (2 * dx + 1) * b2) { ++dx; err += (2 * dx + 1) * b2; }
        if (e2 > -(2 * dy - 1) * a2) { --dy; err -= (2 * dy - 1) * a2; }
    }
    while (dy >= 0);

    while (dx++ < a)
    {
        write_pixel4p(xm+dx, ym, color);
        write_pixel4p(xm-dx, ym, color);
    }
}

void draw_rect(uint64_t x0, uint64_t y0, uint64_t width, uint64_t height, uint8_t color) {
    draw_line(x0,y0,x0+width,y0,color);
    draw_line(x0,y0,x0,y0+height,color);
    draw_line(x0+width,y0,x0+width,y0+height,color);
    draw_line(x0,y0+height,x0+width,y0+height,color);
}