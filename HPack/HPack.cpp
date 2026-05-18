// Or point at your project's
#include "Vendor/stb_image.h"
#include "Vendor/stb_image_write.h"
#include "HPack.h"
#include <thread>

namespace hpack {

    void PackingContext::SortRectanglesByHeight() {
        std::sort(_rectangles_to_pack.begin(), _rectangles_to_pack.end(), [](const Rectangle& a, const Rectangle& b) {
                return a._height > b._height;
			});
    }

    void PackingContext::PackNextRectangle() {
        Rectangle current_rectangle = _rectangles_to_pack.front();
        int current_width = current_rectangle._width;
        int current_height = current_rectangle._height;
        int best_y = INT_MAX;
        int best_x = 0;
        int best_index = 0;
        int best_waste = INT_MAX;

        for (int i = 0; i < _skyline.size(); i++) {
            int max_y = _skyline[i]._y;
            int accum_width = 0;
            int current_waste = 0;
            int j = i;

            while (j < _skyline.size() && accum_width < current_width) {
                int w = j == _skyline.size() - 1 ? _packing_width - _skyline[j]._x : _skyline[j + 1]._x - _skyline[j]._x;
                int covered = std::min(w, current_width - accum_width);
                if (_skyline[j]._y > max_y) {
                    max_y = _skyline[j]._y;
                }
                accum_width += covered;
                j++;
            }

            if (accum_width < current_width) continue;
            // if (max_y + current_height > _packing_height) continue;

            int k = i;
            int accum_width2 = 0;
            while (k < _skyline.size() && accum_width2 < current_width) {
                int w = k == _skyline.size() - 1 ? _packing_width - _skyline[k]._x : _skyline[k + 1]._x - _skyline[k]._x;
                int covered = std::min(w, current_width - accum_width2);
                current_waste += (max_y - _skyline[k]._y) * covered;
                accum_width2 += covered;
                k++;
            }

            if (max_y < best_y || (max_y == best_y && current_waste < best_waste)) {
                best_y = max_y;
                best_x = _skyline[i]._x;
                best_index = i;
                best_waste = current_waste;
            }
        }

        if (best_y == INT_MAX) HPACK_ASSERT(0 && "Could not pack rectangle");

        // Bump left edge height
        _skyline[best_index]._y = best_y + current_rectangle._height;

        // Place the rectangle
        _packed_rectangles.push_back({best_x, best_y, current_width, current_height, current_rectangle._id});
        _rectangles_to_pack.erase(_rectangles_to_pack.begin());

        if (best_index + 1 < (int)_skyline.size() &&
            _skyline[best_index]._x + current_rectangle._width != _skyline[best_index + 1]._x) {
            _skyline.push_back({ best_x + current_rectangle._width, best_y });
        }

        _skyline.erase(
            std::remove_if(_skyline.begin(), _skyline.end(), [=](const Point& p) {
                return p._x > best_x && p._x < best_x + current_rectangle._width;
                }),
            _skyline.end()
        );

        // Sort skyline
        std::sort(_skyline.begin(), _skyline.end(), [](const Point& a, const Point& b) {
            return a._x < b._x;
		});

        // Merge adjacent colinear skyline points
        // Erase guarantees relative order
        for (int i = 0; i < (int)_skyline.size() - 2; ) {
            if (_skyline[i]._y == _skyline[i + 1]._y) {
                _skyline.erase(_skyline.begin() + i + 1);
            }
            else {
                i++;
            }
        }
    }

    void WriteAtlasINI(const PackingContext& ctx) {
        // Convert packed rectangles into UVs
        HPACK_ASSERT(ctx._has_been_packed && "Attempted to get INI for unpacked context!");
    }

    void PackingContext::AddImage(const std::string& fpath, const std::string& entry_name) {
        AtlasEntry new_entry{};

        new_entry._entry_name = entry_name.c_str();
        new_entry._id = ++_ids;

        auto data = stbi_load(
                fpath.c_str(),
                &(new_entry._info._x), 
                &(new_entry._info._y),
                &(new_entry._info._nchannels), 
                4);

        HPACK_ASSERT(data && "Failed to load image using stb image!")
		new_entry._info._data = data;

        _entries.push_back(new_entry);
    }

    void PackingContext::PackAtlas() {

        HPACK_ASSERT(_entries.size() != 0 && "Attempted to pack an atlas with 0 entries!"); // Credits to Mr Lewis Greagen for correcting this spelling mistake.

        _packed_rectangles.reserve(_entries.size());
        _skyline = { {0, 0}, {_packing_width, 0} };
        for (const auto& entry : _entries) {
            Rectangle new_rect{
                ._x = 0,
                ._y = 0,
                ._width = entry._info._x,
                ._height = entry._info._y,
                ._id = entry._id
            };
            _rectangles_to_pack.push_back(new_rect);
        }
       
        SortRectanglesByHeight();

        while (!_rectangles_to_pack.empty()) {
            PackNextRectangle();
        }
        
        int max_height = -INT_MAX;
        for (const auto& rec : _packed_rectangles) {
            if (rec._height + rec._y > max_height) max_height = rec._height + rec._y;
        }
        _packing_height = max_height;

        // Allocate memory for texture
        _atlas = new unsigned char[_packing_width * _packing_height * 4]();
        // Write texture based on rectangles id --> atlasEntry

        for (const auto& rect : _packed_rectangles) {
            auto& info = _entries[rect._id - 1]._info;
            for (i32 row = 0; row < info._y; row++) {
                std::memcpy(
                    _atlas + ((rect._y + row) * _packing_width + rect._x) * 4,
                    info._data + row * info._x * 4,
                    info._x * 4
                );
            }
        }

        stbi_write_png("output.png", _packing_width, _packing_height, 4, _atlas, _packing_width * 4);
        // Free atlas
        delete[] _atlas;
    }

    void PackingContext::ClearImages() {
        for (auto& entry : _entries) {
            stbi_image_free(entry._info._data);
        }
        _entries.clear();
    }

} //namespace HPack
