#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <cstring>

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
        std::string _entry_name;
        ImageInfo _info;
        float _uv[4][2]; // Texcoords
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
            void AddImage(const std::string&, const std::string&);
            void PackAtlas();
            void Clear();
            bool _has_been_packed = false;
            i32 _packing_width = 0;
            i32 _packing_height = 0;
			void WriteAtlasINI();
            std::vector<AtlasEntry>& GetEntries() {return _entries;}
            bool _flip_vertically = true;
        private:
            i32 _ids = 0;
            std::deque<Rectangle> _rectangles_to_pack{};
            std::vector<Point> _skyline{};
            std::vector<Rectangle> _packed_rectangles{};
            std::vector<AtlasEntry> _entries{};
            void PackNextRectangle();
            void SortRectanglesByHeight();
            unsigned char* _atlas;
    };

    class Atlas {
    public:
        void Create(const std::string&);
        struct Entry {
            float _uvs[4][2];
        };
        std::unordered_map<std::string, Entry> _atlas;
    };


} //namespace hpack


