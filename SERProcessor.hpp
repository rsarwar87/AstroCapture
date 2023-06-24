#ifndef __SER_PROC__
#define __SER_PROC__

#include <spdlog/spdlog.h>
#include <climits>
#include <time.h>
#include <chrono>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#define NANOSEC_PER_SEC 1000000000
#define MICROSEC_PER_SEC 1000000
#define TIMEUNITS_PER_SEC (NANOSEC_PER_SEC / 100)
#define SECS_UNTIL_UNIXTIME 62135596800
#define PACKED_STRUCT
#define SER_FILE_ID "LUCAM-RECORDER"


namespace SER {
#pragma pack(push, 1)
typedef struct PACKED_STRUCT {
  char sFileID[14];
  uint32_t uiLuID;
  uint32_t uiColorID;
  /* WARN: For some reason, uiLittleEndian is used in the opposite meanning,
   * so that the image data byte order is big-endian when uiLittleEndian is
   * 1, and little-endian when uiLittleEndian is 0.
   * By default, SERUtils follows this behaviour in order to avoid breaking
   * compatibility with these old softwares.
   * Anyway you can revert this behaviour (so that uiLittleEndian = 1 really
   * means little-endian), by setting invert_endianness to 1.
   * For more info, see:
   * https://free-astro.org/index.php/SER#Specification_issue_with_endianness
   */
  uint32_t uiLittleEndian;
  uint32_t uiImageWidth;
  uint32_t uiImageHeight;
  uint32_t uiPixelDepth;
  uint32_t uiFrameCount;
  char sObserver[40];
  char sInstrument[40];
  char sTelescope[40];
  uint64_t ulDateTime;
  uint64_t ulDateTime_UTC;
} SERHeader;
#pragma pack(pop)
typedef union {
  uint8_t int8;
  uint16_t int16;
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  } rgb8;
  struct {
    uint16_t r;
    uint16_t g;
    uint16_t b;
  } rgb16;
} SERPixelValue;

typedef struct {
  uint32_t id;
  uint32_t index;
  uint64_t datetime;
  time_t unixtime;
  uint32_t littleEndian;
  uint32_t pixelDepth;
  uint32_t colorID;
  uint32_t width;
  uint32_t height;
  size_t size;
  std::unique_ptr<uint8_t[]>
      buffer;  // using a smart pointer is safer (and we don't
} SERFrame;

enum Endianness { LittleEndian = 0, BigEndian = 1 };
enum BAYER {
  /* Monochromatic (one channel) formats */
  COLOR_MONO = 0,
  COLOR_BAYER_RGGB = 8,
  COLOR_BAYER_GRBG = 9,
  COLOR_BAYER_GBRG = 10,
  COLOR_BAYER_BGGR = 11,
  COLOR_BAYER_CYYM = 16,
  COLOR_BAYER_YCMY = 17,
  COLOR_BAYER_YMCY = 18,
  COLOR_BAYER_MYYC = 19,
  /* Color (three channels) formats */
  COLOR_RGB = 100,
  COLOR_BGR = 101

};

class SERBase {
 public:
  SERBase(bool invert = false)
      : is_sysbig_endian(
            (*(uint16_t *)"\0\xff" <
             0x100)),  // std::endian::native == std::endian::little) {
        invert_endianness(invert) {
    header = std::make_unique<SERHeader>();
  }

  ~SERBase() {}

 protected:
  std::string fn;
  std::unique_ptr<SERHeader> header;
  const bool is_sysbig_endian;
  bool invert_endianness;

  void swapEndiannessHeader() {
    swap_endian<uint32_t>(&header->uiLuID);
    swap_endian<uint32_t>(&header->uiColorID);
    swap_endian<uint32_t>(&header->uiLittleEndian);
    swap_endian<uint32_t>(&header->uiImageWidth);
    swap_endian<uint32_t>(&header->uiImageHeight);
    swap_endian<uint32_t>(&header->uiPixelDepth);
    swap_endian<uint32_t>(&header->uiFrameCount);
    swap_endian<uint64_t>(&header->ulDateTime);
    swap_endian<uint64_t>(&header->ulDateTime_UTC);
  }
  template <typename T>
  void swap_endian(T *u) {
    if (invert_endianness) {
      return;
    }
    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
      T u;
      unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = *u;

    for (size_t k = 0; k < sizeof(T); k++)
      dest.u8[k] = source.u8[sizeof(T) - k - 1];

    *u = dest.u;
  }
  int SERGetNumberOfPlanes() {
    uint32_t color = header->uiColorID;
    if (color >= COLOR_RGB) return 3;
    return 1;
  }

  size_t SERGetBytesPerPixel() {
    uint32_t depth = header->uiPixelDepth;
    if (depth < 1) return 0;
    int planes = SERGetNumberOfPlanes();
    if (depth <= 8)
      return planes;
    else
      return (2 * planes);
  }
  size_t SERGetTrailerOffset() {
    uint32_t frame_idx = header->uiFrameCount;
    return SERGetFrameOffset(frame_idx);
  }
  size_t SERGetFrameOffset(size_t frame_idx) {
    return sizeof(SERHeader) + (frame_idx * SERGetFrameSize());
  }
  size_t SERGetFrameSize() {
    return header->uiImageWidth * header->uiImageHeight * SERGetBytesPerPixel();
  }

  auto SERGetColor() {
    std::string str = "UNKOWN";
    switch (header->uiColorID) {
      case COLOR_MONO:
        str = "MONO";
        break;
      case COLOR_BAYER_RGGB:
        str = "RGGB";
        break;
      case COLOR_BAYER_GRBG:
        str = "GRBG";
        break;
      case COLOR_BAYER_GBRG:
        str = "GBRG";
        break;
      case COLOR_BAYER_BGGR:
        str = "BGGR";
        break;
      case COLOR_BAYER_CYYM:
        str = "CYYM";
        break;
      case COLOR_BAYER_YCMY:
        str = "YCMY";
        break;
      case COLOR_BAYER_YMCY:
        str = "YMCY";
        break;
      case COLOR_BAYER_MYYC:
        str = "MYYC";
        break;
      case COLOR_RGB:
        str = "RGB";
        break;
      case COLOR_BGR:
        str = "BGR";
        break;
    }
    return std::make_tuple(str, header->uiColorID);
  }
  uint64_t SERFrameTimeToMicroSec2(uint64_t video_t) {
    double elapsed_sec = video_t / (double)TIMEUNITS_PER_SEC;
    return (uint64_t)(elapsed_sec * MICROSEC_PER_SEC) % MICROSEC_PER_SEC;
  }
  uint64_t SERUnixTimeToVediotime(uint64_t video_t) {
    return reinterpret_cast<uint64_t>(TIMEUNITS_PER_SEC *
                                      (video_t + SECS_UNTIL_UNIXTIME));
  }
  uint64_t SERVideoTimeToUnixtime(uint64_t video_t) {
    double elapsed_sec = video_t / (double)TIMEUNITS_PER_SEC;
    return (uint64_t)elapsed_sec - SECS_UNTIL_UNIXTIME;
  }
  bool SERIsBigEndian() { return header->uiLittleEndian == 1; }
  void print_header() {
    spdlog::info("{}: {} {}", __func__, "sFileID", header->sFileID);
    spdlog::info("{}: {} {}", __func__, "uiLuID", header->uiLuID);
    spdlog::info("{}: {} {} {}", __func__, "uiColorID",
                 std::get<0>(SERGetColor()), header->uiColorID);
    spdlog::info("{}: {} {}", __func__, "uiLittleEndian",
                 header->uiLittleEndian);
    spdlog::info("{}: {} {}", __func__, "uiImageWidth", header->uiImageWidth);
    spdlog::info("{}: {} {}", __func__, "uiImageHeight", header->uiImageHeight);
    spdlog::info("{}: {} {}", __func__, "uiFrameCount", header->uiFrameCount);
    spdlog::info("{}: {} {}", __func__, "uiPixelDepth", header->uiPixelDepth);
    spdlog::info("{}: {} {}", __func__, "sObserver", header->sObserver);
    spdlog::info("{}: {} {}", __func__, "sInstrument", header->sInstrument);
    {
      time_t realtime = (time_t)SERVideoTimeToUnixtime(((header->ulDateTime)));
      spdlog::info("{}: {} {} {}", __func__, "ulDateTime", header->ulDateTime,
                   ctime(&realtime));
    }
    {
      time_t realtime =
          (time_t)SERVideoTimeToUnixtime(((header->ulDateTime_UTC)));
      spdlog::info("{}: {} {} {}", __func__, "ulDateTime_UTC",
                   header->ulDateTime_UTC, ctime(&realtime));
    }
  }
};  // namespace SER

class SERReader : public SERBase {
 public:
  SERReader(std::string _fn) {
    std::ios_base::sync_with_stdio(false);
    cFrame = std::make_unique<SERFrame>();
    fn = _fn;
    fd = std::fstream(fn.c_str(), std::ios::in | std::ios::binary);
    if (!fd.is_open()) {
      spdlog::critical("{}: {} could not be opened", __func__, fn);
      return;
    }
    fd.seekg(0, std::ios::end);
    filesize = fd.tellg();
    spdlog::info("{}: {} was openned, {} size.", __func__, fn, filesize);
    fd.seekg(0, std::ios::beg);
  };
  ~SERReader(){};

  bool SERHasTrailer() {
    spdlog::info("{}: {}", __func__, filesize > (size_t)SERGetTrailerOffset());
    return filesize > (size_t)SERGetTrailerOffset();
  }
  bool GetFrame(uint32_t frame_idx) {
    // assert(header.get() != nullptr);
    if (frame_idx >= header->uiFrameCount) {
      spdlog::critical(
          "{}: Frame idx {} is greater than total number of frames available "
          "{}",
          __func__, frame_idx, header->sFileID);
      return false;
    }
    if (cFrame == nullptr) {
      spdlog::critical("{}: frame struct not initialized", __func__);
      return false;
    }
    size_t sz = SERGetFrameSize();
    size_t offset_start = sizeof(SERHeader) + (frame_idx * sz);
    if (filesize < offset_start) {
      spdlog::critical("{}: Frame offset {} is greater than total size {}",
                       __func__, offset_start, filesize);
      return false;
    } else if (offset_start + sz > filesize) {
      spdlog::critical(
          "{}: Frame end position {} is greater than total size {}", __func__,
          offset_start + sz, filesize);
      return false;
    }
    cFrame->id = frame_idx + 1;
    cFrame->index = frame_idx;
    cFrame->datetime = SERGetFrameDate(frame_idx);
    if (cFrame->datetime > 0)
      cFrame->unixtime = SERVideoTimeToUnixtime(cFrame->datetime);
    else
      cFrame->unixtime = 0;
    if (cFrame->buffer.get() != nullptr) cFrame->buffer.reset();
    cFrame->buffer = std::unique_ptr<uint8_t[]>(new uint8_t[sz]);
    fd.seekg(offset_start, std::ios::beg);
    fd.read((char *)(cFrame->buffer.get()), sz);
    spdlog::info("{}: reading frame @ {} len {}", __func__, offset_start, sz);
    return true;
  }

  bool SEROpenMovie() {
    if (!read_header()) {
      spdlog::critical("{}: Failed to read header", __func__);
      return false;
    }
    if (strcmp(SER_FILE_ID, header->sFileID) != 0) {
      spdlog::critical("{}: invalid sFileID {}", __func__, header->sFileID);
      return false;
    }
    uint32_t frame_c = header->uiFrameCount;
    size_t trailer_offset = SERGetTrailerOffset();
    if (filesize < trailer_offset) {
      header->uiFrameCount = (filesize - sizeof(SERHeader)) / SERGetFrameSize();
      spdlog::warn(
          "{}: Incomplete file {}/{}, only allowing as file size would allow "
          "{}",
          __func__, trailer_offset, filesize, header->uiFrameCount);
      // incomplete file
    } else if (filesize == trailer_offset) {
      spdlog::warn("{}: missing tail info {}/{}", __func__, trailer_offset,
                   filesize);
      // missing taile
    } else if (filesize - trailer_offset < (frame_c * sizeof(uint64_t))) {
      spdlog::error("{}: incomplete tail info {}/{}", __func__,
                    filesize - trailer_offset, (frame_c * sizeof(uint64_t)));
      //      movie->warnings |= WARN_INCOMPLETE_TRAILER;
    }
    firstFrameDate = SERGetFirstFrameDate();
    lastFrameDate = SERGetLastFrameDate();
    if (lastFrameDate > firstFrameDate) {
      duration = lastFrameDate - firstFrameDate;
      duration /= TIMEUNITS_PER_SEC;
    } else {
      spdlog::error("{}: bad timestamp info {}/{}", __func__, firstFrameDate,
                    lastFrameDate);
      // bad date time
    }
    return true;
  }

 private:
  uint64_t firstFrameDate, lastFrameDate, duration;
  std::fstream fd;
  size_t filesize;
  std::unique_ptr<SERFrame> cFrame;
  bool read_header() {
    fd.seekg(0, std::ios::beg);
    read<SERHeader>(header.get(), 1);
    if (is_sysbig_endian) swapEndiannessHeader();
    spdlog::info("{}: {} {}", __func__, is_sysbig_endian, sizeof(SERHeader));
    cFrame->littleEndian = header->uiLittleEndian;
    cFrame->pixelDepth = header->uiPixelDepth;
    cFrame->colorID = header->uiColorID;
    cFrame->width = header->uiImageWidth;
    cFrame->height = header->uiImageHeight;
    cFrame->size = SERGetFrameSize();
    print_header();
    /*printf("Read %lu header bytes\n\n", totread);*/
    return 1;
  }
  uint64_t SERFrameTimeToMicroSec(size_t frame_idx) {
    auto video_t = SERGetFrameDate(frame_idx);
    double elapsed_sec = video_t / (double)TIMEUNITS_PER_SEC;
    return (uint64_t)(elapsed_sec * MICROSEC_PER_SEC) % MICROSEC_PER_SEC;
  }
  uint64_t SERGetFirstFrameDate() { return SERGetFrameDate(0); }
  uint64_t SERGetLastFrameDate() {
    return SERGetFrameDate(header->uiFrameCount - 1);
  }

  template <typename T>
  void read(T *data, std::size_t len) {
    fd.read((char *)&data[0], len * sizeof(T));
  }

  uint64_t SERGetFrameDate(size_t idx) {
    if (!SERHasTrailer()) {
      spdlog::warn("{}: no tail found", __func__);
      return 0;
    }
    uint64_t date = 0;
    if (idx >= header->uiFrameCount) {
      spdlog::warn(
          "{}: index requested {} is greater than the total number of frames "
          "{}",
          __func__, idx, header->uiFrameCount);
      idx = header->uiFrameCount - 1;
    }
    size_t offset = SERGetTrailerOffset();
    offset += (idx * sizeof(uint64_t));
    char *ptr = (char *)&date;
    spdlog::info("{}: seeking to {} ", __func__, offset);
    fd.seekg(offset, std::ios::beg);
    fd.read(ptr, sizeof(uint64_t));

    if (is_sysbig_endian && date > 0) swap_endian<uint64_t>(&date);
    return date;
  }
};

class SERWriter : public SERBase {
 public:
  SERWriter(std::string _fn) {
    std::ios_base::sync_with_stdio(false);
    fn = _fn;
    fd = std::fstream(fn.c_str(), std::ios::out | std::ios::binary);
    is_prepared = false;
  };
  ~SERWriter() {
    spdlog::info("closing writter for: {}", fn);
    close();
  };
  void prepare_header(std::array<uint32_t, 2> dim,
                      std::array<std::string, 3> str, uint8_t nbytes,
                      BAYER bay = COLOR_MONO) {
    std::memcpy(header->sFileID, "LUCAM-RECORDER", sizeof(header->sFileID));
    header->uiLuID = 0;
    header->uiColorID = bay;
    header->uiLittleEndian = is_sysbig_endian ? 1 : 0;
    header->uiImageWidth = dim[0];
    header->uiImageHeight = dim[1];
    header->uiPixelDepth = nbytes * SERGetNumberOfPlanes() * 8;
    header->uiFrameCount = 0;
    str[0].copy(&(header->sObserver[40]),
                str[0].length() > 40 ? 40 : str[0].length());
    str[1].copy(&(header->sInstrument[40]),
                str[1].length() > 40 ? 40 : str[1].length());
    str[2].copy(&(header->sTelescope[40]),
                str[2].length() > 40 ? 40 : str[2].length());
    auto const now = std::chrono::system_clock::now();
    header->ulDateTime =
        SERUnixTimeToVediotime(std::chrono::system_clock::to_time_t(now));
    header->ulDateTime_UTC = header->ulDateTime;

    is_prepared = true;
    write_header();
    sz = SERGetFrameSize();
    spdlog::info("Created file: {}, each frame is {} bytes", fn, sz);

    print_header();
  }
  void write_frame(uint8_t *data) {
    fd.seekg(0, std::ios::end);
    header->uiFrameCount++;
    write<uint8_t>(data, sz);
    auto const now = std::chrono::system_clock::now();
    timestamp.push_back(
        SERUnixTimeToVediotime(std::chrono::system_clock::to_time_t(now)));
  }
  void close() {
    if (header->uiFrameCount > 0) {
      write_header();
      write_tail();
    }
    fd.flush();
    fd.sync();
    spdlog::info("Closing file: {}: {} bytes written", fn, fd.tellg());
    fd.close();
  }

 private:
  std::fstream fd;
  bool is_prepared;
  size_t sz = 0;
  std::vector<uint64_t> timestamp;

  template <typename T>
  void write(T *data, std::size_t bytes) {
    if (!is_prepared) spdlog::error("{}: header not initialized", __func__);
    fd.write((char *)&data[0], bytes * sizeof(T));
  }
  void write_header() {
    if (is_sysbig_endian) swapEndiannessHeader();
    fd.seekg(0, std::ios::beg);
    write<SERHeader>(header.get(), 1);
    fd.seekg(0, std::ios::end);
    if (is_sysbig_endian) swapEndiannessHeader();
  }
  void write_tail() {
    fd.seekg(0, std::ios::end);
    write<uint64_t>(timestamp.data(), timestamp.size());
  }
};
}  // namespace SER

#endif
