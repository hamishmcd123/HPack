# HPack

 **A simple library for creating texture atlases**
 
Example usage:
```cpp
static hpack::PackerContext ctx;

int main() {
  ctx._packing_width = 2048; // Set atlas width in pixels. Atlas height will grow.
  ctx.AddImage("my/file/path", "my_identifer");
  ctx.PackAtlas(); // This will create output.png in the build directory
  ctx.ClearImages();
  return 0;
}
```

## TODO
- Export config file that can be used at runtime to index into texture atlas.
