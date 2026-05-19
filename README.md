# HPack

 **A simple library for creating texture atlases**
 
 Made for my AIE Complex Games unit. 

 Uses stb_image and stb_image_write.
 
# Example usage:

**Creating a texture atlas**
```cpp
static hpack::PackingContext ctx;

// We can change if the atlas is flipped on export by redefining FLIP_VERTICALLY in HPack.h

int main() {
  ctx._packing_width = 2048; // Set atlas width in pixels. Atlas height will grow.
  ctx.AddImage("my/file/path", "my_identifer");
  ctx.PackAtlas(); // This will create output.png in the build directory
  ctx.WriteAtlasINI(); // This will create an output.ini file containing the UVs.
  ctx.ClearImages();
  return 0;
}
```

**Reading a texture atlas**

**NOTE:** You must load the texture atlas yourself. This is just for getting the UVs.
```cpp
static hpack::Atlas my_atlas;
my_atlas.Create("file/path/to/config");

// We can access the atlas through the _atlas member, which is just an unordered map.
my_atlas._atlas["ExampleID"]._uvs[0][0] // Top left x

/*
_uvs[0] --> Top left
_uvs[1] --> Bottom left
_uvs[2] --> Bottom right
_uvs[3] --> Top right
*/
```
