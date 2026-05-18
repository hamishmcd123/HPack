#include "HPack/Hpack.h"

static hpack::PackingContext ctx;

int main() {
	ctx._packing_width = 2048;
	for (int i = 0; i < 100; i++) {
		ctx.AddImage("meme.jpg", "meme");
	}
	ctx.PackAtlas();
	ctx.ClearImages();
	
	return 0;
}