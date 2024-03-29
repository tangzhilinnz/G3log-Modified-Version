/** ==========================================================================
 * 2015 by KjellKod.cc
 *
 * This code is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties.This code is yours to share, useand modify with no
 * strings attachedand no restrictions or obligations.
 * ===========================================================================
 * PUBLIC DOMAIN and Not copywrited.First published at KjellKod.cc
* **********************************************/

#pragma once

#include "g3log/filesinkhelper.hpp"
#include <gtest/gtest.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>


class RotateFileTest : public ::testing::Test {
public:
   RotateFileTest() {};
protected:
   
   // SetUp() is run immediately before a test starts.
   virtual void SetUp() {
      _filename = "g3sink_rotatefile_test";
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
      _directory = "./";
#else
      _directory = "/tmp/";
#endif
      _filesToRemove.push_back(std::string(_directory + _filename + ".log"));
   }

   // TearDown() is invoked immediately after a test finishes.
   virtual void TearDown() {
      auto allFiles = g3::internal::getArchiveLogFilesInDirectory(_directory, _filename + ".log");
      const int kFilePathIndex = 1;
      for (auto& p : allFiles) {
         // std::get(std::pair)
         // Returns a reference to p.first if kFilePathIndex == 0 and 
         // a reference to p.second if kFilePathIndex == 1.
         std::string file = std::get<kFilePathIndex>(p);
         if ((std::find(_filesToRemove.begin(), _filesToRemove.end(), _directory + file) == _filesToRemove.end())) {
            _filesToRemove.push_back(_directory + file/*std::get<kFilePathIndex>(p)*/);
         }
      }

      for (auto filename : _filesToRemove) {
         std::remove(filename.c_str());
      }
   }

   std::string _filename;
   std::string _directory;
   std::vector<std::string> _filesToRemove;
};