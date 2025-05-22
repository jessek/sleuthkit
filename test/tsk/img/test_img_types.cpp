#include "tsk/img/img_types.h"
#include "catch.hpp"

TEST_CASE("tsk_img_type_toid returns correct ID", "[img_types]") {
  SECTION("UTF-8: known format: raw") {
      TSK_IMG_TYPE_ENUM type = tsk_img_type_toid_utf8("raw");
      REQUIRE(type == TSK_IMG_TYPE_RAW);
  }

  SECTION("UTF-8: Unknown format") {
      TSK_IMG_TYPE_ENUM type = tsk_img_type_toid_utf8("not_a_format");
      REQUIRE(type == TSK_IMG_TYPE_UNSUPP);
  }

  SECTION("UTF-8: Null input returns unsupported type") {
    REQUIRE(tsk_img_type_toid_utf8(nullptr) == TSK_IMG_TYPE_UNSUPP);
  }

  SECTION("NON UTF-8: raw returns raw type") {
    REQUIRE(tsk_img_type_toid("raw") == TSK_IMG_TYPE_RAW);
  }

  SECTION("NON UTF-8: null input returns unsupported type") {
    REQUIRE(tsk_img_type_toid(nullptr) == TSK_IMG_TYPE_UNSUPP);
  }

  SECTION("NON UTF-8: unknown input returns unsupported type") {
    REQUIRE(tsk_img_type_toid("unknown") == TSK_IMG_TYPE_UNSUPP);
  }

#if HAVE_LIBEWF
  SECTION("Known format: ewf (if enabled)") {
      TSK_IMG_TYPE_ENUM type = tsk_img_type_toid_utf8("ewf");
      REQUIRE(type == TSK_IMG_TYPE_EWF_EWF);
  }
#endif
}

TEST_CASE("tsk_img_type_print outputs expected content", "[img_types]") {
  // Create a temporary in-memory file
  FILE* tmp = tmpfile();
  REQUIRE(tmp != nullptr);

  // Call the function to print into the file
  tsk_img_type_print(tmp);
  fflush(tmp);               // flush stdout buffers
  fseek(tmp, 0, SEEK_SET);   // rewind to start
  // Read file contents into a buffer
  char buffer[4096] = {0};
  fread(buffer, 1, sizeof(buffer) - 1, tmp);
  fclose(tmp);

  // Convert to std::string for assertion
  std::string output(buffer);

  // Check some expected content
  REQUIRE(output.find("Supported image format types:") != std::string::npos);
  REQUIRE(output.find("raw") != std::string::npos);
  REQUIRE(output.find("Single or split raw file") != std::string::npos);
  #if HAVE_LIBEWF
  SECTION("Known format: ewf (if enabled)") {
    REQUIRE(output.find("ewf") != std::string::npos);
  }
  #endif
}

TEST_CASE("checks that tsk_img_type_toname returns expected name") {
  TSK_IMG_TYPE_ENUM raw = TSK_IMG_TYPE_RAW;
  REQUIRE(!strcmp(tsk_img_type_toname(raw), "raw"));
  TSK_IMG_TYPE_ENUM unsupp = TSK_IMG_TYPE_UNSUPP;
  REQUIRE(tsk_img_type_toname(unsupp) == nullptr);
  #if HAVE_LIBEWF
  SECTION("Known format: ewf (if enabled)") {
    TSK_IMG_TYPE_ENUM ewf = TSK_IMG_TYPE_EWF_EWF;
    REQUIRE(!strcmp(tsk_img_type_toname(ewf), "ewf"));
  }
  #endif

}

TEST_CASE("checks that tsk_img_type_todesc returns expected description") {
  TSK_IMG_TYPE_ENUM raw = TSK_IMG_TYPE_RAW;
  REQUIRE(!strcmp(tsk_img_type_todesc(raw), "Single or split raw file (dd)"));
  TSK_IMG_TYPE_ENUM unsupp = TSK_IMG_TYPE_UNSUPP;
  REQUIRE(tsk_img_type_todesc(unsupp) == nullptr);
  #if HAVE_LIBEWF
  SECTION("Known format: ewf (if enabled)") {
    TSK_IMG_TYPE_ENUM ewf = TSK_IMG_TYPE_EWF_EWF;
    REQUIRE(!strcmp(tsk_img_type_todesc(ewf), "Expert Witness Format (EnCase)"));
  }
  #endif
}

TEST_CASE("checks that tsk_img_type_supported returns expected output") {
  SECTION("check that supported file types are supported") {
    REQUIRE(tsk_img_type_supported() == TSK_IMG_TYPE_RAW);
  }
  #if HAVE_LIBEWF
  SECTION("Known format: ewf (if enabled)") {
    REQUIRE(tsk_img_type_supported() == TSK_IMG_TYPE_EWF_EWF);
  }
  #endif
  SECTION("check that unsupported file types not supported") {
    REQUIRE(tsk_img_type_supported() != TSK_IMG_TYPE_UNSUPP);
  }
}

