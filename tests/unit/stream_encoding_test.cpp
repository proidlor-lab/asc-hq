/***************************************************************************
 * stream_encoding_test.cpp - Google Test suite for stream encoding/decoding
 *
 * Migrated from: source/unittests/streamencoding.cpp
 * Purpose: Test ASCII encoding/decoding and compression streams
 *
 * Part of: ASC Test Framework Migration (Phase 1)
 ***************************************************************************/

#include <gtest/gtest.h>
#include "streamencoding.h"
#include "basestrm.h"
#include <vector>
#include <cstring>

// ========== Test Fixture ==========

class StreamEncodingTest : public ::testing::Test {
  protected:
   static const int maxsize = 100000;
   std::vector<Uint8> buffer;
   int fileSize;

   void SetUp() override {
      buffer.resize(maxsize);
      fileSize = 0;

      // Try to load test file (asc2_dlg.zip)
      // If not available, create synthetic test data
      try {
         tnfilestream stream("asc2_dlg.zip", tnstream::reading);
         fileSize = stream.readdata(buffer.data(), maxsize, false);
      } catch (...) {
         // File not available, use synthetic data
         fileSize = 1000;
         for (int i = 0; i < fileSize; ++i) {
            buffer[i] = static_cast<Uint8>(i % 256);
         }
      }
   }

   void TearDown() override {
      buffer.clear();
   }
};

// ========== ASCII Encoding/Decoding Tests ==========

TEST_F(StreamEncodingTest, BasicASCIIEncoding) {
   ASCString encodedData;

   // Encode
   {
      ASCIIEncodingStream stream;
      stream.writedata(buffer.data(), fileSize);
      encodedData = stream.getResult();
   }

   EXPECT_FALSE(encodedData.empty()) << "Encoded data should not be empty";

   // Decode
   {
      std::vector<Uint8> buffer2(maxsize);
      ASCIIDecodingStream stream(encodedData);

      int size2 = stream.readdata(buffer2.data(), fileSize, true);
      EXPECT_EQ(size2, fileSize) << "Decoded size should match original size";

      // Verify byte-by-byte
      for (int i = 0; i < fileSize; ++i) {
         EXPECT_EQ(buffer2[i], buffer[i])
            << "ASCII Encoding/Decoding failed at offset " << i;
      }
   }
}

// ========== Compression Tests ==========

TEST_F(StreamEncodingTest, CompressionAndEncoding) {
   ASCString encodedData;

   // Compress and encode
   {
      ASCIIEncodingStream outerStream;
      StreamCompressionFilter stream(&outerStream);
      stream.writedata(reinterpret_cast<char*>(buffer.data()), fileSize);
      stream.close();

      encodedData = outerStream.getResult();
   }

   EXPECT_FALSE(encodedData.empty()) << "Compressed and encoded data should not be empty";

   // Decompress and decode
   {
      std::vector<char> buffer2(maxsize);

      ASCIIDecodingStream outerStream(encodedData);
      StreamDecompressionFilter stream(&outerStream);

      int size2 = stream.readdata(buffer2.data(), fileSize, true);
      EXPECT_EQ(size2, fileSize) << "Decompressed size should match original size";

      // Verify byte-by-byte
      for (int i = 0; i < fileSize; ++i) {
         EXPECT_EQ(static_cast<Uint8>(buffer2[i]), buffer[i])
            << "Compression/Decompression failed at offset " << i;
      }
   }
}

// ========== Edge Cases ==========

TEST_F(StreamEncodingTest, EmptyDataEncoding) {
   ASCString encodedData;

   {
      ASCIIEncodingStream stream;
      stream.writedata(buffer.data(), 0);
      encodedData = stream.getResult();
   }

   // Should handle empty data gracefully
   {
      std::vector<Uint8> buffer2(maxsize);
      ASCIIDecodingStream stream(encodedData);

      int size2 = stream.readdata(buffer2.data(), 0, true);
      EXPECT_EQ(size2, 0) << "Empty data should decode to zero bytes";
   }
}

TEST_F(StreamEncodingTest, SmallDataEncoding) {
   const int smallSize = 10;

   ASCString encodedData;

   {
      ASCIIEncodingStream stream;
      stream.writedata(buffer.data(), smallSize);
      encodedData = stream.getResult();
   }

   {
      std::vector<Uint8> buffer2(maxsize);
      ASCIIDecodingStream stream(encodedData);

      int size2 = stream.readdata(buffer2.data(), smallSize, true);
      EXPECT_EQ(size2, smallSize);

      for (int i = 0; i < smallSize; ++i) {
         EXPECT_EQ(buffer2[i], buffer[i]);
      }
   }
}

// ========== Main ==========

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
