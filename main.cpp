#include "HPack/HPack.h"
#include <iostream>

static hpack::PackingContext ctx;

int main() {
	ctx._packing_width = 1024;
	ctx.AddImage("ele/A.png", "A");
	ctx.AddImage("ele/B.png", "B");
	ctx.AddImage("ele/C.png", "C");
	ctx.AddImage("ele/S.png", "S");
	ctx.AddImage("ele/button.png", "button7");
	ctx.AddImage("ele/options.png", "options");
	ctx.AddImage("ele/play.png", "play");
	ctx.AddImage("ele/quit.png", "quit");
	ctx.AddImage("ele/unpause.png", "unpause");
	ctx.AddImage("ele/pause.png", "pause");
	ctx.PackAtlas();
	ctx.ClearImages();
	std::cout << "Yippee!!";
	return 0;
}
