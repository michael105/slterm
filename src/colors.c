// Color related functions
// Default colors are defined in config.h


ushort sixd_to_16bit(int x) { return x == 0 ? 0 : 0x3737 + 0x2828 * x; }

int xloadcolor(int i, const char *name, Color *ncolor) {
	XRenderColor color = {.alpha = 0xffff};

	if (!name) {
		if (BETWEEN(i, 16, 255)) {  /* 256 color */
			if (i < 6 * 6 * 6 + 16) { /* same colors as xterm */
				color.red = sixd_to_16bit(((i - 16) / 36) % 6);
				color.green = sixd_to_16bit(((i - 16) / 6) % 6);
				color.blue = sixd_to_16bit(((i - 16) / 1) % 6);
			} else { /* greyscale */
				color.red = 0x0808 + 0x0a0a * (i - (6 * 6 * 6 + 16));
				color.green = color.blue = color.red;
			}
			return XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &color, ncolor);
		} else
			name = colorname[i];
	}

	return XftColorAllocName(xw.dpy, xw.vis, xw.cmap, name, ncolor);
}

void xloadcolors(void) {
	int i;
	static int loaded;
	Color *cp;

	if (loaded) {
		for (cp = dc.col; cp < &dc.col[dc.collen]; ++cp)
			XftColorFree(xw.dpy, xw.vis, xw.cmap, cp);
	} else {
		dc.collen = MAX(LEN(colorname), 256);
		dc.col = xmalloc(dc.collen * sizeof(Color));
	}

	for (i = 0; i < dc.collen; i++)
		if (!xloadcolor(i, NULL, &dc.col[i])) {
			if (colorname[i])
				die("could not allocate color '%s'\n", colorname[i]);
			else
				die("could not allocate color %d\n", i);
		}
	loaded = 1;
}

int xsetcolorname(int x, const char *name) {
	Color ncolor;

	if (!BETWEEN(x, 0, dc.collen))
		return 1;

	if (!xloadcolor(x, name, &ncolor))
		return 1;

	XftColorFree(xw.dpy, xw.vis, xw.cmap, &dc.col[x]);
	dc.col[x] = ncolor;

	return 0;
}





