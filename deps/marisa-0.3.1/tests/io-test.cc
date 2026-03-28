#ifdef _MSC_VER
 #include <io.h>
#else
 #include <unistd.h>
#endif  // _MSC_VER

#include <fcntl.h>
#include <marisa/grimoire/io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <exception>
#include <sstream>
#include <stdexcept>

#include "marisa-assert.h"

namespace {

void TestFilename() {
  TEST_START();

  {
    marisa::grimoire::Writer writer;
    writer.open("io-test.dat");

    writer.write(std::uint32_t{123});
    writer.write(std::uint32_t{234});

    double values[] = {3.45, 4.56};
    writer.write(values, 2);

    EXCEPT(writer.write(values, SIZE_MAX), std::invalid_argument);
  }

  {
    marisa::grimoire::Reader reader;
    reader.open("io-test.dat");

    std::uint32_t value;
    reader.read(&value);
    ASSERT(value == 123);
    reader.read(&value);
    ASSERT(value == 234);

    double values[2];
    reader.read(values, 2);
    ASSERT(values[0] == 3.45);
    ASSERT(values[1] == 4.56);

    char byte;
    EXCEPT(reader.read(&byte), std::runtime_error);
  }

  {
    marisa::grimoire::Mapper mapper;
    mapper.open("io-test.dat");

    std::uint32_t value;
    mapper.map(&value);
    ASSERT(value == 123);
    mapper.map(&value);
    ASSERT(value == 234);

    const double *values;
    mapper.map(&values, 2);
    ASSERT(values[0] == 3.45);
    ASSERT(values[1] == 4.56);

    char byte;
    EXCEPT(mapper.map(&byte), std::runtime_error);
  }

  {
    marisa::grimoire::Mapper mapper;
    mapper.open("io-test.dat", MARISA_MAP_POPULATE);

    std::uint32_t value;
    mapper.map(&value);
    ASSERT(value == 123);
    mapper.map(&value);
    ASSERT(value == 234);

    const double *values;
    mapper.map(&values, 2);
    ASSERT(values[0] == 3.45);
    ASSERT(values[1] == 4.56);

    char byte;
    EXCEPT(mapper.map(&byte), std::runtime_error);
  }

  {
    marisa::grimoire::Writer writer;
    writer.open("io-test.dat");
  }

  {
    marisa::grimoire::Reader reader;
    reader.open("io-test.dat");

    char byte;
    EXCEPT(reader.read(&byte), std::runtime_error);
  }

  TEST_END();
}

void TestFd() {
  TEST_START();

  {
#ifdef _MSC_VER
    int fd = -1;
    ASSERT(::_sopen_s(&fd, "io-test.dat",
                      _O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC, _SH_DENYRW,
                      _S_IREAD | _S_IWRITE) == 0);
#else   // _MSC_VER
    int fd = ::creat("io-test.dat", 0644);
    ASSERT(fd != -1);
#endif  // _MSC_VER
    marisa::grimoire::Writer writer;
    writer.open(fd);

    std::uint32_t value = 234;
    writer.write(value);

    double values[] = {34.5, 67.8};
    writer.write(values, 2);

#ifdef _MSC_VER
    ASSERT(::_close(fd) == 0);
#else   // _MSC_VER
    ASSERT(::close(fd) == 0);
#endif  // _MSC_VER
  }

  {
#ifdef _MSC_VER
    int fd = -1;
    ASSERT(::_sopen_s(&fd, "io-test.dat", _O_BINARY | _O_RDONLY, _SH_DENYRW,
                      _S_IREAD) == 0);
#else   // _MSC_VER
    int fd = ::open("io-test.dat", O_RDONLY);
    ASSERT(fd != -1);
#endif  // _MSC_VER
    marisa::grimoire::Reader reader;
    reader.open(fd);

    std::uint32_t value;
    reader.read(&value);
    ASSERT(value == 234);

    double values[2];
    reader.read(values, 2);
    ASSERT(values[0] == 34.5);
    ASSERT(values[1] == 67.8);

    char byte;
    EXCEPT(reader.read(&byte), std::runtime_error);

#ifdef _MSC_VER
    ASSERT(::_close(fd) == 0);
#else   // _MSC_VER
    ASSERT(::close(fd) == 0);
#endif  // _MSC_VER
  }

  TEST_END();
}

void TestFile() {
  TEST_START();

  {
#ifdef _MSC_VER
    FILE *file = nullptr;
    ASSERT(::fopen_s(&file, "io-test.dat", "wb") == 0);
#else   // _MSC_VER
    FILE *file = std::fopen("io-test.dat", "wb");
    ASSERT(file != nullptr);
#endif  // _MSC_VER
    marisa::grimoire::Writer writer;
    writer.open(file);

    std::uint32_t value = 10;
    writer.write(value);

    double values[2] = {0.1, 0.2};
    writer.write(values, 2);

    ASSERT(std::fclose(file) == 0);
  }

  {
#ifdef _MSC_VER
    FILE *file = nullptr;
    ASSERT(::fopen_s(&file, "io-test.dat", "rb") == 0);
#else   // _MSC_VER
    FILE *file = std::fopen("io-test.dat", "rb");
    ASSERT(file != nullptr);
#endif  // _MSC_VER
    marisa::grimoire::Reader reader;
    reader.open(file);

    std::uint32_t value;
    reader.read(&value);
    ASSERT(value == 10);

    double values[2];
    reader.read(values, 2);
    ASSERT(values[0] == 0.1);
    ASSERT(values[1] == 0.2);

    char byte;
    EXCEPT(reader.read(&byte), std::runtime_error);

    ASSERT(std::fclose(file) == 0);
  }

  TEST_END();
}

void TestStream() {
  TEST_START();

  std::stringstream stream;

  {
    marisa::grimoire::Writer writer;
    writer.open(stream);

    std::uint32_t value = 12;
    writer.write(value);

    double values[2] = {3.4, 5.6};
    writer.write(values, 2);
  }

  {
    marisa::grimoire::Reader reader;
    reader.open(stream);

    std::uint32_t value;
    reader.read(&value);
    ASSERT(value == 12);

    double values[2];
    reader.read(values, 2);
    ASSERT(values[0] == 3.4);
    ASSERT(values[1] == 5.6);

    char byte;
    EXCEPT(reader.read(&byte), std::runtime_error);
  }

  TEST_END();
}

}  // namespace

int main() try {
  TestFilename();
  TestFd();
  TestFile();
  TestStream();

  return 0;
} catch (const std::exception &ex) {
  std::cerr << ex.what() << "\n";
  throw;
}
