#define SGFX_OPTIMIZE

#include "sgfx.h"

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b) ? (b) : (x)))
#define ABS(x) (((x) < 0) ? (-(x)) : (x))


static void * x_memset (void* dest, int value, size_t len)
{
    size_t dwords = len;
    len %= 4;
    	asm ("cld\n" "rep stosl" : : "a"(value), "D"(dest), "c"(dwords));
        asm (        "rep movsb" : : "c"(len));
}

void sgfx_init(GraphicsContext* ctx, uint32_t* framebuffer, uint32_t width, uint32_t height) {
    ctx->framebuffer = framebuffer;
    ctx->width = width;
    ctx->height = height;
    ctx->bytesize = width * height * 4;;
}

#ifndef SGFX_OPTIMIZE
void sgfx_fill(const GraphicsContext* ctx, uint32_t color) {
    uint32_t size = ctx->bytesize >> 2;
    for (uint32_t i = 0; i < size; i++) {
        ctx->framebuffer[i] = color;
    }
}
#else
void sgfx_fill(const GraphicsContext* ctx, uint32_t color) {
    uint32_t size = ctx->bytesize >> 2;
    x_memset(ctx->framebuffer,color,size);
}
#endif

void sgfx_fill_rect(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    // TODO: optimize?
    for (int32_t yi = 0; yi < height; yi++) {
        int32_t _y = yi + y;
        if (_y < 0 || _y >= (int32_t) ctx->height)
            continue;

        for (int32_t xi = 0; xi < width; xi++) {
            int32_t _x = xi + x;
            if (_x < 0 || _x >= (int32_t) ctx->width)
                continue;
            
            ctx->framebuffer[_x + _y * ctx->width] = color;
        }
    }
}

void sgfx_copy_rect(const GraphicsContext* ctx, int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, uint32_t* source) {
    for (int32_t yi = 0; yi < height; yi++) {
        int32_t _y = yi + ydest;
        if (_y < 0 || _y >= (int32_t) ctx->height)
            continue;

        for (int32_t xi = 0; xi < width; xi++) {
            int32_t _x = xi + xdest;
            if (_x < 0 || _x >= (int32_t) ctx->width)
                continue;
            
            uint32_t c = source[(xsrc + xi) + (ysrc + yi) * width];
            if ((c >> 24) == 0)
                continue;
            ctx->framebuffer[_x + _y * ctx->width] = c;
        }
    }
}

#include "sgfx_font.inc"
#define READ_BIT(bitmap, index) (bitmap[((uint32_t) index) / 8] & (0x80 >> (((uint32_t) index) % 8)))

void sgfx_draw_char(const GraphicsContext* ctx, uint8_t c, int32_t x0, int32_t y0, uint32_t color) {
    int32_t char_x = (int32_t) (c & 0xF) * 10;
    int32_t char_y = (int32_t) (c >> 4) * 10;

    for (int y = 0; y < 10; y++) {
        int sy = y0 + y;
        if (sy < 0 || sy >= (int32_t) ctx->height)
            continue;

        for (int x = 0; x < 10; x++) {
            int sx = x0 + x;
            if (sx < 0 || sx >= (int32_t) ctx->width)
                continue;

            int iy = y + char_y;
            int ix = x + char_x;
            uint8_t b = READ_BIT(sgfx_font_bitmap, (ix + iy * 160));

            if (!b)
                continue;
            
            ctx->framebuffer[sx + sy * ctx->width] = color;
        }
    }
}

void sgfx_draw_string(const GraphicsContext* ctx, const char* str, int32_t x0, int32_t y0, uint32_t color) {
    char* c = str;

    int32_t x = x0;
    int32_t y = y0;
    while (*c != '\0') {
        if (*c != ' ')
            sgfx_draw_char(ctx, *c, x, y, color);

        c++;
        x += 10;
    }
}

void sgfx_draw_hline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t w, uint32_t color) {
    if (w <= 0)
        return;
    if (y < 0 || y >= ctx->height)
        return;
    
    int32_t x0 = x;
    int32_t x1 = x + w;

    x0 = CLAMP(x0, 0, ctx->width);
    x1 = CLAMP(x1, 0, ctx->width);

    if (x1 - x0 <= 0)
        return;

    int32_t i = x0 + y * ctx->width;
    int32_t d = i + x1 - x0;

    while (i < d)
        ctx->framebuffer[i++] = color;
}

void sgfx_draw_vline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t h, uint32_t color) {
    if (h <= 0)
        return;
    if (x < 0 || x >= ctx->width)
        return;
    
    int32_t y0 = y;
    int32_t y1 = y + h;

    y0 = CLAMP(y0, 0, ctx->height);
    y1 = CLAMP(y1, 0, ctx->height);

    if (y1 - y0 <= 0)
        return;

    int32_t i = x + y0 * ctx->width;
    int32_t d = i + (y1 - y0) * ctx->width;

    while (i < d) {
        ctx->framebuffer[i] = color;
        i += ctx->width;
    }
}

void sgfx_pixel(GraphicsContext* context, int32_t x, int32_t y, uint32_t color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= context->width) return;
    if (y >= context->height) return;
    context->framebuffer[x + y * context->width] = color;
}

void sgfx_draw_line(GraphicsContext* context, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    int dx =  ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -ABS(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2;

    for (int i = 0; i < 1000; i++) {
        sgfx_pixel(context, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;
        
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
