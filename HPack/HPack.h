#pragma once
#include <algorithm>
#include <vector>
#include <string>

namespace hpack {
    typedef int32_t i32;

#ifndef HPACK_ASSERT
#include <cassert>
#define HPACK_ASSERT(_EXPR) assert(_EXPR);
#endif
#define HPACK_UNUSED(x) ((void)(x));


    struct ImageInfo {
        unsigned char* _data;
        int _x, _y, _nchannels;
    };

    struct AtlasEntry {
        const char* _entry_name;
        ImageInfo _info;
        i32 _uv[4][2]; // Texcoords
        i32 _id;
    };

    struct Rectangle {
        i32 _x, _y;
        i32 _width, _height;
        i32 _id;
    };

    struct Point {
        i32 _x, _y;
    };

    class PackingContext {
        public:
            void AddImage(const std::string& fpath, const std::string& entry_name);
            void PackAtlas();
            void ClearImages();
            bool _has_been_packed = false;
            i32 _packing_width = 0;
            i32 _packing_height = 0;
        private:
            i32 _ids = 0;
            std::vector<Rectangle> _rectangles_to_pack{};
            std::vector<Rectangle> _packed_rectangles{};
            std::vector<Point> _skyline{};
            std::vector<AtlasEntry> _entries{};
            void PackNextRectangle();
            void SortRectanglesByHeight();
            unsigned char* _atlas;
    };

    void WriteAtlasINI(const PackingContext&);

} //namespace hpack


