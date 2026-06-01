// Or point at your project's
#include "Vendor/stb_image.h"
#include "Vendor/stb_image_write.h"

#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include "HPack.h"

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
        _rectangles_to_pack.pop_front();

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

    void PackingContext::WriteAtlasINI() {
        HPACK_ASSERT(_has_been_packed && "Attempted to write INI for unpacked context!");
            
        float x_factor = 1.0f / _packing_width;
        float y_factor = 1.0f / _packing_height;

        for (const auto& rect : _packed_rectangles) {
            auto& entry = _entries[rect._id - 1];
            /*
                [0] tl
                [1] bl
                [2] br
                [3] tr
            */

            if (_flip_vertically) {
                entry._uv[1][0] = rect._x * x_factor;
                entry._uv[1][1] = (_packing_height - rect._y) * y_factor;

                entry._uv[0][0] = rect._x * x_factor;
                entry._uv[0][1] = (_packing_height - rect._y - rect._height) * y_factor;

                entry._uv[3][0] = (rect._x + rect._width) * x_factor;
                entry._uv[3][1] = (_packing_height - rect._y - rect._height) * y_factor;

                entry._uv[2][0] = (rect._x + rect._width) * x_factor;
                entry._uv[2][1] = (_packing_height - rect._y) * y_factor;
            }
            else {
				entry._uv[0][0] = rect._x * x_factor;
				entry._uv[0][1] = (_packing_height - rect._y) * y_factor;

				entry._uv[1][0] = rect._x * x_factor;
				entry._uv[1][1] = (_packing_height - rect._y - rect._height) * y_factor;

				entry._uv[2][0] = (rect._x + rect._width) * x_factor;
				entry._uv[2][1] = (_packing_height - rect._y - rect._height) * y_factor;

				entry._uv[3][0] = (rect._x + rect._width) * x_factor;
				entry._uv[3][1] = (_packing_height - rect._y) * y_factor;
			}

        }
        auto out = fopen("output.ini", "w");
        HPACK_ASSERT(out && "Failed to open output stream!");

        for (const auto& entry : _entries) { 
            fprintf(out, "[%s]\n", entry._entry_name.c_str());
            fprintf(out, "tluv=%f,%f\n", entry._uv[0][0], entry._uv[0][1]);
            fprintf(out, "bluv=%f,%f\n", entry._uv[1][0], entry._uv[1][1]);
            fprintf(out, "bruv=%f,%f\n", entry._uv[2][0], entry._uv[2][1]);
            fprintf(out, "truv=%f,%f\n", entry._uv[3][0], entry._uv[3][1]);
        }

        fclose(out);
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
        HPACK_ASSERT(_packing_width != 0 && "Remember to set the packing width!");

        _packed_rectangles.reserve(_entries.size());
        _skyline = { {0, 0}, {_packing_width, 0} };
        for (const auto& entry : _entries) {
            Rectangle new_rect{
                0,
                0,
                entry._info._x,
                entry._info._y,
                entry._id
            };
            _rectangles_to_pack.push_back(new_rect);
        }
       
        SortRectanglesByHeight();

        while (!_rectangles_to_pack.empty()) {
            PackNextRectangle();
        }
        
        int max_height = INT_MIN;
        for (const auto& rec : _packed_rectangles) {
            if (rec._height + rec._y > max_height) max_height = rec._height + rec._y;
        }
        _packing_height = max_height;

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
        if (_flip_vertically) {
            stbi_flip_vertically_on_write(1);
        }
        stbi_write_png("output.png", _packing_width, _packing_height, 4, _atlas, _packing_width * 4);
        delete[] _atlas;
        _has_been_packed = true;
    }

    void PackingContext::Clear() {
        for (auto& entry : _entries) {
            stbi_image_free(entry._info._data);
        }
        _entries.clear();
    }

    void Atlas::Create(const std::string& fpath)
    {
        // Parse config file 
        auto file = fopen(fpath.c_str(), "r");
        char line[256];
        char current_section[128];

        HPACK_ASSERT(file && "Failed to open config file!")
		while (fgets(line, sizeof(line), file) != NULL) {
            char* p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '\n' || *p == '\0' || *p == ';' || *p == '#') continue;
            
            if (*p == '[') {
                char* close = strchr(p, ']');
                if (close) {
                    size_t diff = close - p - 1;
                    strncpy(current_section, p + 1, diff);
                    current_section[diff] = '\0';
                    std::string stringified(current_section);
                    auto it = _atlas.find(stringified);
                    HPACK_ASSERT(it == _atlas.end() && "Duplicate names are not allowed in the Atlas!")
                    _atlas[stringified];
                }
                else {
                    HPACK_ASSERT(0 && "Missing closing ] from config file!")
                }
            }
            else {
                char key[128], value[128];
                char* keyend = strchr(p, '=');
                if (keyend) {
                    size_t diff = keyend - p;
                    strncpy(key, p, diff + 1);
                    key[diff] = '\0';

                    char* term = strchr(p, '\0');
                    size_t diff2 = term - keyend;
                    strncpy(value, p + diff + 1, diff2 + 1);
                    value[diff2] = '\0';
                    
                    bool succeeded = false;

                    if (strcmp(key, "tluv") == 0) {
                        char* end = nullptr;
                        float a = strtof(value, &end);
                        float b = strtof(end + 1, nullptr);
                        succeeded = true;
                        _atlas[current_section]._uvs[0][0] = a;
                        _atlas[current_section]._uvs[0][1] = b;
                    }
                    else if (strcmp(key, "bluv") == 0) {
						char* end = nullptr;
                        float a = strtof(value, &end);
                        float b = strtof(end + 1, nullptr);
                        succeeded = true;
                        _atlas[current_section]._uvs[1][0] = a;
                        _atlas[current_section]._uvs[1][1] = b;
                    }
                    else if (strcmp(key, "bruv") == 0) {
						char* end = nullptr;
                        float a = strtof(value, &end);
                        float b = strtof(end + 1, nullptr);
                        succeeded = true;
                        _atlas[current_section]._uvs[2][0] = a;
                        _atlas[current_section]._uvs[2][1] = b;
                    }
                    else if (strcmp(key, "truv") == 0) {
 						char* end = nullptr;
                        float a = strtof(value, &end);
                        float b = strtof(end + 1, nullptr);
                        succeeded = true;
                        _atlas[current_section]._uvs[3][0] = a;
                        _atlas[current_section]._uvs[3][1] = b;
                    }
                    HPACK_ASSERT(succeeded && "Unexpected key in config file!")
                }
                else {
                    HPACK_ASSERT(0 && "Malformed key-value assignment");
                }
		   }
		}

        fclose(file);
    }

} //namespace HPack
