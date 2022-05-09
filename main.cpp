#include "main.h"

#define EXTENSION_JPEG ".jpg"

/*
* @param name_file Имя файла
* @param img_size Размер изображение(first - ширина, second - высота)
* @param buff Буффер с изображением
*/
void safeToJpeg(const char* name_file, const std::pair<std::uint32_t, std::uint32_t> &img_size, unsigned char** buff) {
  jpeg_compress_struct info;
  jpeg_error_mgr err;

  info.err = jpeg_std_error(&err);
  jpeg_create_compress(&info);

  auto file = fopen(name_file, "wb");
  jpeg_stdio_dest(&info, file);

  info.image_width = img_size.first;
  info.image_height = img_size.second;
  info.input_components = 4;
  info.in_color_space = J_COLOR_SPACE::JCS_EXT_RGBX;
  jpeg_set_defaults(&info);

  jpeg_start_compress(&info, TRUE);

  while (info.next_scanline < info.image_height) {
    /*unsigned char bf[info.image_width * 3];
    for (size_t i{}; i < info.image_width; i++) {
      memcpy(&bf[i * 3], &buff[info.next_scanline][i*4], 3);
    }
    JSAMPROW row_pointer = bf;
    jpeg_write_scanlines(&info, &row_pointer, 1);*/

    jpeg_write_scanlines(&info, buff, info.image_height);
  }
    

  jpeg_finish_compress(&info);
  jpeg_destroy_compress(&info);
  fclose(file);
}

void loadFromPng(const char* name_file, std::pair<std::uint32_t, std::uint32_t> *img_size, unsigned char** &buff) {

  auto png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  auto info = png_create_info_struct(png);
  setjmp(png_jmpbuf(png));

  auto file = fopen(name_file, "rb");
  png_init_io(png, file);
  png_read_info(png, info);

  img_size->first = png_get_image_width(png, info);
  img_size->second = png_get_image_height(png, info);

  {
    auto color_type = png_get_color_type(png, info);
    auto bit_depth = png_get_bit_depth(png, info);

    /* Read any color_type into 8bit depth, RGBA format. */
    /* See http://www.libpng.org/pub/png/libpng-manual.txt */
    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    /* PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth. */
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    /* These color_type don't have an alpha channel then fill it with 0xff. */
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);
  }

  png_read_update_info(png, info);

  buff = new unsigned char*[img_size->second];
  for (size_t y{}; y < img_size->second; y++) {
    buff[y] = new unsigned char[png_get_rowbytes(png,info)];
  }
  png_read_image(png, buff);

  png_destroy_read_struct(&png, &info, nullptr);
  fclose(file);
}

inline void keybord_wait() {
  #ifdef _WIN32
  system("pause");
  #elif __linux__
  std::cout << "Press the ENTER button to continue..." << std::endl;
  while(getchar() != '\n') {}
  #else
  #error "No support this platform"
  #endif
}

size_t g_nCountConvert{};
void convert(const std::filesystem::path& path) {
  std::pair<std::uint32_t, std::uint32_t> img_size;

  // static size_t count{};
  unsigned char** bf;
  std::string sPath = path.string();
  std::cout << "[" << ++g_nCountConvert << "]: " << sPath << " ... ";
  loadFromPng(sPath.c_str(), &img_size, bf);

  sPath.replace(sPath.size() - 4, sizeof(EXTENSION_JPEG) - 1, EXTENSION_JPEG);
  safeToJpeg(sPath.c_str(), img_size, bf);
  std::cout << sPath << std::endl;

  for (size_t i{}; i < img_size.second; ++i)
    delete[] bf[i];
  delete[] bf;
}

void findProc(const std::filesystem::path& path) {
  for (auto &it : std::filesystem::directory_iterator(path)) {
    auto ph = it.path();
    if (ph.extension() == ".png") {
      convert(ph);
    }
    else if (it.is_directory()) {
      findProc(ph);
    }
  }
}

int main(int argc, char* argv[]) {

  try {
    if (argc < 2) throw std::runtime_error("Arguments count < 2");

    namespace fs = std::filesystem;
    auto path = fs::path(argv[1]);

    if (!fs::exists(path)) {
      throw std::runtime_error("Target object is not exists");
    } 
    else if (path.extension() == ".png") {
      convert(path);
    } 
    else if (fs::is_directory(path)) {
      findProc(path);
      if (!g_nCountConvert) throw std::runtime_error("No searched PNG files in this directory");
    } 
    else {
      throw std::runtime_error("Target object is not PNG file or directory");
    }
    keybord_wait();
  }
  catch (const std::runtime_error& err) {
    std::cerr << "ERROR: " << err.what() << std::endl;
    keybord_wait();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}