#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace StringUtils{

      inline std::vector<std::string>& Split(const std::string &s, char delim, std::vector<std::string> &elems) {
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                  elems.push_back(item);
            }
            return elems;
      }


      inline std::vector<std::string> Split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            Split(s, delim, elems);
            return elems;
      }

      template<typename T>
      T Parse( const std::string& s ) {
          std::istringstream stream( s );
          T t;
          stream >> t;
          return t;
      }
}
